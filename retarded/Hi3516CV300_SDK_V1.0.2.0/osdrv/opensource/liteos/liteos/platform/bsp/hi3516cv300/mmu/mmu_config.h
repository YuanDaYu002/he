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

/** @defgroup mmu_config Memory management unit config
 *  @ingroup kernel
*/

#ifndef __MMU_CONFIG_H
#define __MMU_CONFIG_H

#include "board.h"
#include "los_config.h"
#include "los_memory.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*************************************common***************************************/
/**
 * @ingroup mmu_config
 * The access permission mode is read-only.
 */
#define ACCESS_PERM_RO_RO                   0

/**
 * @ingroup mmu_config
 * The access permission mode is read and write.
 */
#define ACCESS_PERM_RW_RW                   (1<<0)

/**
 * @ingroup mmu_config
 * The cache enabled.
 */
#define CACHE_ENABLE                   (1<<1)

/**
 * @ingroup mmu_config
 * The cache disabled.
 */
#define CACHE_DISABLE                   0

/**
 * @ingroup mmu_config
 * The buffer enabled.
 */
#define BUFFER_ENABLE                   (1<<2)

/**
 * @ingroup mmu_config
 * The buffer disabled.
 */
#define BUFFER_DISABLE                   0

/**
 * @ingroup mmu_config
 * The buffer enabled.
 */
#define FIRST_SECTION                  (1<<3)

/**
 * @ingroup mmu_config
 * The buffer enabled.
 */
#define SECOND_PAGE             0

#define MMU_1K    0x400
#define MMU_4K    0x1000
#define MMU_16K    0x4000
#define MMU_64K    0x10000
#define MMU_1M    0x100000
#define MMU_4G     0xffffffff

/**
 * @ingroup mmu_config
 * mmu second page information structure.
 *
 */
typedef struct
{
    UINT32 page_addr;           /**< The second page start addr         */
    UINT32 page_length;         /**< The second page length             */
    UINT32 page_descriptor_addr;    /**< The second page page table storage addr, diff second page table can't be coincided   */
    UINT32 page_type;           /**< The second page type, it can be set small page ID(4K):MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID
                                                                                            or big page ID(64K):MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID        */
}SENCOND_PAGE;

/**
 * @ingroup mmu_config
 * mmu param setting information structure.
 *
 */
typedef struct
{
    UINT32 startAddr;      /**< Starting address of a section.     */
    UINT32 endAddr;       /**< Ending address of a section.     */
    UINT32 uwFlag;         /*<   mode set. There are three func could be controlled with three bit.
                                                bit0: ACCESS_PERM_RW_RW/ACCESS_PERM_RO_RO(1/0)
                                                bit1: CACHE_ENABLE/CACHE_DISABLE(1/0)
                                                bit2: BUFFER_ENABLE/BUFFER_DISABLE(1/0)
                                                bit3: FIRST_SECTION/SECOND_PAGE(1/0) it need comfire your memory type, be descripted
                                                bit4~7: ignore*/
    SENCOND_PAGE *stPage;    /*<  the goal object of second page, if uwFlag bit3 is FIRST_SECTION, stPage will be ignored, and you can set this member as NULL */
}MMU_PARAM;

/**
 * @ingroup mmu_config
 * The liteos cache addr & length
 */
#define LITEOS_CACHE_ADDR               SYS_MEM_BASE
#define LITEOS_CACHE_LENGTH           (g_sys_mem_addr_end - LITEOS_CACHE_ADDR)

/**
 * @ingroup mmu_config
 * The page table storage addr
 * notice: must ensure it has enough free mem for storage page table
 */
#define FIRST_PAGE_DESCRIPTOR_ADDR            ((UINT32)first_page_table)
/****************************************depend on cpu**********************************************/

// ARM Domain Access Control Bit Masks
#define ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << ((domain_num)*2))
#define ACCESS_TYPE_CLIENT(domain_num)       (0x1 << ((domain_num)*2))
#define ACCESS_TYPE_MANAGER(domain_num)      (0x3 << ((domain_num)*2))

#define MMU_FIRST_LEVEL_FAULT_ID 0x0
#define MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1
#define MMU_FIRST_LEVEL_SECTION_ID 0x2
#define MMU_FIRST_LEVEL_RESERVED_ID 0x3
/**
 * @ingroup mmu_config
 * The second page type select 64K
 */
#define MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID 0x1

/**
 * @ingroup mmu_config
 * The second page type select 4K
 */
#define MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID 0x2

struct MMU_FIRST_LEVEL_FAULT {
    UINT32 id : 2;
    UINT32 sbz : 30;
};

struct MMU_FIRST_LEVEL_PAGE_TABLE {
    UINT32 id : 2;  // [1:0]
    UINT32 imp1 : 2; // [3:2]
    UINT32 imp2 :1; // [4]
    UINT32 domain : 4; // [8:5]
    UINT32 sbz : 1; // [9]
    UINT32 base_address : 22; // [31:10]
};

struct MMU_SECOND_LEVEL_BIG_PAGE_TABLE {
    UINT32 id : 2; // [1:0]
    UINT32 b : 1; // [2]
    UINT32 c : 1; // [3]
    UINT32 ap0 : 2; // [5:4]
    UINT32 ap1 : 2; // [7:6]
    UINT32 ap2 : 2; // [9:8]
    UINT32 ap3 : 2; // [11:10]
    UINT32 rev : 4; // [15:12]
    UINT32 base_address : 16; // [31:16]
};


struct MMU_SECOND_LEVEL_SMALL_PAGE_TABLE {
    UINT32 id : 2; // [1:0]
    UINT32 b : 1; // [2]
    UINT32 c : 1; // [3]
    UINT32 ap0 : 2; // [5:4]
    UINT32 ap1 : 2; // [7:6]
    UINT32 ap2 : 2; // [9:8]
    UINT32 ap3 : 2; // [11:10]
    UINT32 base_address : 20; // [31:12]
};

struct MMU_FIRST_LEVEL_SECTION {
    UINT32 id : 2;
    UINT32 b : 1;
    UINT32 c : 1;
    UINT32 imp : 1;
    UINT32 domain : 4;
    UINT32 sbz0 : 1;
    UINT32 ap : 2;
    UINT32 sbz1 : 8;
    UINT32 base_address : 12;
};

struct MMU_FIRST_LEVEL_RESERVED {
    UINT32 id : 2;
    UINT32 sbz : 30;
};

#define X_MMU_SECTION(abase,vbase,size,cache,buff,access)      \
    { \
        UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
        desc.word = 0;       \
        desc.section.id = MMU_FIRST_LEVEL_SECTION_ID;                 \
        desc.section.c = (cache);                                     \
        desc.section.b = (buff);                                    \
        desc.section.imp = 1;                                             \
        desc.section.domain = 0;                                          \
        desc.section.ap = (access);                                         \
        k <<= 2;                        \
        k += ttb_base;      \
        for (i = size + j; i > j; ++j, k+=4)                          \
        {                                                            \
            desc.section.base_address = (j);                        \
            *(UINT32 *)k = desc.word;                                            \
        }                                                            \
    }

#define X_MMU_ONE_LEVEL_PAGE(abase, vbase, size)      \
    { \
        UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
        desc.word = 0;                                                    \
        desc.page_table.id = MMU_FIRST_LEVEL_PAGE_TABLE_ID;                 \
        desc.page_table.imp1 = 0;                                             \
        desc.page_table.imp2 = 0x1;                                             \
        desc.page_table.domain = 0x1;                                          \
        desc.page_table.sbz = 0x1;                                          \
        k <<= 2;                        \
        k += ttb_base;      \
        for (i = size + j; i > j; ++j, k+=4)                          \
        {                                                            \
            desc.page_table.base_address = (j);                        \
            *(UINT32 *)k = desc.word;                                            \
        }                                                            \
    }

#define X_MMU_TWO_LEVEL_PAGE_RW(abase, vbase, size, cache, buff, access)      \
    {  \
        UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
        desc.word = 0;                                                    \
        desc.small_page_table.id = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID;                 \
        desc.small_page_table.b = (buff);                                             \
        desc.small_page_table.c = (cache);                                         \
        desc.small_page_table.ap0 = (0x3);                                     \
        desc.small_page_table.ap1 = (0x3);                                     \
        desc.small_page_table.ap2 = (0x3);                                     \
        desc.small_page_table.ap3 = (0x3);                                     \
        k <<= 2;                        \
        k += stt_base;      \
        for (i = size + j; i > j; ++j, k+=4)                          \
        {                                                            \
            desc.small_page_table.base_address = (j);                        \
            *(UINT32 *)k = desc.word;                                            \
        }                                                            \
    }

#define X_MMU_TWO_LEVEL_PAGE_RO(abase, vbase, size, cache, buff, access)      \
    {  \
        UINT32 i; UINT32 j = abase; UINT32 k = vbase;                         \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
        desc.word = 0;                                                                       \
        desc.small_page_table.id = MMU_SECOND_LEVEL_SMALL_PAGE_TABLE_ID;                 \
        desc.small_page_table.b = (buff);                                             \
        desc.small_page_table.c = (cache);                                         \
        desc.small_page_table.ap0 = (0);                                     \
        desc.small_page_table.ap1 = (0);                                     \
        desc.small_page_table.ap2 = (0);                                     \
        desc.small_page_table.ap3 = (0);                                     \
        k <<= 2;                        \
        k += stt_base;      \
        for (i = size + j; i > j; ++j, k+=4)                          \
        {                                                            \
            desc.small_page_table.base_address = (j);                        \
            *(UINT32 *)k = desc.word;                                            \
        }                                                            \
    }

#define X_MMU_TWO_LEVEL_PAGE64K_RW(abase, vbase, size, cache, buff, access)      \
    {  \
        UINT32 i; UINT32 j = abase; UINT32 k = vbase; UINT32 n;                         \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
        desc.word = 0;                                                    \
        desc.big_page_table.id = MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID;                 \
        desc.big_page_table.b = (buff);                                             \
        desc.big_page_table.c = (cache);                                         \
        desc.big_page_table.ap0 = (0x3);                                     \
        desc.big_page_table.ap1 = (0x3);                                     \
        desc.big_page_table.ap2 = (0x3);                                     \
        desc.big_page_table.ap3 = (0x3);                                     \
        k <<= 2;                        \
        k += stt_base;      \
        for (i = size + j; i > j; ++j)                          \
        {                                                            \
            desc.big_page_table.base_address = (j);                        \
            for (n = 0; n < 16; ++n, k += 4)   \
                *(UINT32 *)k = desc.word;                                            \
        }                                                            \
    }

#define X_MMU_TWO_LEVEL_PAGE64K_RO(abase, vbase, size, cache, buff, access)      \
    {  \
        UINT32 i; UINT32 j = abase; UINT32 k = vbase; UINT32 n;                        \
        union MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
        desc.word = 0;                                                                       \
        desc.big_page_table.id = MMU_SECOND_LEVEL_BIG_PAGE_TABLE_ID;                 \
        desc.big_page_table.b = (buff);                                             \
        desc.big_page_table.c = (cache);                                         \
        desc.big_page_table.ap0 = (0);                                     \
        desc.big_page_table.ap1 = (0);                                     \
        desc.big_page_table.ap2 = (0);                                     \
        desc.big_page_table.ap3 = (0);                                     \
        k <<= 2;                        \
        k += stt_base;      \
        for (i = size + j; i > j; ++j)                          \
        {                                                            \
            desc.big_page_table.base_address = (j);                        \
            for (n = 0; n < 16; ++n, k += 4)   \
                *(UINT32 *)k = desc.word;                                            \
        }                                                            \
    }


union MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct MMU_FIRST_LEVEL_FAULT fault;
    struct MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct MMU_FIRST_LEVEL_SECTION section;
    struct MMU_FIRST_LEVEL_RESERVED reserved;
    struct MMU_SECOND_LEVEL_SMALL_PAGE_TABLE small_page_table;
    struct MMU_SECOND_LEVEL_BIG_PAGE_TABLE big_page_table;
};

#define MMU_GET_FIRST_TABLE_ADDR(addr)    (((addr)/MMU_1M)*4 + FIRST_PAGE_DESCRIPTOR_ADDR)  //table start position + offset = 'addr' table item position
#define MMU_GET_FIRST_TABLE_ITEM(addr)    (*(UINT32 *)MMU_GET_FIRST_TABLE_ADDR(addr)) // get item content which storaged by table
#define MMU_GET_SECOND_TABLE_BASE(addr) ((MMU_GET_FIRST_TABLE_ITEM(addr)) & 0xfffffc00) //if the first item ID is MMU_FIRST_LEVEL_PAGE_TABLE_ID, get second table item addr by hi 22bits
#define MMU_GET_SECOND_TABLE_OFFSET(addr) (((addr) % MMU_1M)/MMU_1K) // second table item offset
#define MMU_GET_SECOND_TABLE_ADDR(addr)  (MMU_GET_SECOND_TABLE_BASE(addr) + MMU_GET_SECOND_TABLE_OFFSET(addr))

/**
 *@ingroup mmu_config
 *@brief Memory Management Unit Cache/Buffer/Access Permission Setting.
 *
 *@par Description:
 *This API is used to set the Cache/Buffer/access permission mode of a section that is specified by a starting address and ending address
 *@attention
 *<ul>
 *<li>The passed-in starting address and ending address must be aligned on a boundary of 4K. The access permission mode can be only set to ACCESS_PERM_RO_RO and ACCESS_PERM_RW_RW.</li>
 *</ul>
 *
 *@param MMU_PARAM  [IN] param for mmu setting, the struct contains below members
 * startAddr:     Starting address of a section.
 * endAddr:      Ending address of a section.
 * uwFlag:        mode set. There are three func could be controlled with three bit.
 *                                   bit0: ACCESS_PERM_RW_RW/ACCESS_PERM_RO_RO(1/0)
 *                                   bit1: CACHE_ENABLE/CACHE_DISABLE(1/0)
 *                                   bit2: BUFFER_ENABLE/BUFFER_DISABLE(1/0)
 *                                   bit3: FIRST_SECTION/SECOND_PAGE(1/0) it need comfire your memory type, be descripted
 *                                   bit4~7: ignore
 * stPage:      the goal object of second page, if uwFlag bit3 is FIRST_SECTION, stPage will be ignored, and you can set this member as NULL
 *
 *@retval None.
 *@par Dependency:
 *<ul><li>mmu_config.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R001C00
 */
void LOS_MMUParamSet(MMU_PARAM *mPara);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
