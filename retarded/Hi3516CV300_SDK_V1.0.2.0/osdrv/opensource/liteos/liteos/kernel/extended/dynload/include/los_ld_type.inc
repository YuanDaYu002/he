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

/** @defgroup dynload dynamic loading
 *@ingroup kernel
 */

#ifndef _KERNEL_LOS_LD_TYPE_INC
#define _KERNEL_LOS_LD_TYPE_INC

#include "los_ld_elf.inc"
#include "los_ld_macro.inc"

#include "los_list.ph"
#include "los_typedef.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/* Maximum number of modules that are allowed to be loaded by Huawei LiteOS. */
#define N_MAX_USER_MODULES          10

#define N_MAX_MODULES               (N_MAX_USER_MODULES + 1)

/* Default size of the hash table. */
#define DEFAULT_HASH_SIZE_LOG2      0x08

/* Number of OS symbols. */
#define LENGTH_ARRAY                500

/* Maximum number of default paths */
#define N_DEFAULT_PATH  10

extern LOS_DL_LIST  stFreeHandleList;
extern VOID         *pDynloadHeap;

/* Symbol types */
#define ELF_STT_NOTYPE      0x0     /* Unknown type */
#define ELF_STT_OBJECT      0x1     /* Data object */
#define ELF_STT_FUNC        0x2     /* Function */
#define ELF_STT_SECTION     0x3     /* Section */
#define ELF_STT_FILE        0x4     /* File name*/

/* Symbol binding information */
#define ELF_STB_LOCAL       0x0     /* Local */
#define ELF_STB_GLOBAL      0x1     /* Global */
#define ELF_STB_WEAK        0x2     /* Weak reference */

/* Information about the section in which a symbol is located. The following are constants. */
#define ELF_SHN_ABS         0xFFF1  /* Absolute symbol value, for example, a file name. */
#define ELF_SHN_COMMON      0xFFF2  /* COMMON block */
#define ELF_SHN_UNDEF       0x0000  /* Undefined symbol */

/* Symbol types */
#define OS_SHN_UNDF         0x0     /* Undefined */
#define OS_SHN_LOCAL        0x0     /* Local */
#define OS_SHN_GLOBAL       0x1     /* Global (external) (ORed) */
#define OS_SHN_ABS          0x2     /* Absolute */
#define OS_SHN_TEXT         0x4     /* Text */
#define OS_SHN_DATA         0x6     /* Data */
#define OS_SHN_BSS          0x8     /* BSS */
#define OS_SHN_COMM         0x10    /* Common symbol */
#define OS_SHN_THUMB        0x80    /* Thumb function */

/* New load flags */
#define OS_NO_SYMBOLS       0x1     /* No symbols added to the target
                                       symbol table
                                       */
#define OS_LOCAL_SYMBOLS    0x2     /* Only internal symbols added to
                                       the target symbol table
                                       */
#define OS_GLOBAL_SYMBOLS   0x4     /* Only external symbols added to
                                       the target symbol table
                                       */
#define OS_ALL_SYMBOLS      (OS_LOCAL_SYMBOLS | OS_GLOBAL_SYMBOLS) /* All symbols added to the target
                                                                      symbol table
                                                                      */

#define ELF_BIND_LOCAL      0
#define ELF_BIND_GLOBAL     1
#define ELF_BIND_WEAK       2
#define ELF_BIND_LOPROC     13
#define ELF_BIND_HIPROC     15

#define ELF_TYPE_NOTYPE     0
#define ELF_TYPE_OBJECT     1
#define ELF_TYPE_FUNC       2
#define ELF_TYPE_SECTION    3
#define ELF_TYPE_FILE       4
#define ELF_TYPE_LOPROC     13
#define ELF_TYPE_HIPROC     15

#define MAX_SEARCH_SCOPE    50

#define OS_ELF32_ST_BIND(info)          ((info) >> 4)   /* Obtain the binding information of the symbol table */
#define OS_ELF32_ST_TYPE(info)          ((info) & 0xf)  /* Obtain the type of the symbol table */
#define OS_ELF32_ST_INFO(bind, scType)  (((bind) << 4) + ((scType) & 0xf))

#define OS_ELF32_R_SYM(info)            ((info) >> 8)   /* The upper 24 bits of r_info indicates the subscript of the relocation entrance symbol in the symbol table */
#define OS_ELF32_R_TYPE(info)           ((UINT8)(info))
#define OS_ELF32_R_INFO(sym, scType)    (((sym) << 8) + (UINT8)(scType))

/* 0~15 bits maintain the module temporary flags during loadding, unloadding and searching. */
#define IS_LOADDING(handle)             ((handle)->uwModuleStatus & (1 << 0))
#define IS_UNLOADDING(handle)           ((handle)->uwModuleStatus & (1 << 1))
#define IS_SEARCHING(handle)            ((handle)->uwModuleStatus & (1 << 2))
#define SET_LOADDING(handle)            ((handle)->uwModuleStatus |= (1 << 0))
#define SET_UNLOADDING(handle)          ((handle)->uwModuleStatus |= (1 << 1))
#define SET_SEARCHING(handle)           ((handle)->uwModuleStatus |= (1 << 2))
#define CLEAR_SEARCHING(handle)         ((handle)->uwModuleStatus &= (~(1 << 2)))
#define CLEAR_TMPFLAGS(handle)          ((handle)->uwModuleStatus &= 0xffff0000)
/* 16~31 bits maintain the module status after load. */
#define IS_LOADED(handle)               ((handle)->uwModuleStatus & (1 << 16))
#define IS_TAKEN(handle)                ((handle)->uwModuleStatus & (1 << 17))
#define SET_LOADED(handle)              ((handle)->uwModuleStatus |= (1 << 16))
#define SET_TAKEN(handle)               ((handle)->uwModuleStatus |= (1 << 17))
#define CLEAR_LOADED(handle)            ((handle)->uwModuleStatus &= (~(1 << 16)))
#define CLEAR_TAKEN(handle)             ((handle)->uwModuleStatus &= (~(1 << 17)))
#define CLEAR_MODULESTATUS(handle)      ((handle)->uwModuleStatus = 0)

#define INC_SOHEAD(handle)              ((handle)->swHeadCnt++)
#define DEC_SOHEAD(handle)              ((handle)->swHeadCnt--)
#define IS_SOHEAD(handle)               ((handle)->swHeadCnt > 0)




#define OS_RELOC_COPY_WORD(pDst, pSrc)  \
    do { \
        UINT32 *puwDst = (UINT32*)(pDst); \
        UINT32 *puwSrc = (UINT32*)(pSrc); \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc; \
    } while (0)

/* Relocation Type Definitions */
#define OS_R_ARM_NONE           0   /* No reloc */
#define OS_R_ARM_PC24           1
#define OS_R_ARM_ABS32          2
#define OS_R_ARM_REL32          3
#define OS_R_ARM_LDR_PC_G0      4
#define OS_R_ARM_ABS16          5
#define OS_R_ARM_ABS12          6
#define OS_R_ARM_THM_ABS5       7
#define OS_R_ARM_ABS8           8
#define OS_R_ARM_SBREL32        9
#define OS_R_ARM_THM_CALL       10
#define OS_R_ARM_THM_PC8        11
#define OS_R_ARM_BREL_ADJ       12
#define OS_R_ARM_TLS_DESC       13
#define OS_R_ARM_THM_SWI8       14
#define OS_R_ARM_XPC25          15
#define OS_R_ARM_THM_XPC22      16
#define OS_R_ARM_COPY           20  /* Copy symbol at runtime */
#define OS_R_ARM_GLOB_DAT       21  /* Create GOT entry */
#define OS_R_ARM_JUMP_SLOT      22  /* Create PLT entry */
#define OS_R_ARM_RELATIVE       23  /* Adjust by program base */
#define OS_R_ARM_GOTOFF32       24  /* 32 bit offset to GOT */
#define OS_R_ARM_BASE_PREL      25  /* 32 bit PC relative offset to GOT */
#define OS_R_ARM_GOT_BREL       26  /* 32 bit GOT entry */
#define OS_R_ARM_PLT32          27  /* 32 bit PLT address */
#define OS_R_ARM_THM_JUMP24     30
#define OS_ARM_THM_MOVW_ABS_NC  47
#define OS_ARM_THM_MOVT_ABS     48

/* These are GNU extensions to enable C++ vtable garbage collection. */
#define OS_R_ARM_GNU_VTENTRY    100
#define OS_R_ARM_GNU_VTINHERIT  101

#define OS_R_ARM_THM_JUMP11     102 /* Thumb unconditional branch */
#define OS_R_ARM_THM_JUMP8      103 /* Thumb conditional branch */
#define OS_R_ARM_RXPC25         249
#define OS_R_ARM_RSBREL32       250
#define OS_R_ARM_THM_RPC22      251
#define OS_R_ARM_RREL32         252
#define OS_R_ARM_RABS32         253
#define OS_R_ARM_RPC24          254
#define OS_R_ARM_RBASE          255

#define OS_BIT_AND(x, y)    ((x) & (y))

/**
* Macro capable of relocating ARM symbols
*/

/* Macro capable of modifying the lower X bits */
#define OS_INSERT(buf, value, mask) ((buf) = (((buf) & ~(mask)) | ((value) & (mask))))

#define OS_LOW24_INSERT(buf, value) OS_INSERT((buf), (value), 0x00ffffff)

#define OS_LOW16_INSERT(buf, value) OS_INSERT((buf), (value), 0x0000ffff)

#define OS_LOW12_INSERT(buf, value) OS_INSERT((buf), (value), 0x00000fff)

#define OS_LOW11_INSERT(buf, value) OS_INSERT((buf), (value), 0x000007ff)

#define OS_LOW8_INSERT(buf, value)  OS_INSERT((buf), (value), 0x000000ff)

#define OS_BITS6_10_INSERT(buf, value)  OS_INSERT((buf), (value) << 6, 0x000007c0)

#define OS_UBIT_INSERT(buf, value)  OS_INSERT((buf), (value) << 23 , 0x0080000)

/* The values of n bits of a binary number are 1 */
#define OS_MASK(n)  ((1 << (n)) - 1)

/* The value of bit n of a binary number is 1 */
#define OS_SIGN_BIT_MASK(n) (1 << ((n) - 1))

/* Add an extended bit to the upper bit */
#define OS_SIGN_EXTEND(val, nBits) \
    (((OS_MASK(nBits) & (val)) ^ OS_SIGN_BIT_MASK(nBits)) - OS_SIGN_BIT_MASK(nBits))

/* Check whether the extended bit value falls within the nBits range */
#define OS_CHECK_FITS(val, nBits) \
    ((((val) & ~((1 << ((nBits) - 1)) - 1)) == 0) || \
     (((val) & ~((1 << ((nBits) - 1)) - 1)) == ~(UINT32)((1 << ((nBits) - 1)) - 1)))

#define OS_MEM_READ_32(pRelocAdrs, offset)  ((offset) = *((UINT32 *)(pRelocAdrs)))
#define OS_MEM_READ_16(pRelocAdrs, offset)  ((offset) = *((UINT16 *)(pRelocAdrs)))
#define OS_MEM_READ_8(pRelocAdrs, offset)   ((offset) = *((UINT8 *)(pRelocAdrs)))

#define OS_MEM_WRITE_32(pRelocAdrs, value32) \
    (*((UINT32 *)(pRelocAdrs)) = (value32))
#define OS_MEM_WRITE_16(pRelocAdrs, value16) \
    (*((UINT16 *)(pRelocAdrs)) = (value16))
#define OS_MEM_WRITE_8(pRelocAdrs, value8) \
    (*((UINT8 *)(pRelocAdrs)) = (value8))

#define LD_ELF_RELOC_CopyByWord(pDst, pSrc) \
    do { \
        UINT32 *puwDst = (UINT32 *)(pDst); \
        UINT32 *puwSrc = (UINT32 *)(pSrc); \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc;  puwDst++; puwSrc++; \
        *puwDst = *puwSrc; \
    } while (0)

typedef struct LD_HASH_TBL {    /* LD_HASH_TBL */
    UINT32       uwElements;     /* Number of elements in table */
    UINT32       uwKeyArg;       /* Hash function argument */
    LOS_DL_LIST *pstHashTbl;    /* Pointer to hash table array */
} LD_HASH_TBL;

typedef struct LD_SYMTBL {      /* LD_SYMTBL - symbol table */
    LD_HASH_TBL *pstNameHashId; /* Hash table for names */
} LD_SYMTBL;

typedef struct LD_SYMBOL {      /* LD_SYMBOL - entry in symbol table */
    LOS_DL_LIST stNameHNode;    /* Hash node */
    LOS_DL_LIST pstModuleLinkNode;
    CHAR        *pscName;       /* Pointer to symbol name */
    INT8        *pscValue;      /* Symbol value */
    INT8        scType;         /* Symbol type */
} LD_SYMBOL;

/* Data structures */
typedef struct LD_IDX_TBLS {
    UINT32 *puwLoadScnHdrIdxs;          /* Loadable sections header index tbl */
    UINT32 *puwSymTabScnHdrIdxs;        /* Sym table sections header idx tbl */
    UINT32 *puwRelScnHdrIdxs;           /* Reloc info sections header idx tbl */
    UINT32 *puwStrTabScnHdrIdxs;        /* Str table sections header idx tbl */
} LD_IDX_TBLS;

typedef struct LD_SYM_INFO {
    VOID *pAddr;    /* Symbol's address */
    INT8 scType;    /* Symbol's type */
} LD_SYM_INFO;

typedef struct DYN_COMSYM_INFO {
    LOS_DL_LIST pstLinkNode;
    UINT32      uwSymAddr;
} DYN_COMSYM_INFO;

typedef struct DYN_SEC_INFO {
    LOS_DL_LIST pstLinkNode;
    UINT32      uwStartAddr;
    UINT32      uwSize;
    UINT32      uwType;
} DYN_SEC_INFO;

enum ELF_TYPE {
    ET_NONE, ET_SO, ET_OBJ, ET_EXEC
};

typedef struct DYN_MODULE_INFO {
    LOS_DL_LIST     stManageList;      /* Must comes first*/
    LOS_DL_LIST     stSegInfoList;
    LOS_DL_LIST     stSymInfoList;
    LOS_DL_LIST     stComSymInfoList;
    UINT32          uwSegmentsAddr;
    UINT32          uwSegmentsSize;
    UINT32          uwModuleStatus;
    INT32           swRefCnt;
    INT32           swHeadCnt;
    enum ELF_TYPE   enType;
    CHAR            *pscModuleFileName;
    struct DYN_MODULE_INFO  *apDlNeeded[N_MAX_MODULES];
} DYN_MODULE_INFO;

typedef struct {
    LOS_DL_LIST     stDlLinkNode;

    INT32           swLoadStart;
    INT32           swLoadBias;

    DYN_MODULE_INFO *pstModuleHandle;

    LD_ELF32_PHDR   *pstPhdr;
    UINT32           uwPhnum;

    LD_ELF32_DYN    *pstDynamic;

    CHAR            *pscStrtab;
    UINT32          uwStrtabSize;

    LD_ELF32_SYM    *pstSymtab;
    UINT32           uwSymnum;

    LD_ELF32_REL    *pstRel;
    UINT32          uwRelCount;
} DL_INFO;

typedef struct KERNEL_SYMBOL {
    UINT32  uwAddr;
    CHAR    *pscName;
} KERNEL_SYMBOL;

extern KERNEL_SYMBOL los_elf_symbol_table[LENGTH_ARRAY];
typedef KERNEL_SYMBOL (*OS_SYMTBL_ARRAY_PTR)[LENGTH_ARRAY];

/* initLib */
extern LD_SYMTBL *g_pSymTbl;
extern DYN_MODULE_INFO stSysModuleInfo[N_MAX_MODULES];

typedef INT32 (*LOADSTRATEGY_FUNC)(DL_INFO *pstDlInfo);

/* commLib */
extern INT32 LD_ElfSymTablesHandle(UINT32 *puwSymTabScnHdrIdxs, LD_ELF32_SHDR *pstScnHdrTbl, UINT32 uwModuleAddr,
        LD_ELF32_SYM ***pppstSymTblRefs, LD_SYM_INFO ***pppstSymsAdrsRefs, INT32 *swRetSymtabNum);
extern UINT32 LD_ElfScnHdrCheck(LD_ELF32_SHDR *pstScnHdr, UINT32 uwScnHdrNum);
extern BOOL LD_ElfSymIsVisible(UINT32 uwSymAssoc, UINT32 uwSymBinding, INT32 swLoadFlag);
extern INT8 LD_ElfSymTypeGet(LD_ELF32_SYM *pstSymbol, LD_ELF32_SHDR *pstScnHdrTbl);
extern VOID LD_ElfBufferFree(VOID **ppBuf);
extern UINT32 LD_LoadFileToMem(INT8 *pscElfFileName);

/* hashLib */
extern LD_HASH_TBL *LD_SymtblHashTblCreate(UINT32 uwSizeLog2, UINT32 uwKeyArg);
extern UINT32 LD_SymtblHashTblDelete(LD_HASH_TBL *pstHash);
extern VOID LD_SymtblHashTblPut(LD_HASH_TBL *psthash, LD_SYMBOL *pstSymbol, UINT32 uwIdx);
extern LD_SYMBOL *LD_SymtblHashTblFind(LD_HASH_TBL *pstHash, LD_SYMBOL *pstSymbol, UINT32 *uwRetIdx);
extern LD_SYMBOL *LD_SymtblHashTblFindRedef(LD_HASH_TBL *pstHash, LD_SYMBOL *pstSymbol);
extern INT32 LD_FfsMsb(UINT32 uwIdx);

/* symtblLib */
extern UINT32 LD_SecInfoAdd(UINT32 uwStartAddr, UINT32 uwSecSize, UINT32 uwSecType, DYN_MODULE_INFO *pstModuleInfo);
extern UINT32 LD_ElfAlignGet(UINT32 uwAlignment, VOID *pAddrOrSize);
extern UINT32 LD_CommonManage(DYN_MODULE_INFO *pstModuleHandle, UINT32 uwComAreaSize, INT32 swComAlignment, CHAR *pscSymName,
        VOID **ppSymAddr, INT8 *pscSymType, INT32 swLoadFlag);

/* moduleLib */
extern DYN_MODULE_INFO *LD_ModuleHandleAlloc(CHAR *pscElfFileName);
extern INT32 LD_ModuleMemRelease(DYN_MODULE_INFO *pstModuleHandle);
extern INT32 LD_ModuleDeinit(DYN_MODULE_INFO *pstModuleHandle);

/* symLib */
extern UINT32 LD_SymHFuncName(UINT32 uwElements, LD_SYMBOL *pstSymbol, UINT32 uwSeed);
extern LD_SYMTBL* LD_SymTblCreate(INT32 swHashSizeLog2);
extern LD_SYMBOL *LD_SymAlloc(CHAR *pscName, INT8 *pscValue, INT8 scType);
extern UINT32 LD_SymAdd(DYN_MODULE_INFO *pstModuleHandle, CHAR *pscName, INT8 *pscValue, INT8 stType);
extern INT32 LD_SymRemoveByModuleHandle(DYN_MODULE_INFO *pstModuleHandle);

extern UINT32 LD_ObjARMReloc(UINT32 uwModuleAddr, INT32 swPosCurRelocCmd, LD_ELF32_SHDR *pstScnHdrTbl,
        LD_ELF32_SHDR *pstRelHdr, VOID **ppstScnAddr,  LD_SYM_INFO *pstSymInfoTbl, LD_ELF32_SYM *pstSymsArray);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _KERNEL_LOS_LD_TYPE_INC */
