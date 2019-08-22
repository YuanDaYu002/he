
#ifndef HLE_WATCHDOG_H
#define HLE_WATCHDOG_H

#include "typeport.h"



#ifdef __cplusplus
extern "C"
{
#endif



#define DEBUG_WATCHDOG  1  //调试

/*******************************************************************************
*@ Description    :看门狗模块初始化
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int hle_watchdog_init(void);


/*******************************************************************************
*@ Description    :喂狗函数
*@ Input          :
*@ Output         :
*@ Return         :成功 ： 0
*@ attention      :
*******************************************************************************/
int hle_watchdog_feed(void);

/*******************************************************************************
*@ Description    :退出看门狗
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int hle_watchdog_exit(void);



#ifdef __cplusplus
}
#endif

#endif

