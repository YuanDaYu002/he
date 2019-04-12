 
/***************************************************************************
* @file: hls_print.h
* @author:   
* @date:  4,10,2019
* @brief:  控制打印输出
* @attention:
***************************************************************************/
#ifndef _HLS_PRINT_H
#define _HLS_PRINT_H
#include "typeport.h"

//打印控制开关
//#define HLS_DEBUG
#define HLS_ERROR
#define HLS_FATAR

#ifdef HLS_DEBUG
#define HLS_DEBUG_LOG(args...)  \
		DEBUG_LOG(args)
#else
#define HLS_DEBUG_LOG(args...)
#endif  /*HLS_DEBUG*/


#ifdef HLS_ERROR
#define HLS_ERROR_LOG(args...)  \
		ERROR_LOG(args)
#else
#define HLS_ERROR_LOG(args...)
#endif  /*HLS_ERROR*/


#ifdef HLS_FATAR
#define HLS_FATAR_LOG(args...)  \
		FATAR_LOG(args)
#else
#define HLS_FATAR_LOG(args...)
#endif  /*HLS_FATAR*/




#endif


