#ifndef _LINUX_SCATTERLIST_H
#define _LINUX_SCATTERLIST_H

#include <linux/string.h>
#include <linux/kernel.h>
#include "asm/bug.h"
#include "los_printf.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef unsigned long dma_addr_t;

struct scatterlist {
#ifdef CONFIG_DEBUG_SG
    unsigned long   sg_magic;
#endif
    unsigned long   page_link;
    unsigned int    offset;
    unsigned int    length;
    dma_addr_t      dma_address;
#ifdef CONFIG_NEED_SG_DMA_LENGTH
    unsigned int    dma_length;
#endif
};


#define SG_MAGIC    0x87654321


/**
 * sg_mark_end - Mark the end of the scatterlist
 * @sg:         SG entryScatterlist
 *
 * Description:
 *   Marks the passed in sg entry as the termination point for the sg
 *   table. A call to sg_next() on this entry will return NULL.
 *
 **/
static inline void sg_mark_end(struct scatterlist *sg)
{
#ifdef CONFIG_DEBUG_SG
    BUG_ON(sg->sg_magic != SG_MAGIC);
#endif
    /*
     * Set termination bit, clear potential chain bit
     */
    sg->page_link |= 0x02;
    sg->page_link &= ~0x01;
}

static inline void sg_init_table(struct scatterlist *sgl, unsigned int nents)
{
    memset(sgl, 0, sizeof(*sgl) * nents);

    sg_mark_end(&sgl[nents - 1]);
}

static inline void sg_set_buf(struct scatterlist *sg, const void *buf,
        unsigned int buflen)
{
    sg->dma_address =(unsigned long) buf;
    sg->offset = 0;
    sg->length = buflen;
}

static inline void sg_init_one(struct scatterlist *sg, const void *buf, unsigned int buflen)
{
    sg_init_table(sg, 1);
    sg_set_buf(sg, buf, buflen);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _LINUX_SCATTERLIST_H */
