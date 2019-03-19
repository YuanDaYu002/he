#ifndef __HISOC_RANDOM_H_
#define __HISOC_RANDOM_H_

/************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
//0:not support    1:support
inline int hi_random_hw_support(void)
{
    return 1;
}

int hi_random_hw_init(void);
int hi_random_hw_deinit(void);
int hi_random_hw_getnumber(char *buffer, size_t buflen);
int hi_random_hw_getinteger(unsigned int *pResult);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif
