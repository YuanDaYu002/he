/* sdio_func.c begin */

#include "mmc_core.h"

static void sdio_free_func_tuple(struct sdio_func_tuple  *tuple_tail)
{
    struct sdio_func_tuple *tuple_pre_tail = NULL;

    if (!tuple_tail)
        return;
    while(tuple_tail) {
        tuple_pre_tail = tuple_tail;
        tuple_tail = tuple_tail->next;
        free(tuple_pre_tail);
    }
}

void sdio_delete_func(struct mmc_card *card)
{
    uint32_t cnt = 0;
    /* free card'funcs */
    for (; cnt<card->sdio_func_num; cnt++) {
        sdio_free_func_tuple(card->sdio_funcs[cnt]->tuple_link);
        if (card->sdio_funcs[cnt])
        free(card->sdio_funcs[cnt]);
        card->sdio_funcs[cnt] = NULL;
    }
    /* free card's tuple_link */
    sdio_free_func_tuple(card->tuple_link);
}

struct sdio_func *sdio_get_func(uint32_t func_num,
        uint32_t manufacturer_id, uint32_t device_id)
{
    struct mmc_card *card = NULL;
    struct mmc_host *host = NULL;
    struct sdio_func *func = NULL;
    uint32_t i, j;
    uint32_t max_host = get_mmc_max_num();
    for (i=0; i < max_host; i++) {
        host = get_mmc_host(i);
        if (!host)
            continue;
        card = host->card_cur;
        if (!card)
            continue;
        for(j=0; j<=card->sdio_func_num; j++) {
            func = card->sdio_funcs[j];
            if ((func->func_num == func_num) &&
                    (func->manufacturer_id == manufacturer_id) &&
                    (func->device_id == device_id)) {
                return func;
            }
        }
    }
err:
    mmc_printf(MMC_PRINTF_ERR, "no sdio device\n");
    return NULL;
}

int sdio_en_func(struct sdio_func *func)
{
    int err = 0;
    uint8_t reg = 0;
    int cyclenum = 0;

    mmc_printf(MMC_PRINTF_INFO, "SDIO: Enabling device ...\n");
    err = sdio_rw_direct(func->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
    if (err)
        goto out;

    reg |= 1 << (func->func_num);

    err = sdio_rw_direct(func->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
    if (err)
        goto out;
    do {
        err = sdio_rw_direct(func->card, 0, 0, SDIO_CCCR_IORx, 0, &reg);
        if (err)
            goto out;
        if (reg & (1 << func->func_num))
            break;
        err = -ETIME;
        if( ++cyclenum == 10000)
            goto out;
    }while(1);

    mmc_printf(MMC_PRINTF_INFO, "SDIO: Enabled device\n");

    return 0;

out:
    mmc_printf(MMC_PRINTF_WARN, "SDIO: Failed to enable device\n");
    return err;
}

int sdio_dis_func(struct sdio_func *func)
{
    int err = 0;
    uint8_t reg = 0;

    mmc_printf(MMC_PRINTF_INFO, "SDIO: Disabling device...\n");

    err = sdio_rw_direct(func->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
    if (err)
        goto out;

    reg &= ~(1 << func->func_num);

    err = sdio_rw_direct(func->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
    if (err)
        goto out;

    mmc_printf(MMC_PRINTF_INFO, "SDIO: Disabled device\n");

    return 0;

out:
    mmc_printf(MMC_PRINTF_WARN, "SDIO: Failed to disable device\n");
    return -EIO;
}

int sdio_enable_blksz_for_byte_mode(struct sdio_func *func, uint32_t enable)
{
    if(enable)
        set_card_blksz_for_byte_mode(func->card);
    else
        clr_card_blksz_for_byte_mode(func->card);
    return 0;
}
int sdio_set_cur_blk_size(struct sdio_func *func, uint32_t blksz)
{
    int err = 0;
    uint32_t fbr_addr = 0;

    if (blksz > func->card->host->max_blk_size)
        return -EINVAL;

    if (blksz == 0) {
        blksz = min(func->max_blk_size, func->card->host->max_blk_size);
        blksz = min(blksz, 512u);
    }

    fbr_addr = SDIO_FBR_BASE(func->func_num) + SDIO_FBR_BLKSIZE;
    err = sdio_rw_direct(func->card, 1, 0, fbr_addr,
            (blksz & 0xff), NULL);
    if (err)
        return err;

    fbr_addr = SDIO_FBR_BASE(func->func_num) + SDIO_FBR_BLKSIZE + 1;
    err = sdio_rw_direct(func->card, 1, 0, fbr_addr,
            (blksz >> 8) & 0xff, NULL);
    if (err)
        return err;

    func->cur_blk_size = blksz;
    return 0;
}

static int sdio_rw_ext_block(struct sdio_func *func, int write,
        uint32_t addr, int incr_addr, uint8_t *buf, uint32_t size)
{
    int err = 0;
    uint32_t max_blks;
    uint32_t sdio_max_sz;
    uint32_t remainder = size;
    uint32_t blks;

    //sdio_max_sz = get_max_byte_size(func);

    sdio_max_sz = min(func->card->host->max_blk_size,
            func->card->host->max_request_size);

    if (is_card_blksz_for_byte_mode(func->card))
        sdio_max_sz = min(sdio_max_sz, func->cur_blk_size);
    else
        sdio_max_sz = min(sdio_max_sz, func->max_blk_size);
    sdio_max_sz = min(sdio_max_sz, 512u);

    if (func->card->card_reg.cccr.multi_block && (size > sdio_max_sz)) {
        max_blks = min(func->card->host->max_request_size / func->cur_blk_size,
                func->card->host->max_blk_num);
        max_blks = min(max_blks, 511u);

        for (; remainder > func->cur_blk_size; (remainder -= size)) {
            blks = remainder / func->cur_blk_size;
            if (blks > max_blks)
                blks = max_blks;
            size = blks * func->cur_blk_size;

            err = sdio_io_rw_extended(func->card, write,
                    func->func_num, addr, incr_addr, buf,
                    blks, func->cur_blk_size);
            if (err)
                return err;

            buf += size;
            if (incr_addr)
                addr += size;
        }
    }

    for (; remainder > 0; (remainder -= size)) {
        size = min(remainder, sdio_max_sz);

        err = sdio_io_rw_extended(func->card, write, func->func_num, addr,
                incr_addr, buf, 1, size);
        if (err)
            return err;

        buf += size;
        if (incr_addr)
            addr += size;
    }
    return 0;
}

uint8_t sdio_read_byte(struct sdio_func *func, uint32_t addr, int *err)
{
    uint8_t val;
    int ret;

    ret = sdio_rw_direct(func->card, 0, func->func_num, addr, 0, &val);
    *err = ret ;

    if (ret)
        return 0xFF;
    else
        return val;
}

uint8_t sdio_read_byte_ext(struct sdio_func *func, uint32_t addr,
        int *err, uint32_t in)
{
    uint8_t val;
    int ret;

    ret = sdio_rw_direct(func->card, 0, func->func_num, addr, (uint8_t)in, &val);
    *err = ret;

    if (ret)
        return 0xFF;
    else
        return val;
}

void sdio_write_byte(struct sdio_func *func, uint8_t byte, uint32_t addr, int *err)
{
    int ret;

    ret = sdio_rw_direct(func->card, 1, func->func_num, addr, byte, NULL);
    if (err)
        *err = ret;
}

uint8_t sdio_write_byte_raw(struct sdio_func *func, uint8_t write_byte,
        uint32_t addr, int *err)
{
    int ret;
    uint8_t val;

    ret = sdio_rw_direct(func->card, 1, func->func_num, addr,
            write_byte, &val);
    *err = ret;

    if (ret)
        return 0xff;
    return 0;
}

int sdio_read_incr_block(struct sdio_func *func, void *dst,
        uint32_t addr, int size)
{
    return sdio_rw_ext_block(func, 0, addr, 1, dst, size);
}

int sdio_write_incr_block(struct sdio_func *func, uint32_t addr,
        void *src, int size)
{
    return sdio_rw_ext_block(func, 1, addr, 1, src, size);
}

int sdio_read_fifo_block(struct sdio_func *func, void *dst, uint32_t addr,
        int size)
{
    return sdio_rw_ext_block(func, 0, addr, 0, dst, size);
}

int sdio_write_fifo_block(struct sdio_func *func, uint32_t addr, void *src,
        int size)
{
    return sdio_rw_ext_block(func, 1, addr, 0, src, size);
}

uint8_t sdio_func0_read_byte(struct sdio_func *func, uint32_t addr,
        int *err_ret)
{
    int ret;
    uint8_t val;

    mmc_assert(func);

    if (err_ret)
        *err_ret = 0;

    ret = sdio_rw_direct(func->card, 0, 0, addr, 0, &val);
    if (ret) {
        if (err_ret)
            *err_ret = ret;
        return 0xFF;
    }

    return val;
}

void sdio_func0_write_byte(struct sdio_func *func, uint8_t byte, uint32_t addr,
        int *err_ret)
{
    int ret;
    ret = sdio_rw_direct(func->card, 1, 0, addr, byte, NULL);
    if (err_ret)
        *err_ret = ret;
}


/* irq */
static int process_sdio_pending_irqs(struct mmc_card *card)
{
    int i, err, count;
    uint8_t pending;
    struct sdio_func *func;

    func = card->sdio_irq;
    if (func && card->host->is_sdio_irq_pending) {
        func->sdio_irq_handler(func);
        return 1;
    }

    err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_INTx, 0, &pending);
    if (err) {
        mmc_printf(MMC_PRINTF_ERR, "err:%d,while SDIO_CCCR_INTx \n",
                err);
        return err;
    }

    count = 0;
    for (i = 1; i <= 7; i++) {
        if (pending & (1 << i)) {
            func = card->sdio_funcs[i - 1];
            if (!func) {
                mmc_printf(MMC_PRINTF_WARN, "function not exist.\n");
                err = -EINVAL;
            } else if (func->sdio_irq_handler) {
                func->sdio_irq_handler(func);
                count++;
            } else {
                mmc_printf(MMC_PRINTF_WARN, " function no handler\n");
                err = -EINVAL;
            }
        }
    }

    if (count)
        return count;

    return err;
}

static int sdio_irq_thread(void *_card)
{
    struct mmc_card *card = _card;
    struct mmc_host *host = card->host;

    int ret=0;
    do {
        if (host->caps.bits.cap_sdio_irq) {
            mmc_set_sdio_irq(host, 1);
        }
        (void)mmc_event_wait(&card->sdio_event, 1,
                MMC_EVENT_WAIT_FOREVER);
        process_sdio_pending_irqs(card);
        host->is_sdio_irq_pending = FALSE;

    } while (1);

    if (host->caps.bits.cap_sdio_irq) {
        mmc_set_sdio_irq(host, 0);
    }
    return ret;
}
#define SDIO_STACKSIZE    0x2000
static int sdio_card_irq_get(struct mmc_card *card)
{
    struct mmc_host *host = card->host;
    int ret = 0;

    if ((!card->sdio_irq_count++) && (host->caps.bits.cap_sdio_irq)) {
        if (mmc_event_init(&card->sdio_event)) {
            mmc_err("event init fail!");
            ret = -EACCES;
            return ret;
        }
        card->is_sdio_event_useable = true;
        ret = mmc_thread_create(2,
                sdio_irq_thread,
                SDIO_STACKSIZE,
                card,
                "sdio_Task",
                &card->sdio_task);
        if(ret) {
            mmc_err("sdio_Task create fail");
            ret = -EACCES;
            return ret;
        }
    }
    return 0;
}

static int sdio_card_irq_put(struct mmc_card *card)
{
    mmc_assert(card->sdio_irq_count >= 1);
    (card->sdio_irq_count)--;
    if (!card->sdio_irq_count) {
        card->is_sdio_event_useable = false;
        /* delete the task and event */
        if (mmc_thread_delete(card->sdio_task)) {
            mmc_err("can't delete sdio_task,just try again");
            (void)mmc_thread_delete(card->sdio_task);
        }
        mmc_event_delete(&card->sdio_event);
    }
    return 0;
}

static void sdio_card_irq_set(struct mmc_card *card)
{
    struct sdio_func *func;
    int i;

    card->sdio_irq = NULL;
    if ((card->host->caps.bits.cap_sdio_irq) &&
            card->sdio_irq_count == 1) {
        for (i = 0; i < card->sdio_func_num; i++) {
            func = card->sdio_funcs[i];
            if (func && func->sdio_irq_handler) {
                card->sdio_irq = func;
                break;
            }
        }
    }
}

int sdio_require_irq(struct sdio_func *func, sdio_irq_handler_t *handler)
{
    int err;
    uint8_t reg;

    mmc_assert(func);
    mmc_assert(func->card);

    if (func->sdio_irq_handler) {
        mmc_printf(MMC_PRINTF_ERR, "func[%p] IRQ is busy.\n", func);
        return -EBUSY;
    }

    err = sdio_rw_direct(func->card, 0, 0, SDIO_CCCR_IENx, 0, &reg);
    if (err)
        return err;

    reg |= 1;
    reg |= (1 << func->func_num);

    err = sdio_rw_direct(func->card, 1, 0, SDIO_CCCR_IENx, reg, NULL);
    if (err)
        return err;

    func->sdio_irq_handler = handler;
    err = sdio_card_irq_get(func->card);
    if (err)
        func->sdio_irq_handler = NULL;
    sdio_card_irq_set(func->card);

    return err;
}

int sdio_release_irq(struct sdio_func *func)
{
    int err;
    uint8_t reg;

    mmc_assert(func);
    mmc_assert(func->card);

    if (func->sdio_irq_handler) {
        func->sdio_irq_handler = NULL;
        sdio_card_irq_put(func->card);
        sdio_card_irq_set(func->card);
    }

    err = sdio_rw_direct(func->card, 0, 0, SDIO_CCCR_IENx, 0, &reg);
    if (err)
        return err;

    reg &= (~(1 << func->func_num));

    if (!(reg & 0xFE))
        reg = 0;

    err = sdio_rw_direct(func->card, 1, 0, SDIO_CCCR_IENx, reg, NULL);
    if (err)
        return err;

    return 0;
}

/* sdio_func.c */
