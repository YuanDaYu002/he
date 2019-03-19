/* @(#)s_modf.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
 
#include "los_printf.h"
#include "libcmini.h"

#ifdef LOSCFG_LIB_LIBCMINI
int finite(double x)
{
    int hx;
    GET_HIGH_WORD(hx,x);
    return (int)((unsigned int)((hx&0x7fffffff)-0x7ff00000)>>31);
}

int __fpclassifyd(double d) {
  double_u u;
  u.d = d;
  if (u.bits.dbl_exp == 0) {
    return ((u.bits.dbl_fracl | u.bits.dbl_frach) == 0) ? FP_ZERO : FP_SUBNORMAL;
  }
  if (u.bits.dbl_exp == DBL_EXP_INFNAN) {
    return ((u.bits.dbl_fracl | u.bits.dbl_frach) == 0) ? FP_INFINITE : FP_NAN;
  }
  return FP_NORMAL;
}

int __isnan(double d)
{
    return (__fpclassifyd(d) == FP_NAN);
}

int isnan(double d)
{
    return __isnan(d);
}

size_t strlen(const char *str)
{
    const char *s;

    for (s = str; *s; ++s)
        ;
    return (s - str);
}

static const double one = 1.0;
double modf(double x, double *iptr)
{
    int i0,i1,j0;
    unsigned int i;
    EXTRACT_WORDS(i0,i1,x);
    j0 = ((i0>>20)&0x7ff)-0x3ff;    /* exponent of x */
    if(j0<20) {         /* integer part in high x */
        if(j0<0) {          /* |x|<1 */
            INSERT_WORDS(*iptr,i0&0x80000000,0);    /* *iptr = +-0 */
        return x;
        } else {
        i = (0x000fffff)>>j0;
        if(((i0&i)|i1)==0) {        /* x is integral */
            unsigned int high;
            *iptr = x;
            GET_HIGH_WORD(high,x);
            INSERT_WORDS(x,high&0x80000000,0);  /* return +-0 */
            return x;
        } else {
            INSERT_WORDS(*iptr,i0&(~i),0);
            return x - *iptr;
        }
        }
    } else if (j0>51) {     /* no fraction part */
        unsigned int high;
        if (j0 == 0x400) {      /* inf/NaN */
        *iptr = x;
        return 0.0 / x;
        }
        *iptr = x*one;
        GET_HIGH_WORD(high,x);
        INSERT_WORDS(x,high&0x80000000,0);  /* return +-0 */
        return x;
    } else {            /* fraction part in low x */
        i = ((unsigned int)(0xffffffff))>>(j0-20);
        if((i1&i)==0) {         /* x is integral */
            unsigned int high;
        *iptr = x;
        GET_HIGH_WORD(high,x);
        INSERT_WORDS(x,high&0x80000000,0);  /* return +-0 */
        return x;
        } else {
            INSERT_WORDS(*iptr,i0,i1&(~i));
        return x - *iptr;
        }
    }
}

int raise(int sig)
{
    PRINT_ERR("%s NOT SUPPORT\n", __FUNCTION__);
    return 0;
}

#endif /* LOSCFG_LIB_LIBCMINI */

