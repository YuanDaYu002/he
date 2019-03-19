#ifndef _LINUX_LINKAGE_H
#define _LINUX_LINKAGE_H

#include <linux/compiler.h>
#include <linux/stringify.h>
#include <asm/linkage.h>

#ifdef __cplusplus
#define CPP_ASMLINKAGE    extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#ifndef asmlinkage
#define asmlinkage    CPP_ASMLINKAGE
#endif

#ifndef __ALIGN
#define __ALIGN_STR    ".align 4,0x90"
#define __ALIGN    .align 4,0x90
#endif

#ifdef __ASSEMBLY__

#define ALIGN_STR    __ALIGN_STR
#define ALIGN    __ALIGN

#ifndef END
#define END(name)    .size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
  .type name, @function; \
  END(name)
#endif

#ifndef ENTRY
#define ENTRY(name) \
  .globl name; \
  ALIGN; \
  name:
#endif

#endif
#endif
