/* block.c begin */
#include "mmc_core.h"

#define REQUEST_READ (1 << 0)
#define REQUEST_DISCARD (1 << 1)
#define IS_REQUEST_READ(req)  ((req)->req_flags & 0x1)
#define IS_REQUEST_DISCARD(req)  ((req)->req_flags & REQUEST_DISCARD)

#define IS_SIGNAL_SECTOR_RW(req) ((req)->sector_sz == 1)
#define add_to_queue(req, req_queue) \
    list_add_tail(&req->queuelist, &req_queue->queuehead)
#define check_queue_empty(req_queue) list_empty(&req_queue->queuehead)
#define get_request(req_queue) list_first_entry(&req_queue->queuehead,struct request,queuelist)
#define remove_request(req) list_del(&req->queuelist)

#define check_thread_stop(mmc_queue) (mmc_queue->thread_state & THREAD_SHOULD_STOP)

/* local function */
static int make_up_mmc_req(struct request *request, struct mmc_req_queue* mmc_queue,
        struct mmc_req * mreq);
static int wake_up_thread(void * dev_data);
static int mmc_blk_retrieve(struct mmc_req_queue* mmc_queue,
        struct mmc_req_data* data,unsigned int wait);

struct blk_node_no blk_no ={
    .blk_max_count = MAX_BLOCK_COUNT
};


static int block_queue_init(struct request_queue* blk_q, mmc_mutex *lock,struct mmc_card *card)
{
    INIT_LIST_HEAD(&blk_q->queuehead);
    blk_q->p_mux_queue_lock = lock;
    return 0;
}

static struct request* get_blk_request(struct request_queue* blk_q)
{
    struct request *req = NULL;
    if(!check_queue_empty(blk_q)) {
        req = get_request(blk_q);
        remove_request(req);
    }
    return req;
}

#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
static void request_done(struct mmc_req * mreq, int status)
{
    struct request* request = mreq->request;

    request->error = status;

    mmc_event_signal(&request->wait_event, REQUEST_EVENT_ANSWER);
}

static int card_recovery(struct mmc_card *card, struct mmc_req *mreq)
{
    unsigned int status = 0, stop_status = 0;
    unsigned int mreq_dir = 0;
    int retry = 3;
    int err = 0;

    /*step 1: check the card is CARD_UNPLUGED */
    if (mmc_get_card_detect(card->host)) {
        mmc_err("card is unpluged!\n");
        return MMC_REQ_ABORT;
    }

    /*step 2: sent cmd13 to get card status. */
    for(; retry >= 0; retry--) {
        err = get_card_status(card, &status);
        if(!err)
            break;
    }

    if (!err) {
        /* check card status, and printf it for debug.*/
        if ((status & R1_CARD_ECC_FAILED) ||
                (mreq->req_entity.cmd.resp[0] & R1_CARD_ECC_FAILED) ||
                (mreq->req_entity.data.cmd_stop.resp[0] & R1_CARD_ECC_FAILED))
            mmc_err("cmd.resp[0] = 0x%x,cmd_stop.resp[0] = 0x%x\n",
                    mreq->req_entity.cmd.resp[0],
                    mreq->req_entity.data.cmd_stop.resp[0]);

        if (!IS_REQUEST_READ(mreq->request)) {
            if ((status & R1_ERROR) ||
                    (mreq->req_entity.data.cmd_stop.resp[0] & R1_ERROR)) {
                mmc_err("cmd.resp[0] = 0x%x,cmd_stop.resp[0] = 0x%x\n",
                        mreq->req_entity.cmd.resp[0],
                        mreq->req_entity.data.cmd_stop.resp[0]);
            }
        }

        /* if card status at R1_STATE_DATA and R1_STATE_RCV status, try to
         * sent stop cmd .*/
        if (R1_CURRENT_STATE(status) == CARD_STATE_DATA ||
                R1_CURRENT_STATE(status) == CARD_STATE_RCV) {
            //err = sent_stop(card, mreq, &stop_status);
            if (!IS_REQUEST_READ(mreq->request))
                mreq_dir = 1; // write
            err = sent_stop(card, mreq_dir, &stop_status);
            if (err) {
                mmc_err("stop_status = 0x%x\n", stop_status);
                //return MMC_REQ_ABORT;
            }

            if (stop_status & R1_CARD_ECC_FAILED) {
                mmc_err("card ecc fail\n");
                //return MMC_REQ_ABORT;
            } else
                return MMC_REQ_RETRY;
        }
    } else
        mmc_err("get card status fail!\n");

#ifdef CARD_ERR_RESET_ENABLE
    /*step 3: reset card! */
    if (card_reset(card))
        return MMC_REQ_ABORT;
#endif

    return MMC_REQ_RETRY;
}
static int mreq_err_check(struct mmc_card *card, struct mmc_req *mreq)
{
    struct request* req = mreq->request;
    struct mmc_request* mrq = &mreq->req_entity.mrq;
    unsigned int err_state = 0;
    int status = MMC_REQ_SUCCESS;

    if (mrq->cmd->err || mrq->data->err) {
            mmc_err("cmd->error = %d,data->error = %d\n",
                    mrq->cmd->err, mrq->data->err);
        if (mrq->data->err == -EILSEQ) {
            return MMC_REQ_RETRY;
        }
        status = card_recovery(card, mreq);
    }
    return status;
}



static int issue_discard_rq(struct mmc_req_queue* mmc_queue,
        struct request * req)
{
    int status = 0;
    struct mmc_card *card = mmc_queue->card;
    unsigned int start, blocks , arg;

    if (!mmc_can_erase(card)) {
        status = MMC_REQ_ABORT;
        goto out;
    }
    start = req->sector_s;
    blocks = req->sector_sz;

    if (mmc_can_discard(card))
        arg = MMC_DISCARD_ARG;
    else if (mmc_can_trim(card))
        arg = MMC_TRIM_ARG;
    else
        arg = MMC_ERASE_ARG;

    status = mmc_erase(card, start, blocks, arg);
    if (status == 0)
        status = MMC_REQ_SUCCESS;
    else
        status = MMC_REQ_ABORT;

out:
    return status;
}



static int mmc_blk_req_start(struct mmc_req_queue* mmc_queue,
        struct request * req)
{
    struct request_queue* blk_q = mmc_queue->queue;
    struct mmc_req *mreq = NULL;
    struct request* cur_req = NULL;
    int status = 0;

    cur_req = req;
    do
    {
        if (IS_REQUEST_DISCARD(req)) {
            mreq = &mmc_queue->mreq;
            mreq->request = cur_req;
            status = issue_discard_rq(mmc_queue, cur_req);
        } else {
            mreq = &mmc_queue->mreq;
            make_up_mmc_req(cur_req, mmc_queue, mreq); //make up entity
            mmc_start(mmc_queue->card, &mreq->req_entity);
            status = mreq_err_check(mmc_queue->card, mreq);
        }
        switch(status) {
            /* no error , get next request.*/
            case MMC_REQ_SUCCESS:
            request_done(mreq, ENOERR);
            cur_req = NULL;
            if (mmc_mutex_lock((*(blk_q->p_mux_queue_lock)), MMC_MUTEX_WAIT_DEF_TIME)
                    == MMC_MUTEX_RETURN_TIMEOUT) {
                mmc_err("lock queue_lock timeout!\n");
            } else {
                cur_req = get_blk_request(blk_q);
                mmc_mutex_unlock((*(blk_q->p_mux_queue_lock)));
            }
            break;
            /* try again. */
            case MMC_REQ_RETRY:
            if (cur_req->retries == 0) {
                request_done(mreq, -EIO);
                cur_req = NULL;
            } else {
                mmc_trace(5,"retries = %d!sector_s = 0x%x,sector_sz = 0x%x\n",
                        cur_req->retries, cur_req->sector_s, cur_req->sector_sz);
                cur_req->retries--;
            }
            break;
            case MMC_REQ_ABORT:
            request_done(mreq, -EIO);
            cur_req = NULL;
            mmc_err("card error!\n");
            break;
            default:
            cur_req = NULL;
            mmc_err("unknow error!\n");
            break;
        }
    } while (cur_req);

    return 0;
}

#else /* LOSCFG_DRIVERS_MMC_SPEEDUP */

#define check_req_queue_empty(q) (!((q)->used))
#define check_req_queue_full(q) ((q)->used >= ((q)->capacity -2)) /*FIXME*/
#define BLOCKING_REQ_COUNT 4

static int put_mmc_queue(struct mmc_req_queue* mmc_queue, struct mmc_req * mreq)
{
    int ret = 0;
    if (mmc_queue->used <= 0)
        ret = -1;
    else if (&mmc_queue->mreq[mmc_queue->start] == mreq) {
        mmc_queue->start++;
        mmc_queue->start %= mmc_queue->capacity;
        mmc_queue->used--;
        ret = 0;
    } else
        ret = 1;
    mmc_trace(3,"start = %d, end = %d. used = %d\n",
            mmc_queue->start,
            mmc_queue->end,
            mmc_queue->used);
    return ret;
}

static struct mmc_req* mmc_get_rbuf(struct mmc_req_queue* mmc_queue)
{
    struct mmc_req* mreq = NULL;
    if(mmc_queue->used == (mmc_queue->capacity -2)) {
        mreq = NULL;
    } else {
        //mmc_trace(3,"%s,%d,pf  mmc_queue->end =%d,mmc_queue->used = %d\n",
        //        __func__,__LINE__,mmc_queue->end, mmc_queue->used);
        mreq = &mmc_queue->mreq[mmc_queue->end];
        mmc_queue->end++;
        mmc_queue->end %= mmc_queue->capacity;
        mmc_queue->used++;
    }

    return mreq;
}

static void request_retrieve(struct mmc_request* mrq)
{
    struct mmc_req_entity* req_entity = container_of(mrq, struct mmc_req_entity, mrq);
    struct mmc_req * mreq = container_of(req_entity, struct mmc_req, req_entity);
    mmc_event_signal(mreq->p_thread_event, THREAD_RETIREVE_REQ);
}

static void request_done(struct mmc_req_queue* mmc_queue, struct mmc_req * mreq)
{
    struct mmc_req_entity* req_entity = &mreq->req_entity;
    struct mmc_request* mrq = &req_entity->mrq;
    struct request* request = mreq->request;

    mmc_trace(3,"start = %d, end = %d. used = %d\n",
            mmc_queue->start,
            mmc_queue->end,
            mmc_queue->used);

    if (request) {
        if (mrq->cmd->err || mrq->data->err) {
            mmc_err("cmd->error = %d,data->error = %d\n",
                    mrq->cmd->err, mrq->data->err);
            request->error = -EAGAIN;
        }

        mmc_event_signal(&request->wait_event, REQUEST_EVENT_ANSWER);
    }
    put_mmc_queue(mmc_queue, mreq);
}

static void mreq_queue_init(struct mmc_req_queue* mmc_queue)
{
    mmc_queue->start = 0;
    mmc_queue->end = 0;
    mmc_queue->used =0;
    mmc_queue->capacity = MMC_MAX_REQS_SEND_ONCE;
}

static int mmc_blk_retrieve(struct mmc_req_queue* mmc_queue,
        struct mmc_req_data* data,unsigned int wait)
{
    unsigned int rd_pos;
    struct mmc_req * mreq = NULL;
    struct request * req = NULL;
    struct mmc_req_entity* req_entity;
    struct mmc_card * card = mmc_queue->card;
    struct mmc_host * host = card->host;
    while(!check_req_queue_empty(mmc_queue))
    {
        rd_pos = host->get_rdptr(host);
        mreq = &mmc_queue->mreq[mmc_queue->start];
        mmc_trace(3, "status = %d, start = %d,end = %d\n",
                host->status, mmc_queue->start,mmc_queue->end);
        mmc_trace(3, "remain %d, cmd ptr %d, read ptr = %d",
                mmc_queue->used, mreq->wr_pos, rd_pos);
        if (!host->status && (mreq->wr_pos == rd_pos)) {
            if (wait) {
                msleep(10);
                continue;
            } else {
                break;
            }
        }
        request_done(mmc_queue, mreq);
    }
}

static int mmc_blk_req_start(struct mmc_req_queue* mmc_queue,
        struct request * req)
{
    struct request_queue* blk_q = mmc_queue->queue;
    struct mmc_req *mreq = NULL;
    struct request* next_req = NULL;
    unsigned int reqs = 0;
    int ret = 0;
    if (req == NULL)
        return reqs;
    do {
        mreq = mmc_get_rbuf(mmc_queue);
        ret = make_up_mmc_req(req, mmc_queue, mreq); //make up entity
        if(ret) {
            mmc_err("set up mmc_req fail! err = %d\n", ret);
            return ret;
        }
        next_req = NULL;
        if (!check_req_queue_full(mmc_queue)) {
            if (mmc_mutex_lock((*(blk_q->p_mux_queue_lock)), MMC_MUTEX_WAIT_DEF_TIME)
                    == MMC_MUTEX_RETURN_TIMEOUT) {
                mmc_err("lock queue_lock timeout!\n");
            } else {
                next_req = get_blk_request(blk_q);
                mmc_mutex_unlock((*(blk_q->p_mux_queue_lock)));
            }
        }
        if (next_req && (reqs % BLOCKING_REQ_COUNT)) {
            mreq->req_entity.cmd.resp_type |= MMC_CMD_NON_BLOCKING;
            mmc_trace(3,"not blocking\n");
        }
        mmc_start(mmc_queue->card, &mreq->req_entity);
        mreq->wr_pos = mreq->req_entity.mrq.wr_pos;
        req = next_req;
        reqs++;
    } while(next_req);
    return reqs;
}
#endif /* LOSCFG_DRIVERS_MMC_SPEEDUP */

static int make_up_mmc_req(struct request *request, struct mmc_req_queue* mmc_queue,
        struct mmc_req * mreq)
{
    //struct mmc_req_queue* mmc_queue = &data->mmc_data_queue;
    //struct mmc_req * mreq = NULL;
    struct mmc_req_entity* req_entity = NULL;
    int ret = 0;

    //mreq = &mmc_queue->mreq;
    if (mreq == NULL)
        return -1;
    memset(mreq, 0, sizeof(struct mmc_req));

    mreq->request = request;

    req_entity = &mreq->req_entity;
    req_entity->data.sg = &mreq->sg;

    req_entity->data.data_buffer = request->buffer;
    req_entity->mrq.cmd = &req_entity->cmd;
    req_entity->mrq.data = &req_entity->data;

    req_entity->cmd.arg = request->sector_s;

    if (!is_card_blkaddr(mmc_queue->card))
        req_entity->cmd.arg <<= 9;
    req_entity->cmd.resp_type = MMC_RESP_R1 | MMC_CMD_ADTC;
    req_entity->data.blocksz = MMC_CARDBUS_BLOCK_SIZE;
    #ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
    mreq->p_thread_event = &mmc_queue->thread_event;
    req_entity->cmd.resp_type |= MMC_CMD_TYPE_RW;
    #endif

    if (IS_SIGNAL_SECTOR_RW(request)) {
        if (IS_REQUEST_READ(request)) {
            req_entity->cmd.cmd_code = READ_SINGLE_BLOCK;
            req_entity->data.data_flags = MMC_DATA_READ;
        } else {
            req_entity->cmd.cmd_code = WRITE_BLOCK;
            req_entity->data.data_flags = MMC_DATA_WRITE;
        }

        req_entity->data.blocks = 1;
        req_entity->data.sg_len = 1;
        req_entity->data.data_flags &= ~MMC_CMD_STOP;     /* don't need stop cmd*/

        sg_init_one(req_entity->data.sg, req_entity->data.data_buffer,
                req_entity->data.blocksz * req_entity->data.blocks);

    } else {
        req_entity->data.blocks = request->sector_sz;
        sg_init_one(req_entity->data.sg, req_entity->data.data_buffer,
                MMC_CARDBUS_BLOCK_SIZE * req_entity->data.blocks);
        req_entity->data.sg_len = 1; /*FIXME*/

        if (IS_REQUEST_READ(request)) {
            req_entity->cmd.cmd_code = READ_MULTIPLE_BLOCK;
            req_entity->data.data_flags = MMC_DATA_READ;
            req_entity->data.cmd_stop.resp_type = MMC_RESP_R1 | MMC_CMD_AC;
        } else {
            req_entity->cmd.cmd_code = WRITE_MULTIPLE_BLOCK;
            req_entity->data.data_flags = MMC_DATA_WRITE;
            req_entity->data.cmd_stop.resp_type = MMC_RESP_R1B | MMC_CMD_AC;
        }

        req_entity->data.data_flags |= MMC_CMD_STOP;
        req_entity->data.cmd_stop.cmd_code = STOP_TRANSMISSION;
        req_entity->data.cmd_stop.arg = 0;
    }
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    req_entity->mrq.done = NULL;
#else
    req_entity->mrq.done = request_retrieve;
#endif
    return 0;
}

int clear_blk_queue(struct request_queue* blk_q)
{
    struct request *req = NULL;
    if (mmc_mutex_lock((*(blk_q->p_mux_queue_lock)), MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
        mmc_err("lock queue_lock timeout!\n");
        return -1;
    }
    CLR_BLK_QUEUE_NORMAL(blk_q);
    while (req = get_blk_request(blk_q))
    {
        req->error = -ESRCH;
        mmc_event_signal(&req->wait_event, REQUEST_EVENT_ANSWER);
    }
    mmc_mutex_unlock((*(blk_q->p_mux_queue_lock)));
    return 0;
}
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
static int mmc_data_thread(struct mmc_req_data* data)
{
    struct mmc_req_queue* mmc_queue = &data->mmc_data_queue;
    struct request_queue* blk_q = mmc_queue->queue;
    struct request *req = NULL;
    do
    {
        if(check_thread_stop(mmc_queue)) {
            /* clear block queue request */
            clear_blk_queue(blk_q);
            break;
        }

        req = NULL;
        if (mmc_mutex_lock((*(blk_q->p_mux_queue_lock)), MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
            mmc_err("lock queue_lock timeout!\n");
            continue;
        }
        req = get_blk_request(blk_q);
        mmc_mutex_unlock((*(blk_q->p_mux_queue_lock)));

        if (req) {
            mmc_blk_req_start(mmc_queue, req);
        } else
            (void)mmc_event_wait(&mmc_queue->thread_event, THREAD_RUNNING, WAIT_THREAD_RUNING_TIMEOUT);
    } while(1);
    mmc_event_signal(&data->mmc_data_queue.thread_event, THREAD_STOPPED);
    return 0;
}
#else
static int mmc_data_thread_speedup(struct mmc_req_data* data)
{
    struct mmc_req_queue* mmc_queue = &data->mmc_data_queue;
    struct request_queue* blk_q = mmc_queue->queue;
    struct request *req = NULL;
    struct mmc_req *mreq = NULL;

    int ret = 0;
    do
    {
        /* retrieve reqs*/
        mmc_blk_retrieve(mmc_queue, data, false);

        if (check_thread_stop(mmc_queue)) {
            /* clear block queue request */
            clear_blk_queue(blk_q);
            /* wait req queue is done */
            if (check_req_queue_empty(mmc_queue))
            break;
        }
        req = NULL;
        if (!check_req_queue_full(mmc_queue)) {
            if (mmc_mutex_lock((*(blk_q->p_mux_queue_lock)), MMC_MUTEX_WAIT_DEF_TIME)
                    == MMC_MUTEX_RETURN_TIMEOUT) {
                mmc_err("lock queue_lock timeout!\n");
                continue;
            }
            req = get_blk_request(blk_q);
            mmc_mutex_unlock((*(blk_q->p_mux_queue_lock)));
        }

        if (req || !check_req_queue_empty(mmc_queue)) {
            mmc_blk_req_start(mmc_queue, req);
            (void)mmc_event_wait(&mmc_queue->thread_event, THREAD_RUNNING | THREAD_RETIREVE_REQ, WAIT_THREAD_RUNING_TIMEOUT);
        } else {
            (void)mmc_event_wait(&mmc_queue->thread_event, THREAD_RUNNING , WAIT_THREAD_RUNING_TIMEOUT);
        }
    } while(1);

    mmc_event_signal(&data->mmc_data_queue.thread_event, THREAD_STOPPED);
    return 0;
}
#endif /* LOSCFG_DRIVERS_MMC_SPEEDUP */

static int dev_data_init(struct mmc_card * card, struct request_queue* blk_q)
{
    struct mmc_req_data* data;
    int ret;

    data = malloc(sizeof(struct mmc_req_data));
    if(!data) {
        mmc_err("malloc for card->card_data fail\n");
        return -ENOMEM;
    }
    memset(data, 0x0, sizeof(struct mmc_req_data));

    card->card_data = (void*) data;
    data->card = card;

    data->mmc_data_queue.queue = blk_q;
    data->mmc_data_queue.card = card;
    if (mmc_event_init(&data->mmc_data_queue.thread_event)) {
        mmc_err("event init fail!");
        ret = -EACCES;
        goto free_data;
    }

#ifdef  LOSCFG_DRIVERS_MMC_SPEEDUP
    mreq_queue_init(&data->mmc_data_queue);
    ret = mmc_thread_create(6,
            mmc_data_thread_speedup,
            BLOCK_THREAD_STACKSZ,
            data,
            "mmc_data_thread",
            &data->mmc_data_queue.thread);
#else
    ret = mmc_thread_create(6,
            mmc_data_thread,
            BLOCK_THREAD_STACKSZ,
            data,
            "mmc_data_thread",
            &data->mmc_data_queue.thread);
#endif

    if (ret) {
        mmc_err("create thread for mmc_data_thread fail!\n");
        goto free_data;
    }
    SET_BLK_QUEUE_NORMAL(blk_q);
    return 0;

free_data:
    free(data);
out:
    return ret;
}

int dev_data_deinit(struct mmc_block* block)
{
    struct mmc_req_data* data = (struct mmc_req_data*)block->data;
    unsigned int retry_count = 3;
    int ret = 0;

    /* step 1: disable user read/write and wait request in block done */
    block->block_status = BLOCK_ABNORMAL;
    do {
        if (mmc_atomic_read(&block->cur_reqs) == 0)
            break;
        else
            mmc_sleep_ms(10);
    }while(1);

    /*step 2: delete data_queue. */

    data->mmc_data_queue.thread_state |= THREAD_SHOULD_STOP;
    /*needs clear block queue request here.*/
    do
    {
        wake_up_thread(block->data);
        ret = mmc_event_wait(&data->mmc_data_queue.thread_event, THREAD_STOPPED, WAIT_THREAD_STOP_TIMEOUT);

        if (ret == THREAD_STOPPED)
            break;
        else if(ret == LOS_ERRNO_EVENT_READ_TIMEOUT) {
            retry_count--;
            msleep(10);
            continue;
        } else {
            mmc_err("kill mmc_queue thread fail,ret = %d\n", ret);
            break;
        }
    } while(retry_count);

    if (!retry_count)
        mmc_err("kill mmc_queue thread timeout!\n");

    mmc_event_delete(&data->mmc_data_queue.thread_event);
    /* free dev_data */
    free(data);
    return 0;
}


static int wake_up_thread(void * dev_data)
{
    struct mmc_req_data * data = (struct mmc_req_data *)dev_data;
    mmc_event_signal(&data->mmc_data_queue.thread_event, THREAD_RUNNING);
    return 0;
}

static int request_start(struct mmc_block *block, struct request* req)
{
    struct request_queue* blk_q = &block->queue;
    int ret;

    mmc_event_init(&req->wait_event);
    if (mmc_mutex_lock(block->queue_lock, MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
        mmc_err("lock block->queue_lock timeout!\n");
        ret = -ETIMEDOUT;
        goto err;
    }

    if (IS_BLK_QUEUE_NORMAL(blk_q)) {
        add_to_queue(req, blk_q);
        mmc_mutex_unlock(block->queue_lock);
    } else {
        ret = -EIO;
        mmc_mutex_unlock(block->queue_lock);
        goto err;
    }

    wake_up_thread(block->data);

    ret = mmc_event_wait(&req->wait_event,
            REQUEST_EVENT_ANSWER,
            req->timeout);
    if (ret != REQUEST_EVENT_ANSWER) {
        if (IS_REQUEST_READ(req)) {
            mmc_err("mmc_read wait event fail! Event ret = %d\n", ret);
        }
        else {
            mmc_err("mmc_write wait event fail! Event ret = %d\n", ret);
        }
        goto del_request;
    }

    if(req->error) {
        mmc_err("req->sector_s = 0x%x,req->sector_sz = 0x%x,req_flags = 0x%x\n",
                req->sector_s,req->sector_sz,req->req_flags);
        ret = -EAGAIN;
        goto err;
    }
    mmc_event_delete(&req->wait_event);
    return req->sector_sz;

del_request:

    /*wait event timeout, del request from queue*/
    if (mmc_mutex_lock(block->queue_lock, MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
        mmc_err("Warning:lock block->queue_lock timeout!\n");
        goto err;
    } else
        remove_request(req);
    mmc_mutex_unlock(block->queue_lock);

err:
    mmc_event_delete(&req->wait_event);
    return ret;
}

#ifndef LOSCFG_FS_FAT_CACHE
#define ALIGNED_BUFF_NUM 128
unsigned char readbuf_aligned[ALIGNED_BUFF_NUM*512 + 64];
unsigned char writebuf_aligned[ALIGNED_BUFF_NUM*512 + 64];
#endif

#define CONFIG_DEF_ERASE_SECTOR  0x100000 //512M
ssize_t do_mmc_erase(unsigned int block_id, size_t start_sector, unsigned int nsectors)
{
    struct mmc_block *block = NULL;
    struct mmc_card* card = NULL;

    struct request qreq;
    struct request* req = &qreq;
    unsigned int sector_now = nsectors;

    int ret = 0;

    block = blk_no.blocks[block_id];
    if (!block) {
        mmc_err("block no found!\n");
        return -EINVAL;
    }
    card = (struct mmc_card *)block->dev;

    mmc_atomic_inc(&block->cur_reqs);
    if (block->block_status != BLOCK_NORMAL) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    if (is_card_present(card) == 0) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    do
    {
        memset(req, 0x0, sizeof(struct request));

        if (sector_now > CONFIG_DEF_ERASE_SECTOR) {
            req->sector_s = start_sector;
            req->sector_sz = CONFIG_DEF_ERASE_SECTOR;
        } else {
            req->sector_s = start_sector;
            req->sector_sz = sector_now;
        }
        req->req_flags |= REQUEST_DISCARD;
        req->q = &block->queue;
        req->retries = 1;
        req->timeout = MMC_EVENT_WAIT_DEF_TIME;

#ifndef LOSCFG_FS_FAT_CACHE
        mmc_err("Not Support!\n");
        return -EIO;
#else
        req->buffer = NULL;
        ret = request_start(block, req);
        if (ret != req->sector_sz)
            break;
#endif
        sector_now -= ret;
        start_sector += ret;
    }while(sector_now);
    mmc_atomic_dec(&block->cur_reqs);
    return ret;
}
static ssize_t mmc_read(FAR struct inode *inode, FAR unsigned char *buffer,
        size_t start_sector, unsigned int nsectors)
{
    struct mmc_block *block = (struct mmc_block*)inode->i_private;
    struct mmc_card* card = (struct mmc_card *)block->dev;
    struct request qreq;
    struct request* req = &qreq;
    int ret = 0;

    mmc_atomic_inc(&block->cur_reqs);
    if (block->block_status != BLOCK_NORMAL) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    if (is_card_present(card) == 0) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    memset(req, 0x0, sizeof(struct request));

    req->req_flags |= REQUEST_READ;
    req->sector_s = start_sector;
    req->sector_sz = nsectors;
    req->q = &block->queue;
    req->retries = REQUEST_RETRIES_TIMES;
    req->timeout = MMC_EVENT_WAIT_DEF_TIME;

#ifndef LOSCFG_FS_FAT_CACHE
    unsigned char * alignedbuffer,* alignedbuffer_save;
    unsigned int sectors_tmp = nsectors;
    unsigned int buffer_offset = 0;
    alignedbuffer_save = readbuf_aligned;
    alignedbuffer = (unsigned char *)(((int)alignedbuffer_save + CACHE_ALIGNED_SIZE)
        & (~(CACHE_ALIGNED_SIZE - 1)));
    req->buffer = alignedbuffer;

    do
    {
        req->sector_sz = (sectors_tmp > ALIGNED_BUFF_NUM)?ALIGNED_BUFF_NUM:sectors_tmp;
        sectors_tmp -= req->sector_sz;
        ret = request_start(block, req);
        if (ret != req->sector_sz) {
            mmc_err("read fail!ret = %d\n",ret);
            mmc_atomic_dec(&block->cur_reqs);
            return ret;
        }
        memcpy(buffer + buffer_offset, alignedbuffer, req->sector_sz* MMC_CARDBUS_BLOCK_SIZE);
        buffer_offset += req->sector_sz* MMC_CARDBUS_BLOCK_SIZE;
    } while (sectors_tmp);
#else
    req->buffer = buffer;
    ret = request_start(block, req);
#endif
    mmc_atomic_dec(&block->cur_reqs);
    return ret;
}

static ssize_t mmc_write(FAR struct inode *inode, FAR const unsigned char *buffer,
        size_t start_sector, unsigned int nsectors)
{
    struct mmc_block *block = (struct mmc_block*)inode->i_private;
    struct mmc_card* card = (struct mmc_card *)block->dev;
    struct request qreq ;
    struct request* req = &qreq;
    int ret = 0;

    mmc_atomic_inc(&block->cur_reqs);
    if (block->block_status != BLOCK_NORMAL) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    if (is_card_present(card) == 0) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    memset(req, 0x0, sizeof(struct request));

    req->req_flags &= ~REQUEST_READ;
    req->sector_s = start_sector;
    req->sector_sz = nsectors;
    req->q = &block->queue;
    req->retries = REQUEST_RETRIES_TIMES;
    req->timeout = MMC_EVENT_WAIT_DEF_TIME;
#ifndef LOSCFG_FS_FAT_CACHE
    unsigned char * alignedbuffer,* alignedbuffer_save;
    unsigned int sectors_tmp = nsectors;
    unsigned int buffer_offset = 0;
    unsigned char* buff;
    buff = (unsigned char*)buffer;
    alignedbuffer_save = writebuf_aligned;
    alignedbuffer = (unsigned char*)(((int)alignedbuffer_save + CACHE_ALIGNED_SIZE)
        & (~(CACHE_ALIGNED_SIZE - 1)));
    req->buffer = alignedbuffer;

    do
    {
        req->sector_sz = (sectors_tmp > ALIGNED_BUFF_NUM)?ALIGNED_BUFF_NUM:sectors_tmp;
        sectors_tmp -= req->sector_sz;
        memcpy(alignedbuffer + buffer_offset, buff, req->sector_sz* MMC_CARDBUS_BLOCK_SIZE);
        buffer_offset += req->sector_sz* MMC_CARDBUS_BLOCK_SIZE;
        ret = request_start(block, req);
        if (ret != req->sector_sz) {
            mmc_err("write fail!ret = %d\n",ret);
            mmc_atomic_dec(&block->cur_reqs);
            return ret;
        }
    } while (sectors_tmp);
#else
    req->buffer = (unsigned char *)buffer;
    ret = request_start(block, req);
#endif
    mmc_atomic_dec(&block->cur_reqs);
    return ret;
}

static int mmc_geometry(FAR struct inode *inode, FAR struct geometry *geometry)
{
    struct mmc_block *block = (struct mmc_block*)inode->i_private;
    struct mmc_card* card = (struct mmc_card *)block->dev;

    if(geometry == NULL)
        return -EINVAL;

    mmc_atomic_inc(&block->cur_reqs);
    if (block->block_status != BLOCK_NORMAL) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    geometry->geo_available    = true;
    geometry->geo_mediachanged = false;
    geometry->geo_writeenabled = true;


    if (!(is_card_sd(card)) && (is_card_blkaddr(card))) {
        geometry->geo_nsectors = card->card_reg.ext_csd.sectors;
    } else {
        geometry->geo_nsectors = card->card_reg.csd.capacity << (card->card_reg.csd.read_blkbits - 9);
    }

    geometry->geo_sectorsize = MMC_CARDBUS_BLOCK_SIZE;
    mmc_atomic_dec(&block->cur_reqs);

    return ENOERR;
}

static int mmc_ioctl(FAR struct inode *inode, int cmd, unsigned long arg)
{

    struct mmc_block *block = (struct mmc_block*)inode->i_private;
    struct mmc_card* card = (struct mmc_card *)block->dev;
    struct geometry gm;

    mmc_atomic_inc(&block->cur_reqs);
    if (block->block_status != BLOCK_NORMAL) {
        mmc_atomic_dec(&block->cur_reqs);
        return -EIO;
    }

    switch (cmd) {
        case RT_DEVICE_CTRL_BLK_GETGEOME:
            mmc_geometry(inode, &gm);
            struct rt_device_blk_geometry *rt_geo = (struct rt_device_blk_geometry *)arg;
            rt_geo->sector_count = gm.geo_nsectors;
            rt_geo->bytes_per_sector = MMC_CARDBUS_BLOCK_SIZE;
            rt_geo->block_size = 512;
            break;
        case RT_DEVICE_CARD_STATUS:
            if(is_card_present(card)==0)
                *(int *)arg = 1;
            else
                *(int *)arg = 0;
            break;
        case RT_DEVICE_CARD_AU_SIZE:
            *(unsigned int *)arg = card->card_reg.ssr.au_value;
            break;
        default:
            break;
    }
    mmc_atomic_dec(&block->cur_reqs);
    return ENOERR;
}

static int  mmc_open(FAR struct inode *inode)
{
    return 0;
}

static int mmc_close(FAR struct inode *inode)
{
    return 0;
}


int block_init(void)
{
    if (mmc_mutex_init(&blk_no.flag_lock)) {
        mmc_err("block init err");
        return -EACCES;
    }
    memset(blk_no.node_flag, 0x0, blk_no.blk_max_count);
    return 0;
}

static int get_blknode_nu(struct mmc_block *block)
{
    int i;
    if (mmc_mutex_lock(blk_no.flag_lock, MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
        mmc_err("lock blk_no.flag_lock timeout!\n");
        return 0;
    }
    for(i = 0; i < blk_no.blk_max_count; i++) {
        if(blk_no.node_flag[i] == 0) {
            blk_no.node_flag[i] = 1;
            blk_no.blocks[i] = block;
            mmc_mutex_unlock(blk_no.flag_lock);
            return i;
        }
    }
    mmc_mutex_unlock(blk_no.flag_lock);
    mmc_err("block number out of range!\n");
    return i;
}

static void free_blknode_nu(int number)
{
    if (mmc_mutex_lock(blk_no.flag_lock, MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
        mmc_err("lock blk_no.flag_lock timeout!\n");
        return;
    }
    if (blk_no.node_flag[number] == 0)
        mmc_err("This is no a right number!\n");
    blk_no.node_flag[number] = 0;
    blk_no.blocks[number]  = NULL;
    mmc_mutex_unlock(blk_no.flag_lock);
}

struct disk_divide_info emmc = {.sector_count = 0xffffffff};
int mmc_block_init(void* dev)
{
    struct mmc_block *block;
    struct mmc_card* card = (struct mmc_card*)dev;
    unsigned long long capacity = 0;
    block->cur_reqs = MMC_ATOMIC_INIT(0);
    block->block_status = BLOCK_NORMAL;
    unsigned int gib_val = 0;
    unsigned int mib_val = 0;
    int ret = 0;
#ifdef MULTI_DEVICE
    int disk_id;
#endif

    block = malloc(sizeof(struct mmc_block));
    if (!block) {
        mmc_err("err:%s,%d,malloc memory for block fail!\n");
        return -ENOMEM;
    }
    memset(block, 0x0, sizeof(struct mmc_block));
    block->dev = (void*)dev;

    if(is_card_mmc(card) || is_card_sd(card) || is_card_combo(card)) {
        block->node_no = get_blknode_nu(block);
        snprintf(block->node_name, sizeof(block->node_name), "/dev/mmcblk%0d", block->node_no);
    } else if (is_card_sdio(card)) {
        /* sdio don't need data */
        snprintf(block->node_name, sizeof(block->node_name), "/dev/sdio%0d", card->card_idx);
        mmc_trace(3,"register sdio %s\n",block->node_name);
        register_driver(block->node_name, NULL, 0666, card);
        card->block = block;
        return 0;
    }

    if (mmc_mutex_init(&block->queue_lock)) {
        mmc_err("block create metex fail\n");
        ret = -EACCES;
        goto free_block;
    }
    if (!(is_card_sd(card)) && (is_card_blkaddr(card))) {
        capacity = card->card_reg.ext_csd.sectors;
    } else {
        capacity = (unsigned long long)card->card_reg.csd.capacity << (card->card_reg.csd.read_blkbits - 9);
        mmc_trace(MMC_TRACE_DEBUG, "csd.capacity: %u, read_blkbits %u\n",
                card->card_reg.csd.capacity, card->card_reg.csd.read_blkbits);
    }
    gib_val = capacity >> 21;
    mib_val = (capacity & ~(gib_val << 21)) >> 11;
    if (gib_val)
        mmc_printf(0, "card capacity %d.%d Gib\n",gib_val,mib_val);
    else
        mmc_printf(0, "card capacity %d Mib\n",mib_val);

    block->blk_capacity = capacity;

    block_queue_init(&block->queue, &block->queue_lock, card);

    ret = dev_data_init(card, &block->queue);
    if (ret < 0) {
        mmc_err("dev_data_init fail\n");
        goto free_block;
    }

    block->data = card->card_data;
    block->bops.open = mmc_open;
    block->bops.close = mmc_close;
    block->bops.read = mmc_read;
    block->bops.write = mmc_write;
    block->bops.geometry = mmc_geometry;
    block->bops.ioctl = mmc_ioctl;

    if (is_card_removeable(card)) { //sd
#ifdef MULTI_DEVICE
        disk_id = los_alloc_diskid_byname(block->node_name);
        los_disk_init(block->node_name, &block->bops, (void *)block,
                disk_id, NULL);
#else
        los_disk_init(block->node_name, &block->bops, (void *)block,
                block->node_no, NULL);
#endif
    } else { //emmc
        emmc.sector_count  = capacity; //FIXME
#ifdef MULTI_DEVICE
        disk_id = los_alloc_diskid_byname(block->node_name);
        los_disk_init(block->node_name, &block->bops, (void *)block,
                disk_id, &emmc);
#else
        los_disk_init(block->node_name, &block->bops, (void *)block,
                block->node_no, &emmc);
#endif
    }

    card->block = block;
    return 0;

free_block:
    free(block);
out:
    return ret;
}

int mmc_block_deinit(void* dev)
{
    struct mmc_card* card = (struct mmc_card*)dev;
    struct mmc_block* block = card->block;
#ifdef MULTI_DEVICE
    int disk_id;
#endif

    if (is_card_sdio(card)) {
        /* sdio don't need data */
        unregister_driver(block->node_name);
        goto free_block;
    }

    dev_data_deinit(block);

    if(is_card_mmc(card)|| is_card_sd(card) || is_card_combo(card))
    {
#ifdef MULTI_DEVICE
        disk_id = los_get_diskid_byname(block->node_name);
        los_disk_deinit(disk_id);
#else
        los_disk_deinit(block->node_no);
#endif
    }

    mmc_mutex_delete(block->queue_lock);

    free_blknode_nu(block->node_no);

free_block:
    /*free block */
    free(block);
    return 0;
}

int sector_rw(int dev, FAR const unsigned char *buffer,
        size_t start_sector, unsigned int nsectors,int rw)
{

    struct mmc_block* block = NULL;
    struct inode sec_node;
    int ret = 0;
    if (mmc_mutex_lock(blk_no.flag_lock, MMC_MUTEX_WAIT_DEF_TIME) == MMC_MUTEX_RETURN_TIMEOUT) {
        mmc_err("lock blk_no.flag_lock timeout!\n");
        return 0;
    }
    block = blk_no.blocks[dev];
    if (!block) {
        mmc_err("no device!\n");
        mmc_mutex_unlock(blk_no.flag_lock);
        return -1;
    }
    mmc_mutex_unlock(blk_no.flag_lock);
    sec_node.i_private = block;
    if (0 == rw) {
        /* read */
        ret = mmc_read(&sec_node, (unsigned char *)buffer, start_sector, nsectors);
    } else {
        /* write */
        ret = mmc_write(&sec_node, buffer, start_sector, nsectors);
    }
    return ret;
}

/* eMMC sectors write/read */
unsigned int emmc_raw_write(char * buffer, unsigned int start_sector, unsigned int nsectors)
{
    struct mmc_block* block = NULL;
    struct mmc_card* card = NULL;
    int ret = 0;
    unsigned int i = 0;
    unsigned int sector_now = nsectors;
    for(i = 0;i < blk_no.blk_max_count; i++) {
        block = blk_no.blocks[i];
        if (!block)
            continue;
        card = (struct mmc_card*)block->dev;
        if (is_card_removeable(card))
            continue;
        else
            break;
    }
    if (i == blk_no.blk_max_count) {
        mmc_err("No eMMC found!\n");
        return 0;
    }
    do
    {
        if(sector_now >= 2048) {
            ret = sector_rw(i, buffer, start_sector, 2048, 1);
            if (ret != 2048)
                return ret;
        }
        else {
            ret = sector_rw(i, buffer, start_sector, sector_now, 1);
            if (ret != sector_now)
                return ret;
        }
        sector_now -= ret;
        start_sector += ret;
        buffer += ret<<9;
    } while(sector_now);
    return nsectors;
}

unsigned int emmc_raw_read(char * buffer, unsigned int start_sector, unsigned int nsectors)
{
    struct mmc_block* block = NULL;
    struct mmc_card* card = NULL;
    int ret = 0;
    unsigned int i = 0;
    unsigned int sector_now = nsectors;
    for(i = 0;i < blk_no.blk_max_count; i++) {
        block = blk_no.blocks[i];
        if (!block)
            continue;
        card = (struct mmc_card*)block->dev;
        if (is_card_removeable(card))
            continue;
        else
            break;
    }
    if (i == blk_no.blk_max_count) {
        mmc_err("No eMMC found!\n");
        return 0;
    }
    do
    {
        if(sector_now >= 2048) {
            ret = sector_rw(i, buffer, start_sector, 2048, 0);
            if (ret != 2048)
                return ret;
        }
        else {
            ret = sector_rw(i, buffer, start_sector, sector_now, 0);
            if (ret != sector_now)
                return ret;
        }
        sector_now -= ret;
        start_sector += ret;
        buffer += ret<<9;
    } while(sector_now);
    return nsectors;
}
/* end of file block.c */
