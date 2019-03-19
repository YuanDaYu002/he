/* $OpenBSD: ctype_private.h,v 1.1 2005/08/08 05:53:00 espie Exp $ */
/* Written by Marc Espie, public domain */
#ifndef _INCLUDE_CTYPE_PRIVATE_H
#define _INCLUDE_CTYPE_PRIVATE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define CTYPE_NUM_CHARS       256
extern const char _C_ctype_[];
extern const short _C_toupper_[];
extern const short _C_tolower_[];

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _INCLUDE_CTYPE_PRIVATE_H */

