#ifndef __LINUX_PAGEMAP_H__
#define __LINUX_PAGEMAP_H__

#include <asm/bug.h>
#include <asm/page.h>



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PAGE_SHIFT  12
#define PAGE_CACHE_SHIFT        PAGE_SHIFT
#define PAGE_CACHE_SIZE         PAGE_SIZE

#define PageLocked(pg) 1
#define Page_Uptodate(pg) 0
#define UnlockPage(pg)
#define PAGE_BUG(pg) BUG()
#define ClearPageUptodate(pg)
#define SetPageError(pg)
#define ClearPageError(pg)
#define SetPageUptodate(pg)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __LINUX_PAGEMAP_H__ */
