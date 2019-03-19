#include "los_hw.h"
#include "linux/interrupt.h"
#include "linux/kernel.h"
#include "dmac_ext.h"
#include "hi_dmac.h"
#include "los_event.h"
#include "hisoc/dmac.h"
#include "asm/delay.h"

void *reg_dmac_base_va;
unsigned int *g_phandlli = NULL;

extern dmac_peripheral  g_peripheral[DMAC_MAX_PERIPHERALS];

extern void dma_cache_clean(int start, int end);
extern void dma_cache_inv(int start, int end);

#define RX  0
#define TX  1

static int dmac_channel[CHANNEL_NUM] = {
#if CHANNEL_NUM >= 1
    0,
#endif

#if CHANNEL_NUM >= 2
    1,
#endif

#if CHANNEL_NUM >= 3
    2,
#endif

#if CHANNEL_NUM >= 4
    3,
#endif
};

#define CLR_INT(i) ((*(uint32_t *)(DMAC_REG_BASE+0x008)) = (1<<i))

int g_channel_status[CHANNEL_NUM];

#define     DMAC_EVENTT_TYPE       0x01
EVENT_CB_S  dmac_event[CHANNEL_NUM];

static dmac_lli *pheadlliList[CHANNEL_NUM]  = {

#if CHANNEL_NUM >= 1
    NULL,
#endif

#if CHANNEL_NUM >= 2
    NULL,
#endif

#if CHANNEL_NUM >= 3
    NULL,
#endif

#if CHANNEL_NUM >= 4
    NULL,
#endif
};


static dmac_lli_bak *plliList_bak[CHANNEL_NUM] = {

#if CHANNEL_NUM >= 1
    NULL,
#endif

#if CHANNEL_NUM >= 2
    NULL,
#endif

#if CHANNEL_NUM >= 3
    NULL,
#endif

#if CHANNEL_NUM >= 4
    NULL,
#endif
};
/*
static unsigned int config_register_bak = 0;
static unsigned int sync_register_bak = 0;
*/
/* #define DMA_DEBUG */
#ifdef DMA_DEBUG
#define dma_debug dprintf
#else
#define dma_debug(fmt, ...) do {} while (0);
#endif

#define dma_err dprintf

/*
 * Define Memory range
 */
mem_addr mem_num[MEM_MAX_NUM] = {
    {DDRAM_ADRS, DDRAM_SIZE}
    //{FLASH_BASE, FLASH_SIZE}
};

typedef void REG_ISR(int dma_chn, int dma_status) ;
REG_ISR *function[CHANNEL_NUM];

/*funciton declaration*/

static void dmac_channel_close(unsigned int channel, unsigned int channel_status);
/*
 * memory address validity check
 */
static int mem_check_valid(unsigned int addr)
{
    unsigned int cnt;

    for (cnt = 0; cnt < MEM_MAX_NUM; cnt++) {
        if ((addr >= mem_num[cnt].addr_base) &&
                (addr <= (mem_num[cnt].addr_base + mem_num[cnt].size)))
            return 0;
    }

    return -1;
}

/*
 * dmac interrupt handle function
 */
irqreturn_t dmac_isr(int irq, void *dev_id)
{
    unsigned int channel_status;
    unsigned int channel_tc_status, channel_err_status;
    unsigned int i;

    dma_debug("isr entry\n");
    /* read the status of current interrupt */
    dmac_readw(DMAC_INTSTATUS, channel_status);

    /* decide which channel has trigger the interrupt */
    for (i = 0; i < DMAC_MAX_CHANNELS; i++) {
        if ((channel_status >> i) & 0x01) {

            dmac_readw(DMAC_INTTCSTATUS, channel_tc_status);
            dmac_readw(DMAC_INTERRORSTATUS, channel_err_status);
            //CLR_INT(i); /* clear transfer complete status */
            //dma_debug("channel_tc_status 0x%x, channel_err_status 0x%x\n", channel_tc_status, channel_err_status);

            if ((g_channel_status[i] == DMAC_CHN_VACANCY)
                    && ((function[i]) == NULL)) {
                if (0x1 == ((channel_tc_status >> i) & 0x1))
                    dmac_writew(DMAC_INTTCCLEAR, (0x1<<i));
                else if (0x1 == ((channel_err_status>>i)&0x1))
                    dmac_writew(DMAC_INTERRCLR, (0x1<<i));
                continue;
            }

            /*
             * save the current channel transfer
             * status to g_channel_status[i]
             */
            if (0x1 == ((channel_tc_status >> i) & 0x1)) {
                g_channel_status[i] = DMAC_CHN_SUCCESS;
                dmac_writew(DMAC_INTTCCLEAR, (0x1 << i));
            } else if (0x1 == ((channel_err_status>>i)&0x1)) {
                g_channel_status[i] = -DMAC_CHN_ERROR;
                dmac_writew(DMAC_INTERRCLR, (0x01 << i));
            } else
                dma_err("Isr Error in DMAC_IntHandeler"\
                        "%d! channel\n", i);

            if ((function[i]) != NULL) {
                function[i](i, g_channel_status[i]);
                LOS_EventWrite(&dmac_event[i], DMAC_EVENTT_TYPE);
            }
            //dma_debug("g_channel_status:%d \n", g_channel_status[i]);
        }
    }
    return IRQ_RETVAL(1);
}

/*
 * update the state of channels
 */
static int dma_update_status(unsigned int channel)
{
    unsigned int channel_status;
    unsigned int channel_tc_status[3];
    unsigned int channel_err_status[3];
    unsigned int i = channel, j;
    unsigned int timeout_tick= 0;

    timeout_tick = LOS_TickCountGet() + LOS_MS2Tick(5000); //5000 ms 

    if (g_channel_status[i])
        return g_channel_status[i];

    while (1) {
        for (j = 0; j < 3; j++) {
            dmac_readw(DMAC_INTTCSTATUS, channel_status);
            channel_tc_status[j] = (channel_status >> i) & 0x1;
            dmac_readw(DMAC_INTERRORSTATUS, channel_status);
            channel_err_status[j] = (channel_status >> i) & 0x1;
        }

        if ((channel_tc_status[0] == 0x1)
                && (channel_tc_status[1] == 0x1)
                && (channel_tc_status[2] == 0x1)) {
            g_channel_status[i] = DMAC_CHN_SUCCESS;
            dmac_writew(DMAC_INTTCCLEAR, (0x1 << i));
            break;
        }

        if ((channel_err_status[0] == 0x1)
                && (channel_err_status[1] == 0x1)
                && (channel_err_status[2] == 0x1)) {
            g_channel_status[i] = -DMAC_CHN_ERROR;
            dma_err("Error in DMAC %d finish!\n", i);
            dmac_writew(DMAC_INTERRCLR, (0x01 << i));
            break;
        }
        if (LOS_TickCountGet() > timeout_tick ) {
            dma_err("DMA Timeout!\n");
            g_channel_status[i] = -DMAC_CHN_TIMEOUT;
            break;
        }
    }

    return g_channel_status[i];
}


/*
 * check the state of channels
 */
static int dmac_check_over(unsigned int channel)
{
    if (-DMAC_CHN_ERROR == g_channel_status[channel]) {
        dma_err("Error transfer %d finished\n", channel);
        dmac_writew(DMAC_CxCONFIG(channel), DMAC_CxDISABLE);
        g_channel_status[channel] = DMAC_CHN_VACANCY;
        return -DMAC_CHN_ERROR;
    } else if (DMAC_NOT_FINISHED == g_channel_status[channel])
        return DMAC_NOT_FINISHED;
    else if (DMAC_CHN_ALLOCAT == g_channel_status[channel])
        return DMAC_CHN_ALLOCAT;
    else if (DMAC_CHN_VACANCY == g_channel_status[channel])
        return DMAC_CHN_VACANCY;
    else if (-DMAC_CHN_TIMEOUT == g_channel_status[channel]) {
        dma_err("transfer %d timeout!\n", channel);
        return -DMAC_CHN_TIMEOUT;
    } else if (DMAC_CHN_SUCCESS == g_channel_status[channel])
        /*The transfer of Channel %d has finished successfully!*/
        return DMAC_CHN_SUCCESS;
    else {
        dmac_writew(DMAC_CxCONFIG(channel), DMAC_CxDISABLE);
        g_channel_status[channel] = DMAC_CHN_VACANCY;
        return -DMAC_CHN_ERROR;
    }
}


/*
 * allocate channel.
 */
unsigned int dmac_channel_allocate(void *pisr)
{
    unsigned int i, channelinfo;

    for (i = 0; i < CHANNEL_NUM; i++)
        dmac_check_over(dmac_channel[i]);

    dmac_readw(DMAC_ENBLDCHNS, channelinfo);
    channelinfo = channelinfo & 0x00ff;

    for (i = 0; i < CHANNEL_NUM; i++) {
        if (g_channel_status[dmac_channel[i]] == DMAC_CHN_VACANCY) {
            channelinfo = channelinfo >> dmac_channel[i];
            /* channel is disabled currently */
            if (0x00 == (channelinfo & 0x01)) {
                /*clear the interrupt in this channel */
                dmac_writew(DMAC_INTERRCLR,
                        (0x01 << dmac_channel[i]));
                dmac_writew(DMAC_INTTCCLEAR,
                        (0x01 << dmac_channel[i]));

                g_channel_status[dmac_channel[i]]
                    = DMAC_CHN_ALLOCAT;
                //dma_debug("allocate channel is %d......\n", i);
                return dmac_channel[i];
            }
        }
    }
    dma_err("[%s:%d]no to alloc\n", __func__, __LINE__);
    return DMAC_CHANNEL_INVALID;
}


int dmac_register_isr(unsigned int channel, void *pisr)
{
    if (channel >= CHANNEL_NUM) {
        dma_err("invalid dma channel %d !\n", channel);
        return -1;
    }

    /*clear the interrupt in this channel */
    dmac_writew(DMAC_INTERRCLR, (0x01 << channel));
    dmac_writew(DMAC_INTTCCLEAR, (0x01 << channel));

    function[channel] = (void *)pisr;
    g_channel_status[channel] = DMAC_CHN_ALLOCAT;

    return 0;
}

/*
 * close channel
 */
static void dmac_channel_close(unsigned int channel, unsigned int channel_status)
{
    if (pheadlliList[channel]) {
        dma_debug("free: 0x%x\n", pheadlliList[channel]);
        free(pheadlliList[channel]);
    }
    //dma_debug("%s: ch:%d,sta:%d, head:%p \n",__func__, channel, channel_status, pheadlliList[channel]);
    pheadlliList[channel] = NULL;
    function[channel] = NULL;
    g_channel_status[channel] = DMAC_CHN_VACANCY;
    dmac_writew(DMAC_CxCONFIG(channel), DMAC_CxDISABLE);
}

/*
 * free channel
 */
int dmac_channel_free(unsigned int channel)
{
    if (channel >= CHANNEL_NUM)
        return -EINVAL;

    if (pheadlliList[channel]) {
        //dma_debug("free: 0x%x\n", pheadlliList[channel]);
        free(pheadlliList[channel]);
    }
    //dma_debug("%s: ch:%d, head:%p \n",__func__, channel, pheadlliList[channel]);
    pheadlliList[channel] = NULL;

    function[channel] = NULL;
    g_channel_status[channel] = DMAC_CHN_VACANCY;
    dmac_writew(DMAC_CxCONFIG(channel), DMAC_CxDISABLE);
    return 0;
}

int dmac_check_request(unsigned int peripheral_addr,
        int direction)
{
    int i;
    /* check request pipe with peripheral_addr */
    for (i = direction; i < DMAC_MAX_PERIPHERALS; i = i + 2) {
        if (g_peripheral[i].peri_addr == peripheral_addr)
            return i;
    }

    dma_err("Invalid devaddr\n");

    return -1;
}

unsigned int dmac_get_current_dest(unsigned int channel)
{
    unsigned int reg_value = 0;
    dmac_readw(DMAC_CxDESTADDR(channel), reg_value);
    return reg_value;
}

/*
 * alloc_dma_lli_space
 * output:
 */
int allocate_dmalli_space(dmac_lli **ppheadlli, unsigned int length)
{
    unsigned int *alloc_addr = NULL;
    unsigned int lli_num = 0;
    unsigned int alloc_length = 0;

    lli_num = length/(DMAC_TRANS_SIZE);

    if ((length % (DMAC_TRANS_SIZE)) != 0) {
        lli_num++;
    }
    alloc_length = lli_num * sizeof(dmac_lli);
    alloc_length = ALIGN(alloc_length, CACHE_ALIGNED_SIZE);
    alloc_addr = (unsigned int *)memalign(CACHE_ALIGNED_SIZE, alloc_length);
    if (!alloc_addr)
    {
        dma_err("can't malloc llimem for dma!\n ");
        return -1;
    }
    dma_debug("lli_num:%d, alloc_length:%d",lli_num,alloc_length);
    memset(alloc_addr, 0, alloc_length);
    *ppheadlli =(dmac_lli *)alloc_addr;
    dma_debug("dma malloc mem %p,lli_num:%d,length:%d, ppheadlli:%p\n", alloc_addr, lli_num, alloc_length, *ppheadlli);
    //return ALIGN(alloc_length, CACHE_ALIGNED_SIZE);
    return alloc_length;
}


/*
 *    free_dma_lli_space
 */
void free_dmalli_space(dmac_lli *ppheadlli)
{
    if(ppheadlli) {
        free(ppheadlli);
        ppheadlli = NULL;
    }
}


/*
 * config register for memory to memory DMA tranfer without LLI
 * note:
 * the channel is not actually started yet, you need to call
 * dmac_channelstart to start it up.
 */
int dmac_start_m2m(unsigned int channel,
        unsigned int psource,
        unsigned int pdest,
        unsigned int uwnumtransfers)
{
    unsigned int uwchannel_num, tmp_trasnsfer;

    //dma_debug("dmac_start_m2m: dmac_start_m2m[0x%x]\n", dmac_start_m2m);
    if (mem_check_valid(psource) | mem_check_valid(pdest)) {
        dma_err("Invalid dma addr psrc = 0x%x, pdest = 0x%x!\n",
                psource, pdest);
        return -EINVAL;
    }

    if (uwnumtransfers > (DMAC_MAXTRANSFERSIZE << 2)) {
        dma_err("Invalidate transfer size,size=%x\n",
                uwnumtransfers);
        return -EINVAL;
    }

    uwchannel_num = channel;

    if ((uwchannel_num == DMAC_CHANNEL_INVALID)
            || (uwchannel_num > CHANNEL_NUM)) {
        dma_err("invalid dma channel %d\n", uwchannel_num);
        return -EFAULT;
    }

    /* dmac_writew (DMAC_CxCONFIG(uwchannel_num), DMAC_CxDISABLE); */
    dmac_writew(DMAC_CxSRCADDR(uwchannel_num), psource);
    dmac_writew(DMAC_CxDESTADDR(uwchannel_num), pdest);
    dmac_writew(DMAC_CxLLI(uwchannel_num), 0);
    tmp_trasnsfer = (uwnumtransfers) & 0xfff;
    tmp_trasnsfer = tmp_trasnsfer | (DMAC_CxCONTROL_M2M & (~0xfff));
    //dma_debug("ctrl:0x%X \n", tmp_trasnsfer);
    dmac_writew(DMAC_CxCONTROL(uwchannel_num), tmp_trasnsfer);
    dmac_writew(DMAC_CxCONFIG(uwchannel_num), DMAC_CxCONFIG_M2M);
    return 0;
}


/*
 * channel enable
 * start a dma transfer immediately
 */
int dmac_channelstart(unsigned int u32channel)
{
    unsigned int reg_value;

    if (u32channel >= DMAC_MAX_CHANNELS) {
        dma_err("channel larger %d\n", DMAC_MAX_CHANNELS);
        return -EINVAL;
    }

    g_channel_status[u32channel] = DMAC_NOT_FINISHED;
    dmac_readw(DMAC_CxCONFIG(u32channel), reg_value);
    reg_value &= 0x0007ffff;
    dmac_writew(DMAC_CxCONFIG(u32channel),
            (reg_value | DMAC_CHANNEL_ENABLE));

    return 0;
}

/*
 * wait for transfer end
 */
int dmac_wait(unsigned int channel)
{
    int ret_result;

    if (channel >= CHANNEL_NUM)
        return -1;

    while (1) {
        ret_result = dma_update_status(channel);
        if (ret_result == -DMAC_CHN_ERROR) {
            dma_err("Transfer Error.\n");
            return -1;
        } else  if (ret_result == DMAC_NOT_FINISHED)
            udelay(10);
        else if (ret_result == DMAC_CHN_SUCCESS)
            return DMAC_CHN_SUCCESS;
        else if (ret_result == DMAC_CHN_VACANCY)
            return DMAC_CHN_SUCCESS;
        else if (ret_result == -DMAC_CHN_TIMEOUT) {
            dma_err("Timeout.\n");
            dmac_writew(DMAC_CxCONFIG(channel), DMAC_CxDISABLE);
            g_channel_status[channel] = DMAC_CHN_VACANCY;
            return -1;
        }
    }
}
/*
 * wait for irq event write, when transfer finished
 *
 * */
int dmac_wait_for_irq(unsigned int channel)
{
    unsigned int ret_result = 0;

    if (channel >= CHANNEL_NUM)
        return -1;
    ret_result = LOS_EventRead(&dmac_event[channel], DMAC_EVENTT_TYPE,
            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, LOS_MS2Tick(5000)); //5000ms
    if (DMAC_EVENTT_TYPE == ret_result) {
        ret_result = DMAC_CHN_SUCCESS;
    } else {
        dma_err("Timeout.\n");
        ret_result = -1;
    }

    return ret_result;
}

/*
 * buile LLI for memory to memory DMA tranfer
 */
int dmac_buildllim2m(dmac_lli *ppheadlli,
        unsigned int psource,
        unsigned int pdest,
        unsigned int totaltransfersize,
        unsigned int numtransfers)
{
    unsigned int lli_num = 0;
    unsigned int last_lli = 0;
    unsigned int srcaddr, destaddr;
    dmac_lli *pheadlli = NULL;
    unsigned int j;

    if (mem_check_valid(psource) | mem_check_valid(pdest)) {
        dma_err("Invalid dma addr psrc = 0x%x, pdest = 0x%x!\n",
                psource, pdest);
        return -EINVAL;
    }

    if (ppheadlli == NULL) {
        dma_err("ppheadlli is NULL!\n");
        return -ENOMEM;
    }

    pheadlli = ppheadlli;

    lli_num = (totaltransfersize / numtransfers);
    if ((totaltransfersize % numtransfers) != 0)
        last_lli = 1, ++lli_num;

    //building  lli
    //dma_debug("lli_num:%d\n", lli_num);
    for (j = 0; j < lli_num; j++) {
        //dma_debug("cnt:%d, pheadlli: %p, length:%d \n", j, pheadlli, numtransfers);
        //dma_debug("srcpheadlli:%p, ", &(pheadlli->src_addr));
        srcaddr = (psource + (j*numtransfers));
        //dma_debug("srcaddr: 0x%X,\n", srcaddr);
        dmac_writew(&(pheadlli->src_addr), srcaddr);
        //dest
        //dma_debug("dstpheadlli:%p", &(pheadlli->dst_addr));
        destaddr = (pdest + (j*numtransfers));
        //dma_debug("destaddr:0x%X\n", destaddr);
        dmac_writew(&(pheadlli->dst_addr), destaddr);
        //next lli
        //dma_debug("&lli:%p \n ", &(pheadlli->next_lli));
        if (j == (lli_num - 1))
            dmac_writew(&(pheadlli->next_lli), 0);
        else {
            //dma_debug("lli: 0x%X,\n", (((unsigned int)(pheadlli+1) & (~0x3)) | DMAC_CxLLI_LM));
            dmac_writew(&(pheadlli->next_lli), (((unsigned int)(pheadlli+1) & (~0x3)) | DMAC_CxLLI_LM));
        }
        //dma_debug("pheadlli->next_lli:0x%X\n", (pheadlli->next_lli));

        //control, 1 trans_size = 4Bytes(32bit mode)
        if ((j == (lli_num - 1)) && (last_lli == 0)) {
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    ((DMAC_CxCONTROL_LLIM2M & (~0xfff))
                     | (numtransfers >> 2)
                     | DMAC_CxCONTROL_INT_EN));

        } else if ((j == (lli_num - 1)) && (last_lli == 1)) {
            /*dma_debug("last,ctrl:0x%x\n",((DMAC_CxCONTROL_LLIM2M & (~0xfff))
              | ((totaltransfersize % numtransfers) >> 2)
              | DMAC_CxCONTROL_INT_EN));*/
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    ((DMAC_CxCONTROL_LLIM2M & (~0xfff))
                     | ((totaltransfersize % numtransfers) >> 2)
                     | DMAC_CxCONTROL_INT_EN));
        } else
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    (((DMAC_CxCONTROL_LLIM2M & (~0xfff)) | (numtransfers >> 2))
                     & 0x7fffffff));

        pheadlli++;
    }

    return 0;
}

/*
 * buile LLI for peripheral to memory DMA tranfer
 */
int dmac_buildllip2m(dmac_lli *ppheadlli,
        unsigned int peripheralid,
        unsigned int pdest,
        unsigned int totaltransfersize,
        unsigned int numtransfers)
{
    unsigned int lli_num = 0;
    unsigned int last_lli = 0;
    unsigned int srcaddr, destaddr;
    dmac_lli *pheadlli = NULL;
    unsigned int j;

    if  (mem_check_valid(pdest)) {
        dma_err("Invalid dma addr ,pdest = 0x%x!\n", pdest);
        return -EINVAL;
    }
    if (peripheralid >= DMAC_MAX_PERIPHERALS) {
        dma_err("Invalid peripheralid,id= %d!\n", pdest);
        return -EINVAL;
    }
    if (ppheadlli == NULL) {
        dma_err("ppheadlli is NULL!\n");
        return -ENOMEM;
    }

    pheadlli = ppheadlli;

    lli_num = (totaltransfersize / numtransfers);
    if ((totaltransfersize % numtransfers) != 0)
        last_lli = 1, ++lli_num;

    //building  lli
    for (j = 0; j < lli_num; j++) {
        //dma_debug("cnt:%d, pheadlli: %p, length:%d \n", j, pheadlli, numtransfers);
        //dma_debug("srcpheadlli:%p, ", &(pheadlli->src_addr));
        srcaddr = g_peripheral[peripheralid].peri_addr;                                //(psource + (j*numtransfers));
        //dma_debug("srcaddr: 0x%X,\n", srcaddr);
        dmac_writew(&(pheadlli->src_addr), srcaddr);
        //dest
        //dma_debug("dstpheadlli:%p", &(pheadlli->dst_addr));
        destaddr = (pdest + (j*numtransfers));
        //dma_debug("destaddr:0x%X\n", destaddr);
        dmac_writew(&(pheadlli->dst_addr), destaddr);
        //next lli
        //dma_debug("&lli:%p \n ", &(pheadlli->next_lli));
        if (j == (lli_num - 1)) {
            //dmac_writew(&(pheadlli->next_lli), 0);
            dmac_writew(&(pheadlli->next_lli), (((unsigned int)ppheadlli & (~0x3)) | DMAC_CxLLI_LM));
        }
        else {
            //dma_debug("lli: 0x%X,\n", (((unsigned int)(pheadlli+1) & (~0x3)) | DMAC_CxLLI_LM));
            dmac_writew(&(pheadlli->next_lli),
                    (((unsigned int)(pheadlli+1) & (~0x3)) | DMAC_CxLLI_LM));
        }
        //control, 1 trans_size = 1Byte(8bit mode)
        if ((j == (lli_num - 1)) && (last_lli == 0)) {
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    ((g_peripheral[peripheralid].transfer_ctrl & (~0xfff))
                     | (numtransfers & 0xfff)));

        } else if ((j ==(lli_num - 1)) && (last_lli == 1)) {
            /*dma_debug("last,ctrl:0x%x\n",((DMAC_CxCONTROL_LLIM2M & (~0xfff))
              | ((totaltransfersize % numtransfers) >> 2)
              | DMAC_CxCONTROL_INT_EN));*/
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    ((g_peripheral[peripheralid].transfer_ctrl & (~0xfff))
                     | ((totaltransfersize % (numtransfers & 0xfff)))));
        } else
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    (((g_peripheral[peripheralid].transfer_ctrl
                       & (~0xfff)) | (numtransfers & 0xfff))
                     & 0x7fffffff));

        pheadlli++;
    }

    return 0;
}

/*
 * disable channel
 * used before the operation of register configuration
 */
int dmac_channelclose(unsigned int channel)
{
    unsigned int reg_value, count;

    if (channel >= DMAC_MAX_CHANNELS) {
        dma_err("channel larger than total.\n");
        return -EINVAL;
    }

    dmac_readw(DMAC_CxCONFIG(channel), reg_value);

#define CHANNEL_CLOSE_IMMEDIATE
#ifdef CHANNEL_CLOSE_IMMEDIATE
    reg_value &= 0xFFFFFFFE;
    dmac_writew(DMAC_CxCONFIG(channel) , reg_value);
#else
    reg_value |= DMAC_CONFIGURATIONx_HALT_DMA_ENABLE;
    /*ignore incoming dma request*/
    dmac_writew(DMAC_CxCONFIG(channel), reg_value);
    dmac_readw(DMAC_CxCONFIG(channel), reg_value);
    /*if FIFO is empty*/
    while ((reg_value & DMAC_CONFIGURATIONx_ACTIVE)
            == DMAC_CONFIGURATIONx_ACTIVE)
        dmac_readw(DMAC_CxCONFIG(channel), reg_value);
    reg_value &= 0xFFFFFFFE;
    dmac_writew(DMAC_CxCONFIG(channel), reg_value);
#endif

    dmac_readw(DMAC_ENBLDCHNS, reg_value);
    reg_value = reg_value & 0x00ff;
    count = 0;
    while (((reg_value >> channel) & 0x1) == 1) {
        dmac_readw(DMAC_ENBLDCHNS, reg_value);
        reg_value = reg_value & 0x00ff;
        if (count++ > 10000) {
            dma_err("close failure.\n");
            return -1;
        }
    }
    g_channel_status[channel] = DMAC_CHN_VACANCY;
    return 0;
}


/*
 *    load configuration from LLI for peripheral to memory
 */
int dmac_start_llip2m(unsigned int channel, dmac_lli *pfirst_lli, unsigned int peri_id)
{
    unsigned int uwchannel_num;
    dmac_lli  *plli;
    unsigned int first_lli;
    if (NULL == pfirst_lli) {
        dma_err("Invalidate LLI head!\n");
        return -EFAULT;
    }

    uwchannel_num = channel;
    plli = pfirst_lli;
    if ((uwchannel_num == DMAC_CHANNEL_INVALID)
            || (uwchannel_num > CHANNEL_NUM)) {
        dma_err("invalid dma channel %d\n", uwchannel_num);
        return -EINVAL;
    }
    //dma_debug("%s: ", __func__);
    /*dma_debug("src:0x%X,dst:0x%X,lli:0x%X,ctrl:0x%X \n",
      (plli->src_addr),(plli->dst_addr),(plli->next_lli),(plli->lli_transfer_ctrl));
      */
    dmac_writew(DMAC_CxCONFIG(uwchannel_num), DMAC_CxDISABLE);
    dmac_writew(DMAC_CxSRCADDR(uwchannel_num),
            (unsigned int)(plli->src_addr));
    dmac_writew(DMAC_CxDESTADDR(uwchannel_num),
            (unsigned int)(plli->dst_addr));
    dmac_writew(DMAC_CxLLI(uwchannel_num),
            (unsigned int)(plli->next_lli));
    dmac_writew(DMAC_CxCONTROL(uwchannel_num),
            (unsigned int)(plli->lli_transfer_ctrl));
    //dma_debug("peri:%d,cfg:0x%X \n", peri_id, g_peripheral[peri_id].transfer_cfg);
    dmac_writew(DMAC_CxCONFIG(uwchannel_num), g_peripheral[peri_id].transfer_cfg);

    return 0;
}


/*
 *    load configuration from LLI for memory to memory
 */
int dmac_start_llim2m(unsigned int channel, dmac_lli *pfirst_lli)
{
    unsigned int uwchannel_num;
    dmac_lli  *plli;
    unsigned int first_lli;
    if (NULL == pfirst_lli) {
        dma_err("Invalidate LLI head!\n");
        return -EFAULT;
    }

    uwchannel_num = channel;
    plli = pfirst_lli;
    if ((uwchannel_num == DMAC_CHANNEL_INVALID)
            || (uwchannel_num > CHANNEL_NUM)) {
        dma_err("invalid dma channel %d\n", uwchannel_num);
        return -EINVAL;
    }
    //dma_debug("%s: ", __func__);
    /*dma_debug("src:0x%X,dst:0x%X,lli:0x%X,ctrl:0x%X \n",
      (plli->src_addr),(plli->dst_addr),(plli->next_lli),(plli->lli_transfer_ctrl));*/
    dmac_channelclose(uwchannel_num);
    dmac_writew(DMAC_INTTCCLEAR, (0x1 << uwchannel_num));
    dmac_writew(DMAC_INTERRCLR, (0x1 << uwchannel_num));
    dmac_writew(DMAC_SYNC, 0x0);

    dmac_writew(DMAC_CxCONFIG(uwchannel_num), DMAC_CxDISABLE);
    dmac_writew(DMAC_CxSRCADDR(uwchannel_num),
            (unsigned int)(plli->src_addr));
    dmac_writew(DMAC_CxDESTADDR(uwchannel_num),
            (unsigned int)(plli->dst_addr));
    dmac_writew(DMAC_CxLLI(uwchannel_num),
            (unsigned int)(plli->next_lli));
    dmac_writew(DMAC_CxCONTROL(uwchannel_num),
            (unsigned int)(plli->lli_transfer_ctrl));
    dmac_writew(DMAC_CxCONFIG(uwchannel_num), DMAC_CxCONFIG_LLIM2M);

    return 0;
}


/*
 * load configuration from LLI for memory and peripheral
 */
int dmac_start_llim2p(unsigned int channel,
        unsigned int *pfirst_lli,
        unsigned int uwperipheralid)
{
    unsigned int uwchannel_num;
    dmac_lli plli;
    unsigned int first_lli;
    unsigned int temp = 0;

    if (NULL == pfirst_lli) {
        dma_err("Invalid LLI head!\n");
        return -EINVAL;
    }

    uwchannel_num = channel;
    if ((uwchannel_num == DMAC_CHANNEL_INVALID)
            || (uwchannel_num > CHANNEL_NUM)) {
        dma_err("invalid dma channel %d\n", uwchannel_num);
        return -EINVAL;
    }

    memset(&plli, 0, sizeof(plli));
    first_lli = (unsigned int)pfirst_lli[1];
    dmac_readw(first_lli, plli.src_addr);
    dmac_readw(first_lli+4, plli.dst_addr);
    dmac_readw(first_lli+8, plli.next_lli);
    dmac_readw(first_lli+12, plli.lli_transfer_ctrl);

    dmac_channelclose(uwchannel_num);
    dmac_writew(DMAC_INTTCCLEAR, (0x1<<uwchannel_num));
    dmac_writew(DMAC_INTERRCLR, (0x1<<uwchannel_num));
    dmac_writew(DMAC_SYNC, 0x0);

    dmac_readw(DMAC_CxCONFIG(uwchannel_num), temp);
    dmac_writew(DMAC_CxCONFIG(uwchannel_num), temp|DMAC_CxDISABLE);
    dmac_writew(DMAC_CxSRCADDR(uwchannel_num), plli.src_addr);
    dmac_writew(DMAC_CxDESTADDR(uwchannel_num), plli.dst_addr);
    dmac_writew(DMAC_CxLLI(uwchannel_num), plli.next_lli);
    dmac_writew(DMAC_CxCONTROL(uwchannel_num), plli.lli_transfer_ctrl);

    return 0;
}


/*
 * enable memory and peripheral dma transfer
 * note:
 *  it is necessary to call dmac_channelstart to enable channel
 */
int dmac_start_m2p(unsigned int channel,
        unsigned int memaddr,
        unsigned int uwperipheralid,
        unsigned int uwnumtransfers,
        unsigned int next_lli_addr)
{

    unsigned int uwtrans_control = 0;
    unsigned int  tmp;
    unsigned int uwdst_addr = 0, uwsrc_addr = 0;
    unsigned int uwwidth;
    unsigned int uwchannel_num;

    if (mem_check_valid(memaddr)){
        dma_err("Invalid dma addr memaddr = 0x%x!\n", memaddr);
        return -EINVAL;
    }

    if ((uwperipheralid > 15)) {
        dma_err("Invalid peripheral id%x\n", uwperipheralid);
        return -EINVAL;
    }

    uwchannel_num = channel;
    if ((uwchannel_num == DMAC_CHANNEL_INVALID)
            || (uwchannel_num > CHANNEL_NUM)) {
        dma_err("invalid channel %d\n", uwchannel_num);
        return -EFAULT;
    }

    /* must modified with different peripheral */
    uwwidth = g_peripheral[uwperipheralid].transfer_width;

    /*
     * check transfer direction
     * even number-->TX, odd number-->RX
     */
    uwsrc_addr = memaddr;
    uwdst_addr = (unsigned int)(g_peripheral[uwperipheralid].peri_addr);

    tmp = uwnumtransfers >> uwwidth;
    if (tmp & (~0x0fff)) {
        dma_err("Invalidate size%x\n", uwnumtransfers);
        return -EINVAL;
    }

    tmp = tmp & 0xfff;
    uwtrans_control = tmp |    (g_peripheral[uwperipheralid].transfer_ctrl & (~0xfff));
    dma_debug("(in)uwdst_addr = %#x\n", (unsigned int)uwdst_addr);
    dmac_writew(DMAC_INTTCCLEAR, (0x1<<uwchannel_num));
    dmac_writew(DMAC_INTERRCLR, (0x1<<uwchannel_num));
    dmac_writew(DMAC_CxSRCADDR(uwchannel_num), (unsigned int)uwsrc_addr);
    dmac_writew(DMAC_CxDESTADDR(uwchannel_num), (unsigned int)uwdst_addr);
    dmac_writew(DMAC_CxCONTROL(uwchannel_num), (unsigned int)uwtrans_control);
    dmac_writew(DMAC_CxCONFIG(uwchannel_num), (g_peripheral[uwperipheralid].transfer_cfg));

    return 0;
}

/*
 * enable memory and peripheral dma transfer
 * note:
 * it is necessary to call dmac_channelstart to enable channel
 */
int dmac_start_p2m(unsigned int channel,
        unsigned int memaddr,
        unsigned int uwperipheralid,
        unsigned int uwnumtransfers,
        unsigned int next_lli_addr)
{
    unsigned int uwtrans_control = 0;
    unsigned int  tmp;
    unsigned int uwdst_addr = 0, uwsrc_addr = 0;
    unsigned int uwwidth;
    unsigned int uwchannel_num;

    if (mem_check_valid(memaddr)) {
        dma_err("Invalidate dma addr memaddr = 0x%x!\n", memaddr);
        return -EINVAL;
    }

    if ((uwperipheralid > 15)) {
        dma_err("Invalid peripheral id%x\n", uwperipheralid);
        return -EINVAL;
    }

    uwchannel_num = channel;
    if ((uwchannel_num == DMAC_CHANNEL_INVALID)
            || (uwchannel_num > CHANNEL_NUM)) {
        dma_err("failure alloc\n");
        return -EFAULT;
    }

    /* must modified with different peripheral */
    uwwidth = g_peripheral[uwperipheralid].transfer_width;

    /*
     * check transfer direction
     * even number-->TX, odd number-->RX
     */
    uwsrc_addr = (unsigned int)(g_peripheral[uwperipheralid].peri_addr);
    uwdst_addr = memaddr;

    tmp = uwnumtransfers >> uwwidth;
    if (tmp & (~0x0fff)) {
        dma_err("Invalidate size%x\n", uwnumtransfers);
        return -EINVAL;
    }

    tmp = tmp & 0xfff;
    uwtrans_control = tmp | (g_peripheral[uwperipheralid].transfer_ctrl & (~0xfff));
    dma_debug("(in)uwdst_addr = %#x\n", (unsigned int)uwdst_addr);
    dmac_writew(DMAC_INTTCCLEAR, (0x1<<uwchannel_num));
    dmac_writew(DMAC_INTERRCLR, (0x1<<uwchannel_num));
    dmac_writew(DMAC_CxSRCADDR(uwchannel_num), (unsigned int)uwsrc_addr);
    dmac_writew(DMAC_CxDESTADDR(uwchannel_num), (unsigned int)uwdst_addr);
    dmac_writew(DMAC_CxCONTROL(uwchannel_num), (unsigned int)uwtrans_control);

    dmac_writew(DMAC_CxCONFIG(uwchannel_num), (g_peripheral[uwperipheralid].transfer_cfg));

    return 0;
}


/*
 * execute memory to memory dma transfer with LLI
 */
int dmac_llim2m_transfer(unsigned int source,
        unsigned int dest,
        unsigned int length)
{
    int ret = 0;
    unsigned int ret_result = 0;
    int lli_length = 0;
    unsigned int lli_num = 0;
    unsigned int chnn;
    dmac_lli *pllihead = NULL;

    if (mem_check_valid(source) | mem_check_valid(dest)) {
        dma_err("Invalid dma addr psrc = 0x%x, pdest = 0x%x!\n",
                source, dest);
        return -EINVAL;
    }

    //if src addr or dst addr no aligned by CACHE_ALIGNED_SIZE
    //then copy by cpu.
    if ((source & (CACHE_ALIGNED_SIZE - 1)) || (dest & (CACHE_ALIGNED_SIZE - 1))||\
            (length & (CACHE_ALIGNED_SIZE - 1))) {
        memcpy((unsigned int *)dest, (unsigned int *)source, length);
        dma_err("dma addr is not aligned by CACHE_ALIGNED_SIZE,coping by cpu !\n");
        return 0;
    }
    //alloc dma channel
    chnn = dmac_channel_allocate(NULL);
    if (DMAC_CHANNEL_INVALID == chnn) {
        dma_err("no channel available, coping by cpu !\n");
        memcpy((unsigned int *)dest, (unsigned int *)source, length);
        return 0;
    }
    dma_debug("chnn:%d,src:0x%X,dst:0x%X,len:%d\n",chnn,source, dest, length);

    lli_length = allocate_dmalli_space(&pllihead, length);
    if (lli_length < 0) return -1;

    dma_cache_clean(source, source + length);
    dma_cache_inv(dest, dest + length);

    /*save head*/
    pheadlliList[chnn] = pllihead;

    dma_debug("pllihead: 0x%X, length:%d\n",(unsigned int)pllihead, lli_length);
    ret = dmac_buildllim2m(pllihead, source, dest, length, DMAC_TRANS_SIZE);
    dma_cache_clean((unsigned int)pllihead, ((unsigned int)pllihead) + lli_length);

    if (ret) {
        dma_err("build lli error...\n");
        return -1;
    }
    dmac_register_isr(chnn, dmac_channel_close);

    ret = dmac_start_llim2m(chnn, pllihead);
    if (ret) return -1;

    if (dmac_channelstart(chnn) != 0) {
        dma_err("start channel error...\n");
        return -1;
    }
    ret_result = LOS_EventRead(&dmac_event[chnn], DMAC_EVENTT_TYPE,
            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, LOS_MS2Tick(5000));
    if (DMAC_EVENTT_TYPE != ret_result) {
        dma_err("dmac m2m transfer timeout...\n");
        ret = -1;
    }
    return ret;
}

static int dmac_buildllim2m_isp(dmac_lli **ppheadlli,unsigned int *psource,
        unsigned int *pdest, unsigned int *length,unsigned int lli_num)
{
    unsigned int i;
    dmac_lli * pheadlli = (dmac_lli *)g_phandlli;
    if (!pheadlli) {
        dma_err("g_phandlli is NULL!\n");
        return -1;
    }
    *ppheadlli =(dmac_lli *)g_phandlli;
    memset(g_phandlli, 0, (lli_num * sizeof(dmac_lli)));
    for(i = 0;i < lli_num; i++) {
        dmac_writew(&(pheadlli->src_addr), psource[i]);
        dmac_writew(&(pheadlli->dst_addr), pdest[i]);
        if (i == (lli_num - 1))
            dmac_writew(&(pheadlli->next_lli), 0);
        else
            dmac_writew(&(pheadlli->next_lli),
                    (((unsigned int)(pheadlli+1) & (~0x03))
                     | DMAC_CxLLI_LM));
        //dma_debug("pheadlli->next_lli:0x%X\n", (pheadlli->next_lli));
        if (i == (lli_num - 1)) {
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    ((DMAC_CxCONTROL_LLIM2M_ISP & (~0xfff))
                     | (length[i])
                     | DMAC_CxCONTROL_INT_EN));
        } else {
            dmac_writew(&(pheadlli->lli_transfer_ctrl),
                    (((DMAC_CxCONTROL_LLIM2M_ISP & (~0xfff)) | (length[i]))
                     & 0x7fffffff));
        }
        pheadlli++;
    }
    return 0;
}
int do_dma_llim2m_isp(unsigned int *source,
        unsigned int  *dest,
        unsigned int *length,
        unsigned int num)
{
    int ret = 0;
    unsigned int ret_result = 0;
    unsigned int chnn;
    dmac_lli *pllihead = NULL;
    if (mem_check_valid((unsigned int)source) | mem_check_valid((unsigned int)dest)) {
        dma_err("Invalid dma addr psrc = 0x%x, pdest = 0x%x!\n",
                source, dest);
        return -EINVAL;
    }
    if (num >= (DMAC_MAX_LLI_SIZE/sizeof(dmac_lli))) {
        dma_err("num is too large, num = %d\n",num);
        return -EINVAL;
    }
    chnn = 2;

    ret = dmac_buildllim2m_isp(&pllihead, source, dest, length, num);
    if (ret < 0)
        return -EINVAL;

    dma_cache_clean((unsigned int)pllihead, ((unsigned int)pllihead) + (num * sizeof(dmac_lli)));
    ret = dmac_start_llim2m(chnn, pllihead);
    if (ret)
        return -1;

    if (dmac_channelstart(chnn) != 0) {
        dma_err("start channel error...\n");
        return -1;
    }

    return ret;
}
/*
 * execute memory to memory dma transfer without LLI
 */
int dmac_m2m_transfer(unsigned int source,
        unsigned int dest,
        unsigned int length)
{
    unsigned int ulchnn, dma_size = 0;
    unsigned int dma_count, left_size;

    int ret;

    left_size = length;
    dma_count = 0;

    if (mem_check_valid(source) | mem_check_valid(dest)) {
        dma_err("Invalid dma addr psrc = 0x%x, pdest = 0x%x!\n",
                source, dest);
        return -EINVAL;
    }
    //if src addr or dst addr no aligned by 64bit
    //then copy by cpu.
    if ((source & (CACHE_ALIGNED_SIZE - 1)) || (dest & (CACHE_ALIGNED_SIZE - 1))||\
            (length & (CACHE_ALIGNED_SIZE - 1))) {
        memcpy((unsigned int *)dest, (unsigned int *)source, length);
        dma_err("dma addr is not aligned by CACHE_ALIGNED_SIZE,coping by cpu !\n");
        return 0;
    }

    ulchnn = dmac_channel_allocate(NULL);
    if (DMAC_CHANNEL_INVALID == ulchnn)
        return -1;
    dma_cache_clean(source, source + length);
    dma_cache_inv(dest, dest + length);

    dma_size   = DMAC_TRANS_SIZE;
    while ((left_size >> 2) >= dma_size) {
        left_size -= (dma_size << 2);

        //dma_debug("dma_count: %d, left_size: 0x%X\n",dma_count, left_size);
        //transfer: dma_size*4 Bytes
        ret = dmac_start_m2m(ulchnn,
                (unsigned int)(source + dma_count * (dma_size << 2)),
                (unsigned int)(dest + dma_count * (dma_size << 2)),
                dma_size);        //(dma_size << 2));
        if (ret)
        {
            dma_err("set dma mem  error...\n");
        }
        if (dmac_channelstart(ulchnn) != 0) {
            dma_err("start channel error...\n");
            return -1;
        }

        if (dmac_wait(ulchnn) != DMAC_CHN_SUCCESS) {
            dma_err("dma transfer error...\n");
            return -1;
        }

        dma_count++;
    }

    dmac_start_m2m(ulchnn,
            (source + dma_count * (dma_size << 2)),
            (dest + dma_count * (dma_size << 2)),
            (left_size >> 2) );             //(left_size << 2));

    if (dmac_channelstart(ulchnn) != 0)
        return -1;

    if (dmac_wait(ulchnn) != DMAC_CHN_SUCCESS)
        return -1;

    dmac_channelclose(ulchnn);
    return 0;
}


/*
 * execute memory to peripheral dma transfer without LLI
 */
int dmac_m2p_transfer(unsigned int memaddr,
        unsigned int uwperipheralid,
        unsigned int length)
{
    unsigned int ulchnn, dma_size = 0;
    unsigned int dma_count, left_size;
    unsigned int uwwidth;

    left_size = length;
    dma_count = 0;

    ulchnn = dmac_channel_allocate(NULL);
    if (DMAC_CHANNEL_INVALID == ulchnn)
        return -1;

    uwwidth = g_peripheral[uwperipheralid].transfer_width;

    //if memaddr no aligned by CACHE_ALIGNED_SIZE
    if ((memaddr & (CACHE_ALIGNED_SIZE - 1)) ) {
        dma_err("dma addr is not aligned by CACHE_ALIGNED_SIZE!\n");
        dmac_channel_free(ulchnn);
        return -1;
    }
    dma_cache_clean(memaddr, memaddr+(((int)length+ CACHE_ALIGNED_SIZE) & (~(CACHE_ALIGNED_SIZE - 1))));

    while ((left_size >> uwwidth) >= 0xffc) {
        dma_size = 0xffc;
        left_size -= (dma_size << uwwidth);

        if (dmac_start_m2p(ulchnn,
                    (unsigned int)(memaddr + dma_count * dma_size),
                    uwperipheralid, (dma_size << uwwidth), 0) < 0)
        {
            dmac_channel_free(ulchnn);
            return -1;
        }
        if (dmac_channelstart(ulchnn) != 0){
            dma_err("[%d]: dma start channel failed\n", dma_count);
            dmac_channel_free(ulchnn);
            return -1;
        }
        if (dmac_wait(ulchnn) != DMAC_CHN_SUCCESS)
            if (dmac_channelstart(ulchnn) != 0)
            {
                dmac_channel_free(ulchnn);
                return -1;
            }
        if (dmac_wait(ulchnn) != DMAC_CHN_SUCCESS){
            dma_err("[%d]: dma wait failed\n", dma_count);
            dmac_channel_free(ulchnn);
            return -1;
        }

        dma_count++;
    }

    dma_debug("memaddr=0x%x\n", (memaddr + dma_count * dma_size));
    if(left_size > 0)
    {
        if (dmac_start_m2p(ulchnn,
                    (memaddr + dma_count * dma_size),
                    uwperipheralid, left_size, 0) < 0)
        {
            dmac_channel_free(ulchnn);
            return -1;
        }
        if (dmac_channelstart(ulchnn) != 0){
            dma_err("[%d]: dma start channel failed\n", dma_count);
            dmac_channel_free(ulchnn);
            return -1;
        }
    }
    return ulchnn;
}

/*
 * execute peripheral to memory dma transfer with lLI
 */
int dmac_llip2m_transfer(unsigned int memaddr,
        unsigned int peripheralid,
        unsigned int length,
        unsigned int *channel)
{
    int ret = 0;
    int lli_length = 0;
    unsigned int chnn = 0;
    unsigned int width = 0;
    dmac_lli *pheadlli = NULL;
    unsigned int lli_num = 0;

    if ((memaddr & (CACHE_ALIGNED_SIZE - 1)) ) {
        dma_err("dma addr not aligned by CACHE_ALIGNED_SIZE!\n");
        return -1;
    }
    if ((peripheralid >= DMAC_MAX_PERIPHERALS) || (!channel)) {
        dma_err("peripheralid out or channel is null!\n");
        return -2;
    }
    chnn = dmac_channel_allocate(NULL);
    if (DMAC_CHANNEL_INVALID == chnn) return -1;

    width = g_peripheral[peripheralid].transfer_width;

    /* if can transfer the data once, needn't build lli (just one lli) */
    if ((length >> width) <= 0xffc) {
        if (dmac_start_p2m(chnn, memaddr,
                    peripheralid, length, 0) < 0)
        {
            dmac_channel_free(chnn);
            return -1;
        }
    } else {
        lli_length = allocate_dmalli_space(&pheadlli, length);
        if (lli_length < 0) return -1;

        /*save head*/
        pheadlliList[chnn] = pheadlli;

        dmac_buildllip2m(pheadlli, peripheralid, memaddr, length, DMAC_TRANS_SIZE);

        dma_cache_clean((unsigned int)pheadlli, ((unsigned int)pheadlli) + lli_length);
        ret = dmac_start_llip2m(chnn, pheadlli, peripheralid);
        if (ret) return -1;
    }
    dmac_register_isr(chnn, dmac_channel_close);

    (void)dmac_channelstart(chnn);

    *channel = chnn;

    return ret;
}

/*
 * execute memory to peripheral dma transfer without LLI
 */
int dmac_p2m_transfer(unsigned int memaddr,
        unsigned int uwperipheralid,
        unsigned int length)
{
    unsigned int ulchnn, dma_size = 0;
    unsigned int dma_count, left_size;
    unsigned int uwwidth;

    left_size = length;
    dma_count = 0;

    ulchnn = dmac_channel_allocate(NULL);
    if (DMAC_CHANNEL_INVALID == ulchnn)
        return -1;

    uwwidth = g_peripheral[uwperipheralid].transfer_width;

    //if memaddr no aligned by CACHE_ALIGNED_SIZE
    if ((memaddr & (CACHE_ALIGNED_SIZE - 1)) ) {
        dma_err("dma addr is not aligned by CACHE_ALIGNED_SIZE!\n");
        dmac_channel_free(ulchnn);
        return -1;
    }
    dma_cache_inv(memaddr, memaddr + (((int)length + CACHE_ALIGNED_SIZE) & (~(CACHE_ALIGNED_SIZE - 1))));

    while ((left_size >> uwwidth) >= 0xffc) {
        dma_size = 0xffc;
        left_size -= (dma_size << uwwidth);

        if (dmac_start_p2m(ulchnn,
                    (unsigned int)(memaddr + dma_count * dma_size),
                    uwperipheralid, (dma_size << uwwidth), 0) < 0)
        {
            dmac_channel_free(ulchnn);
            return -1;
        }
        if (dmac_channelstart(ulchnn) != 0){
            dmac_channel_free(ulchnn);
            return -1;
        }
        if (dmac_wait(ulchnn) != DMAC_CHN_SUCCESS){
            dmac_channel_free(ulchnn);
            return -1;
        }

        dma_count++;
    }

    if (left_size > 0)
    {
        if (dmac_start_p2m(ulchnn,
                    (unsigned int)(memaddr + dma_count * dma_size),
                    uwperipheralid, left_size, 0) < 0)
        {
            dmac_channel_free(ulchnn);
            return -1;
        }

        if (dmac_channelstart(ulchnn) != 0){
            dmac_channel_free(ulchnn);
            return -1;
        }
    }
    return ulchnn;
}

int do_dma_m2p(unsigned int memaddr,
        unsigned int peri_addr,
        unsigned int length)
{
    int ret = 0;
    int uwperipheralid;

    uwperipheralid = dmac_check_request(peri_addr, TX);
    if (uwperipheralid < 0) {
        dma_err("m2p:Invalid devaddr\n");
        return -1;
    }

    ret = dmac_m2p_transfer(memaddr, uwperipheralid, length);
    if (ret == -1) {
        dma_err("m2p:trans err\n");
        return -1;
    }

    return ret;
}

int do_dma_p2m(unsigned int memaddr, unsigned int peri_addr, unsigned int length)
{
    int ret = -1;
    int uwperipheralid;

    uwperipheralid = dmac_check_request(peri_addr, RX);
    if (uwperipheralid < 0) {
        dma_err("p2m:Invalid devaddr.\n");
        return -1;
    }

    ret = dmac_p2m_transfer(memaddr, uwperipheralid, length);
    if (ret == -1) {
        dma_err("p2m:trans err\n");
        return -1;
    }

    return ret;
}

int do_dma_llip2m(unsigned int memaddr, unsigned int peri_addr, unsigned int length)
{
    int ret = -1;
    int uwperipheralid;
    int channel;

    uwperipheralid = dmac_check_request(peri_addr, RX);
    if (uwperipheralid < 0) {
        dma_err("p2m:Invalid devaddr.\n");
        return -1;
    }

    ret = dmac_llip2m_transfer(memaddr, uwperipheralid, length, &channel);
    if (ret == -1) {
        dma_err("p2m:trans err\n");
        return -1;
    }

    return channel;
}

/*
 *    init dmac register
 *    clear interupt flags
 *    called by dma_driver_init
 */
int dmac_init(void)
{
    unsigned int i, tempvalue;

    reg_dmac_base_va = (void *)DMAC_REG_BASE;

    g_phandlli = (unsigned int*)memalign(CACHE_ALIGNED_SIZE, DMAC_MAX_LLI_SIZE);
    if (!g_phandlli)
    {
        dma_err("can't malloc llimem for dma!\n ");
        return -1;
    }
    hidmac_clk_en();

    hidmac_unreset();

    dmac_readw(DMAC_CONFIG, tempvalue);
    if (tempvalue == 0) {
        dmac_writew(DMAC_CONFIG, DMAC_CONFIG_VAL);
        dmac_writew(DMAC_INTTCCLEAR, 0xFF);
        dmac_writew(DMAC_INTERRCLR, 0xFF);
        for (i = 0; i < DMAC_MAX_CHANNELS; i++){
            dmac_writew(DMAC_CxCONFIG(i), DMAC_CxDISABLE);
            g_channel_status[i] = DMAC_CHN_VACANCY;
            pheadlliList[i] = NULL;
            function[i] = NULL;
            LOS_EventInit(&dmac_event[i]);
        }
    }

#if 1
    if (request_irq(NUM_HAL_INTERRUPT_DMAC, &dmac_isr, (unsigned long)NULL, "hi_dma", NULL)) {
        dma_err("DMA Irq %d request failed\n", NUM_HAL_INTERRUPT_DMAC);
        return -1;
    }
#endif
    return 0;
}

int hi_dmac_remove(void)
{
    int i;

    for (i = 0; i < DMAC_MAX_CHANNELS; i++)
        g_channel_status[i] = DMAC_CHN_VACANCY;

    return 0;
}

void dmac_suspend(unsigned int channel)
{
    /*disable dmac*/
    if (DMAC_NOT_FINISHED == g_channel_status[channel]) {

        plliList_bak[channel] = (dmac_lli_bak *)zalloc(sizeof(dmac_lli_bak));
        if (NULL == plliList_bak[channel]) {
            dma_err("DMA suspend request failed\n");
            return;
        }
        dmac_readw(DMAC_CxSRCADDR(channel), plliList_bak[channel]->src_addr);
        dmac_readw(DMAC_CxDESTADDR(channel), plliList_bak[channel]->dst_addr);
        dmac_readw(DMAC_CxLLI(channel), plliList_bak[channel]->next_lli);
        dmac_readw(DMAC_CxCONTROL(channel), plliList_bak[channel]->lli_transfer_ctrl);
        dmac_readw(DMAC_CxCONFIG(channel), plliList_bak[channel]->lli_transfer_config);

        dmac_writew(DMAC_CxCONFIG(channel), DMAC_CxDISABLE);
    }
}

void dmac_resume(unsigned int channel)
{
    hidmac_clk_en();
    hidmac_unreset();

    dmac_writew(DMAC_CONFIG, DMAC_CONFIG_VAL);
    /*dmac_writew(DMAC_SYNC, sync_register_bak);*/
    if (DMAC_NOT_FINISHED == g_channel_status[channel]) {
        if (NULL != plliList_bak[channel]) {
            dmac_writew(DMAC_CxSRCADDR(channel), plliList_bak[channel]->src_addr);
            dmac_writew(DMAC_CxDESTADDR(channel), plliList_bak[channel]->dst_addr);
            dmac_writew(DMAC_CxLLI(channel), plliList_bak[channel]->next_lli);
            dmac_writew(DMAC_CxCONTROL(channel), plliList_bak[channel]->lli_transfer_ctrl);
            dmac_writew(DMAC_CxCONFIG(channel), plliList_bak[channel]->lli_transfer_config);

            free(plliList_bak[channel]);
            plliList_bak[channel] = NULL;
        }
    }
}

int full_duplex_dma_ch_wait(unsigned int rx_ch, unsigned int tx_ch)
{

    unsigned int ret_result = 0;
    int ret = 0;

    ret_result = LOS_EventRead(&dmac_event[rx_ch], DMAC_EVENTT_TYPE,
            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, LOS_MS2Tick(5000));
    if (DMAC_EVENTT_TYPE != ret_result) {
        dma_err("rx ch timeout...\n");
        ret = -1;
    }

    ret_result = LOS_EventRead(&dmac_event[tx_ch], DMAC_EVENTT_TYPE,
            LOS_WAITMODE_OR+LOS_WAITMODE_CLR, LOS_MS2Tick(5000));
    if (DMAC_EVENTT_TYPE != ret_result) {
        dma_err("tx ch timeout...\n");
        ret = -1;
    }

    return 0;

#if 0
    if ((dmac_wait(rx_ch) != DMAC_CHN_SUCCESS)) {
        dma_err("wait rx error!\n");
        return -1;
    }

    if ((dmac_wait(tx_ch) != DMAC_CHN_SUCCESS)) {
        dma_err("wait rx error!\n");
        return -1;
    }
#endif
}

int full_duplex_dma_ch_free(unsigned int rx_ch, unsigned int tx_ch)
{
    dmac_channel_free(rx_ch);
    dmac_channel_free(tx_ch);
    return 0;
}
int dmac_transfer_without_start_ch(unsigned int memaddr,
        unsigned int peripheralid,
        unsigned int rxtxflags,
        unsigned int length)
{
    unsigned int ulchnn;
    unsigned int uwwidth;

    ulchnn = dmac_channel_allocate(NULL);
    if (DMAC_CHANNEL_INVALID == ulchnn)
        return -1;

    uwwidth = g_peripheral[peripheralid].transfer_width;

    if ((memaddr & (CACHE_ALIGNED_SIZE - 1)) ) {
        dma_err("dma addr is not aligned by CACHE_ALIGNED_SIZE!\n");
        dmac_channel_free(ulchnn);
        return -1;
    }

    if ((length >> uwwidth) > 0xfff)
        dma_err("Invalidate transfer size,size=%x! \n", length);

    if (rxtxflags == 0) {
        dma_cache_inv(memaddr, memaddr + (((int)length + CACHE_ALIGNED_SIZE) & (~(CACHE_ALIGNED_SIZE - 1))));
        if (dmac_start_p2m(ulchnn, (unsigned int)(memaddr),
                    peripheralid, length, 0) < 0) {
            dmac_channel_free(ulchnn);
            return -1;
        }
    } else {
        dma_cache_clean(memaddr, memaddr + (((int)length+ CACHE_ALIGNED_SIZE) & (~(CACHE_ALIGNED_SIZE - 1))));
        if (dmac_start_m2p(ulchnn, (unsigned int)(memaddr),
                    peripheralid, length, 0) < 0) {
            dmac_channel_free(ulchnn);
            return -1;
        }
    }
    dmac_register_isr(ulchnn, dmac_channel_close);
    return ulchnn;
}

/*end of file hi_dmac.c*/
