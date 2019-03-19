/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "mmu_config.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif /* LOSCFG_LIB_LIBC */
#include "los_printf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define UNCACHEABLE                         0
#define CACHEABLE                           1
#define UNBUFFERABLE                        0
#define BUFFERABLE                          1
#define ACCESS_RW                   3
#define ACCESS_RO                   0

#define MMU_AP_STATE(flag)  (((flag) & 0x1)?ACCESS_RW:ACCESS_RO)
#define MMU_CACHE_STATE(flag)  (((flag) >> 1) & 0x1)
#define MMU_BUFFER_STATE(flag) (((flag) >> 2) & 0x1)
#define MMU_GET_AREA(flag) ((flag)&(0x1<<3))

extern void dma_cache_clean(int swPageStart, int swPageEnd);
extern void *memset(void *dst, int c, size_t n);

__attribute__((aligned(MMU_16K))) __attribute__((section(".bss.prebss.translation_table"))) UINT32 first_page_table[MMU_4K];
__attribute__((aligned(MMU_1K))) UINT32 second_page_table_os[MMU_4K];
__attribute__((aligned(MMU_1K))) UINT32 second_page_table_mmz[DDR_MEM_SIZE/MMU_1K];

SENCOND_PAGE stOsPage = {0};
SENCOND_PAGE stAppPage = {0};

static void set_mem_page(MMU_PARAM *mPara)
{
    UINT32 page_start_index;
    UINT32 page_end_index;
    UINT32 length, uwCache, uwBuf, uwAP;
    UINT32 page_size = (mPara->stPage->page_type == MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID) ? MMU_64K : MMU_4K;
    UINT32 stt_base = mPara->stPage->page_descriptor_addr;

    if ((mPara->startAddr % page_size) != 0)
    {
        return;
    }

    if ((mPara->endAddr % page_size) != 0)
    {
        mPara->endAddr = (mPara->endAddr + (page_size -1)) & ~ (page_size - 1);
    }

    uwAP = MMU_AP_STATE(mPara->uwFlag);
    uwCache = MMU_CACHE_STATE(mPara->uwFlag);
    uwBuf = MMU_BUFFER_STATE(mPara->uwFlag);

    page_start_index = (mPara->startAddr - mPara->stPage->page_addr)/page_size;
    page_end_index = (mPara->endAddr - mPara->stPage->page_addr)/page_size;
    length = page_end_index - page_start_index;

    if (uwAP == ACCESS_RW)
    {
        if (page_size == MMU_64K)
        {
            X_MMU_TWO_LEVEL_PAGE64K_RW((mPara->startAddr/page_size), page_start_index, length, uwCache, uwBuf, ACCESS_RW); /*lint !e440 !e442 !e443*/
        }
        else
        {
            X_MMU_TWO_LEVEL_PAGE_RW((mPara->startAddr/page_size), page_start_index, length, uwCache, uwBuf, ACCESS_RW);
        }
    }
    else
    {
        if (page_size == MMU_64K)
        {
            X_MMU_TWO_LEVEL_PAGE64K_RO((mPara->startAddr/page_size), page_start_index, length, uwCache, uwBuf, ACCESS_RW); /*lint !e440 !e442 !e443*/
        }
        else
        {
            X_MMU_TWO_LEVEL_PAGE_RO((mPara->startAddr/page_size), page_start_index, length, uwCache, uwBuf, ACCESS_RW);
        }
    }
}

VOID LOS_SecPageEnable(SENCOND_PAGE *stPage, UINT32 uwFlag)
{
    UINT32 uwPageStart;
    UINT32 uwPageEnd;
    UINT32 ttb_base = FIRST_PAGE_DESCRIPTOR_ADDR;
    MMU_PARAM mPara;

    if (stPage == NULL)
    {
        PRINT_ERR("second page table(stPage) can't be NULL\n");
        return;
    }

    mPara.startAddr = stPage->page_addr;
    mPara.endAddr = stPage->page_addr + stPage->page_length;
    mPara.uwFlag = uwFlag;
    mPara.stPage = stPage;

    set_mem_page(&mPara); /*lint !e522 */
    uwPageStart = stPage->page_descriptor_addr;
    uwPageEnd = stPage->page_descriptor_addr + (stPage->page_length / MMU_4K * 4); /*lint !e504*//* page size = 2^12, 4K*/
    dma_cache_clean((int)uwPageStart, (int)uwPageEnd);

    X_MMU_ONE_LEVEL_PAGE((stPage->page_descriptor_addr/MMU_1K), (stPage->page_addr/MMU_1M), (stPage->page_length/MMU_1M)); /*lint !e681 */
    dma_cache_clean((int)(ttb_base + mPara.startAddr/MMU_1M * 4), (int)(ttb_base + mPara.endAddr/MMU_1M * 4));

    __asm volatile ( "mov    %0, #0\n" "mcr    p15, 0, %0, c8, c7, 0\n" : : "r" (0));
}

void LOS_MMUParamSet(MMU_PARAM *mPara)
{
    UINT32 uwPageStart;
    UINT32 uwPageEnd;
    UINT32 uwTtb = 0;
    UINT32 uwItemStart;
    UINT32 uwItemEnd;
    UINT32 uwItemTemp;
    UINT32 uwCache, uwBuf, uwAP,uwTableType;

    if (mPara == NULL)
    {
        PRINT_ERR("input is null\n");
        return;
    }

    uwItemStart = MMU_GET_FIRST_TABLE_ADDR(mPara->startAddr);
    uwItemEnd = MMU_GET_FIRST_TABLE_ADDR(mPara->endAddr - 1);
    uwItemTemp = uwItemStart;

    if (uwItemStart > uwItemEnd)
    {
        PRINT_ERR("wrong addr input, uwItemStart:0x%x, uwItemEnd:0x%x\n", uwItemStart, uwItemEnd);
        return;
    }

    uwAP = MMU_AP_STATE(mPara->uwFlag);
    uwCache = MMU_CACHE_STATE(mPara->uwFlag);
    uwBuf = MMU_BUFFER_STATE(mPara->uwFlag);
    uwTableType = MMU_GET_AREA(mPara->uwFlag);

    if (uwTableType == SECOND_PAGE)
    {
        if (mPara->startAddr < mPara->stPage->page_addr || mPara->endAddr > (mPara->stPage->page_length + mPara->stPage->page_addr))
        {
            PRINT_ERR("addr input not belongs to this second page\n");
            PRINT_ERR("mPara->startAddr:0x%x, mPara->stPage->page_addr:0x%x\n", mPara->startAddr,mPara->stPage->page_addr);
            PRINT_ERR("mPara->endAddr:0x%x, (mPara->stPage->page_length + mPara->stPage->page_addr):0x%x\n", mPara->endAddr, (mPara->stPage->page_length + mPara->stPage->page_addr));
            return;
        }
        while (uwItemTemp <= uwItemEnd)
        {
            if ((*(UINT32 *)uwItemTemp & 0x3) != MMU_FIRST_LEVEL_PAGE_TABLE_ID)
            {
                PRINT_ERR("not all mem belongs to second page(4K or 64K every item), mmu table ID:%d \n", (*(UINT32 *)uwItemTemp & 0x3));
                return;
            }
            uwItemTemp += 4;
        }

        set_mem_page(mPara); /*lint !e522*/
        uwPageStart = MMU_GET_SECOND_TABLE_ADDR(mPara->startAddr);
        uwPageEnd = MMU_GET_SECOND_TABLE_ADDR(mPara->endAddr - 1);
        dma_cache_clean((int)uwPageStart, (int)uwPageEnd);
    }
    else  if (uwTableType == FIRST_SECTION)
    {
        while (uwItemTemp <= uwItemEnd)
        {
            if ((*(UINT32 *)uwItemTemp & 0x3) != MMU_FIRST_LEVEL_SECTION_ID)
            {
                PRINT_ERR("not all mem belongs to first section(1M every item), mmu table ID:%d\n", (*(UINT32 *)uwItemTemp & 0x3));
                return;
            }
            uwItemTemp += 4;
        }

        uwItemTemp = uwItemStart;
        while (uwItemTemp <= uwItemEnd)
        {
            union MMU_FIRST_LEVEL_DESCRIPTOR desc;
            desc.word = (*(UINT32 *)uwItemTemp);
            desc.section.c = (uwCache);
            desc.section.b = (uwBuf);
            desc.section.ap = (uwAP);
            (*(UINT32 *)uwItemTemp)= desc.word;
            uwItemTemp += 4;
        }
        dma_cache_clean((int)uwItemStart, (int)uwItemEnd);
    }

    __asm volatile ( "mov    %0, #0\n" "mcr    p15, 0, %0, c8, c7, 0\n" : : "r" (uwTtb));
}/*lint !e529 !e438*/

void hal_mmu_init(void) {
    UINT32 ttb_base = FIRST_PAGE_DESCRIPTOR_ADDR + 0x0;
    UINT32 uwReg = 0;

    // Set the TTB register
    __asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

    // Set the Domain Access Control Register
    uwReg = ACCESS_TYPE_MANAGER(0)    |
        ACCESS_TYPE_CLIENT(1)  |
        ACCESS_TYPE_NO_ACCESS(2)  |
        ACCESS_TYPE_NO_ACCESS(3)  |
        ACCESS_TYPE_NO_ACCESS(4)  |
        ACCESS_TYPE_NO_ACCESS(5)  |
        ACCESS_TYPE_NO_ACCESS(6)  |
        ACCESS_TYPE_NO_ACCESS(7)  |
        ACCESS_TYPE_NO_ACCESS(8)  |
        ACCESS_TYPE_NO_ACCESS(9)  |
        ACCESS_TYPE_NO_ACCESS(10) |
        ACCESS_TYPE_NO_ACCESS(11) |
        ACCESS_TYPE_NO_ACCESS(12) |
        ACCESS_TYPE_NO_ACCESS(13) |
        ACCESS_TYPE_NO_ACCESS(14) |
        ACCESS_TYPE_NO_ACCESS(15);
    __asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(uwReg));

#ifdef LOSCFG_KERNEL_RUNSTOP
    extern INT32 g_swResumeFromImg;
    if (g_swResumeFromImg == 1) return;
#endif

    // First clear all TT entries - ie Set them to Faulting
    memset((void *)ttb_base, 0, MMU_16K);
    // set all mem 4G as uncacheable & rw first
    X_MMU_SECTION(0, 0, (MMU_4G/MMU_1M + 1), UNCACHEABLE, UNBUFFERABLE, ACCESS_RW);
    X_MMU_SECTION(((SYS_MEM_BASE + TEXT_OFFSET)/MMU_1M), 0, (1), UNCACHEABLE, UNBUFFERABLE, ACCESS_RW); /*lint !e74 */

    /**************************************************************************************************
    *    set table as your config
    *    1: LITEOS_CACHE_ADDR ~ LITEOS_CACHE_ADDR + LITEOS_CACHE_LENGTH ---- set as section(1M) and cacheable & rw
    ****************************************************************************************************/
    X_MMU_SECTION((LITEOS_CACHE_ADDR/MMU_1M), (LITEOS_CACHE_ADDR/MMU_1M), (LITEOS_CACHE_LENGTH/MMU_1M), CACHEABLE, BUFFERABLE, ACCESS_RW);
}/*lint !e438  !e550*/

extern unsigned long g_usb_mem_size;
VOID osSecPageInit(VOID)
{
    extern unsigned long __rodata1_end;
    stOsPage.page_addr = SYS_MEM_BASE;
    stOsPage.page_length = ((((unsigned long)&__rodata1_end - SYS_MEM_BASE) + MMU_1M - 1)& ~ (MMU_1M - 1));
    stOsPage.page_descriptor_addr = (UINT32)second_page_table_os;
    stOsPage.page_type = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID;

    LOS_SecPageEnable(&stOsPage, BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RW_RW);

    stAppPage.page_addr = g_sys_mem_addr_end + g_usb_mem_size + MMZ_MEM_BASE_OFFSET;
    stAppPage.page_length = SYS_MEM_BASE + DDR_MEM_SIZE - stAppPage.page_addr;
    stAppPage.page_descriptor_addr = (UINT32)second_page_table_mmz;
    stAppPage.page_type = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID;

    LOS_SecPageEnable(&stAppPage, BUFFER_DISABLE|CACHE_DISABLE|ACCESS_PERM_RW_RW);
}

VOID osRemapCached(unsigned long phys_addr, unsigned long size)
{PRINTK("%s not support\n", __FUNCTION__);}
VOID osRemapNoCached(unsigned long phys_addr, unsigned long size)
{PRINTK("%s not support\n", __FUNCTION__);}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
