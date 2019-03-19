/*
 *  arch/arm/include/asm/assembler.h
 *
 *  Copyright (C) 1996-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  This file contains arm architecture specific defines
 *  for the different processors.
 *
 *  Do not include any C declarations in this file - it is included by
 *  assembler source.
 */
#ifndef __ASM_ASSEMBLER_H__
#define __ASM_ASSEMBLER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
  * Endian independent macros for shifting bytes within registers.
  */
 #ifndef __ARMEB__ //big endian
 #define pull            lsr
 #define push            lsl
 #define get_byte_0      lsl #0
 #define get_byte_1  lsr #8
 #define get_byte_2  lsr #16
 #define get_byte_3  lsr #24
 #define put_byte_0      lsl #0
 #define put_byte_1  lsl #8
 #define put_byte_2  lsl #16
 #define put_byte_3  lsl #24
 #else
 #define pull            lsl
 #define push            lsr
 #define get_byte_0  lsr #24
 #define get_byte_1  lsr #16
 #define get_byte_2  lsr #8
 #define get_byte_3      lsl #0
 #define put_byte_0  lsl #24
 #define put_byte_1  lsl #16
 #define put_byte_2  lsl #8
 #define put_byte_3      lsl #0
 #endif


/*
 * On Feroceon there is much to gain however, regardless of cache mode.
 */
#define CALGN(code...)

/*
 * Data preload for architectures that support it
 */
#define PLD(code...)    code

/*
 * Endian independent macros for shifting bytes within registers.
 */
#define put               lsl
#define get                 lsr

#define push               lsl
#define pull                 lsr

#define load(code...)   code
#define store(code...)  code

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* __ASM_ASSEMBLER_H__ */
