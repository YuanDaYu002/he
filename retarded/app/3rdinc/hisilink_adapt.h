/******************************************************************************
  Copyright (C), 2004-2050, Hisilicon Tech. Co., Ltd.
******************************************************************************
  File Name     : hisilink_adapt.h
  Version       : Initial Draft
  Author        : Hisilicon WiFi software group
  Created       : 2016-06-06
  Last Modified :
  Description   : the API of hisilink for user calls
  Function List :
  History       :
  1.Date        : 2016-06-06
  Author        :
  Modification  : Created file
******************************************************************************/
#ifndef __HISILINK_ADAPT_H__
#define __HISILINK_ADAPT_H__
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "hisilink_lib.h"
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define HSL_CHANNEL_NUM     18
#define HSL_RSSI_LEVEL      -100 //hsl�����鲥����RSSI����ֵ
typedef void (*hsl_connect_cb)(hsl_result_stru*);
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
typedef enum
{
    RECEIVE_FLAG_OFF,
    RECEIVE_FLAG_ON,
    RECEIVE_FLAG_BUTT
}recevie_flag_enum;
/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/
typedef struct
{
    hsl_uint32                ul_taskid1;       //HSL config�Ǵ�ͳ���Ӵ����̵߳�ID
    hsl_uint32                ul_taskid2;       //��ͳ���ӷ�ʽ�����߳�ID
    hsl_uint16                us_timerout_id;   //HSL config��ʱ��ʱ��
    hsl_uint16                us_timer_id;     //�������ŵ��Ķ�ʱ��ID
    hsl_uint8                 en_status;       //HSL config��ǰ����״̬
    hsl_uint8                 en_connect;      //��ǰ����״̬
    hsl_uint8                 auc_res[2];
    hsl_uint32                ul_mux_lock;     //������
}hisi_hsl_status_stru;

typedef struct hsl_datalist
{
    struct hsl_datalist *next;
    void *data;
    unsigned int length;
}hsl_data_list_stru;
/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
hsl_int32 hisi_hsl_adapt_init(void);
hsl_int32 hisi_hsl_adapt_deinit(void);
hsl_int32 hisi_hsl_change_channel(void);
hsl_int32 hisi_hsl_process_data(void);
hsl_int32 hisi_hsl_online_notice(hsl_result_stru *pst_result);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif //__HSLCONFIG_ADAPT_H__

