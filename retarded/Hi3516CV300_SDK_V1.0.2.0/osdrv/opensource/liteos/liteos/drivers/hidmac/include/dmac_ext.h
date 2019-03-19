#ifndef __DMAC_EXT_H__
#define __DMAC_EXT_H__
#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*sturcture for LLI*/
typedef struct dmac_lli {
    uint32_t src_addr;            /* source address */
    uint32_t dst_addr;            /* destination address */
    uint32_t next_lli;            /* pointer to next LLI */
    uint32_t lli_transfer_ctrl;    /* control word */
} dmac_lli;

/*sturcture for LLI, for dmac suspend*/
typedef struct dmac_lli_bak{
    uint32_t src_addr;            /* source address */
    uint32_t dst_addr;            /* destination address */
    uint32_t next_lli;            /* pointer to next LLI */
    uint32_t lli_transfer_ctrl;    /* control word */
    uint32_t lli_transfer_config;  /* config word */
} dmac_lli_bak;

typedef struct hi_dma_llihead {
    uint32_t phys;
    uint32_t virt;
} hi_dma_llihead;

extern int dmac_init(void);
extern int hi_dmac_remove(void);

extern int dmac_m2p_transfer(uint32_t pmem, uint32_t peri_id, uint32_t size);
extern int dmac_llip2m_transfer(uint32_t pmem, uint32_t peri_id, uint32_t size, uint32_t *channel);
extern int dmac_m2m_transfer(uint32_t psrc, uint32_t pdest, uint32_t size);
extern int dmac_llim2m_transfer(uint32_t psrc, uint32_t pdest, uint32_t size);
extern int dmac_channelstart(uint32_t channel);
extern int dmac_start_llim2m(uint32_t channel, dmac_lli *pfirst_lli);
extern uint32_t dmac_get_current_dest(uint32_t channel);
extern int dmac_wait(uint32_t channel);
extern int dmac_wait_for_irq(uint32_t channel);
extern uint32_t dmac_channel_allocate(void *pisr);
extern int allocate_dmalli_space(dmac_lli **ppheadlli, uint32_t length);
extern int dmac_channelclose(uint32_t channel);
extern int dmac_register_isr(uint32_t channel,void *pisr);
extern int dmac_channel_free(uint32_t channel);
extern void free_dmalli_space(dmac_lli *ppheadlli);

extern int dmac_start_llim2p(uint32_t channel,
                        uint32_t *pfirst_lli,
                        uint32_t peri_id);

extern int dmac_buildllim2m(dmac_lli * ppheadlli,
        uint32_t psrc,
        uint32_t pdest,
        uint32_t total_size,
        uint32_t uwnumtransfers);

extern int dmac_buildllip2m(dmac_lli *ppheadlli,
        uint32_t uwperipheralid,
        uint32_t memaddr,
        uint32_t totaltransfersize,
        uint32_t uwnumtransfers);

extern int dmac_buildllim2p(hi_dma_llihead *ppheadlli,
                    uint32_t *pmemaddr,
                    uint32_t uwperipheralid,
                    uint32_t totaltransfersize,
                    uint32_t uwnumtransfers ,uint32_t burstsize);

extern int dmac_start_m2p(uint32_t channel,
                    uint32_t pmem,
                    uint32_t peri_id,
                    uint32_t  uwnumtransfers,
                    uint32_t next_lli_addr);

extern int dmac_start_m2m(uint32_t channel,
                    uint32_t psrc,
                    uint32_t pdest,
                    uint32_t uwnumtransfers);

extern int do_dma_llim2m_isp(unsigned int *source,
        unsigned int  *dest,
        unsigned int *length,
        unsigned int num);

extern int do_dma_m2p(unsigned int memaddr,
        unsigned int peri_addr,
        unsigned int length);
extern int do_dma_p2m(unsigned int memaddr,
        unsigned int peri_addr,
        unsigned int length);
extern int do_dma_llip2m(unsigned int memaddr,
        unsigned int peri_addr,
        unsigned int length);

extern int dmac_start_p2m(unsigned int channel,
        unsigned int memaddr,
        unsigned int uwperipheralid,
        unsigned int uwnumtransfers,
        unsigned int next_lli_addr);

extern int dmac_start_llip2m(unsigned int channel, dmac_lli *pfirst_lli, unsigned int peri_id);

extern void dmac_suspend(uint32_t channel);

extern void dmac_resume(uint32_t channel);

extern int full_duplex_dma_ch_free(unsigned int rx_ch, unsigned int tx_ch);

extern int dmac_transfer_without_start_ch(unsigned int memaddr,
        unsigned int peripheralid,
        unsigned int rxtxflags,
        unsigned int length);

extern int dmac_check_request(unsigned int peripheral_addr,
                int direction);
extern int full_duplex_dma_ch_wait(unsigned int rx_ch, unsigned int tx_ch);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

