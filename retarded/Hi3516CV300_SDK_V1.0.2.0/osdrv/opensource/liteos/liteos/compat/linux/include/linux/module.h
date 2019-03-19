#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/list.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define module_init(f)

#define module_exit(f)

#define module_param_named(u, v , t, f)
#define MODULE_PARM_DESC(_parm, desc)

#define MODULE_AUTHOR(a)
#define MODULE_DESCRIPTION(d)
#ifndef MODULE_LICENSE
#define MODULE_LICENSE(s)
#endif
#define MODULE_VERSION(v)
#define module_param_array(name, type, nump, perm)

struct module
{
};

static inline int try_module_get(struct module *module)
{
    return 1;
}

static inline void module_put(struct module *module)
{
}

#define THIS_MODULE ((struct module *)0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LINUX_MODULE_H */
