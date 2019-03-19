#ifndef __ASM_ATOMIC_H__
#define __ASM_ATOMIC_H__

#include "los_atomic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define atomic_t int
#define atomic_inc(atom)     LOS_AtomicInc(atom)
#define atomic_dec(atom)    LOS_AtomicDec(atom)
#define atomic_sub(n, v)      LOS_AtomicAdd(v, -n)
#define atomic_add(n, v)     LOS_AtomicAdd(v, n)
#define atomic_read(atom)    (*((volatile typeof(atom))(atom)))
#define atomic_set(p, v)      LOS_AtomicXchg32bits(p, v)
#define ATOMIC_INIT(x)     (x)

#define atomic_add_return(i, v)    LOS_AtomicAdd(v, i)
#define atomic_inc_return(v)        LOS_AtomicIncRet(v)
#define atomic_dec_return(v)       LOS_AtomicDecRet(v)
#define atomic_dec_and_test(v)    (atomic_dec_return(v) == 0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __ASM_ATOMIC_H__ */
