/******************************************************************************
  Copyright (C), 2004-2050, Hisilicon Tech. Co., Ltd.
******************************************************************************
  File Name     : hisilink_lib.h
  Version       : Initial Draft
  Author        : Hisilicon WiFi software group
  Created       : 2016-06-06
  Last Modified :
  Description   : header file for hisilink_lib.c
  Function List :
  History       :
  1.Date        : 2016-06-06
  Author        :
  Modification  : Created file
******************************************************************************/
#ifndef __HISILINK_LIB_H__
#define __HISILINK_LIB_H__
#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif
/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#ifndef hsl_uint8
#define hsl_uint8 unsigned char 
#endif
#ifndef hsl_int8
#define hsl_int8  char 
#endif
#ifndef hsl_uint16
#define hsl_uint16 unsigned short
#endif
#ifndef hsl_uint32
#define hsl_uint32 unsigned int
#endif
#ifndef hsl_int32
#define hsl_int32  int
#endif
#define IP_ADDR_LEN           4
#define CRC_DATA_LEN          2
#define HSL_IV_LEN          16//IV�����ĳ���
#define HSL_NULL            (0L)
#define HSL_START_FRAME_NUM 3  //��ʼ���ĸ���
#define HSL_LOCK_FRAME_NUM  5  //���ŵ����հ��ĸ���
#define HSL_LOCK_TIME       2000 //���ŵ���ʱ��ms
/* ����AP������� */
#define HSL_SSID_HEAD             "Hisi_"
#define HSL_SSID_HEAD_LEN         (sizeof(HSL_SSID_HEAD)-1)
#define HSL_SSID_VERSION_LEN      1
#define HSL_SSID_MANUFACTUREF_LEN 3
#define HSL_SSID_DEVICE_LEN       3
#define HSL_SSID_MAC_LEN          12
#define HSL_SSID_LEN              32
#define HSL_MAC_LEN               6
#define MAC_TO_ASSIC_H(x) (((x & 0xf0) >> 4) > 9?(((x & 0xf0) >> 4) + 55):(((x & 0xf0) >> 4) + 48))
#define MAC_TO_ASSIC_L(x) ((x & 0x0f) > 9?((x & 0x0f) + 55):((x & 0x0f) + 48))
#define HSL_SSID_LEN_MAX         64
#define HSL_PWD_LEN_MAX          128

/* ����֪ͨ��� */
#define HSL_ONLINE_HEAD      "online:"
#define HSL_ONLINE_HEAD_LEN  (sizeof(HSL_ONLINE_HEAD)-1)
/* �����ȶ��� */
#define HSL_KEY_FRAME_OPEN_LEN   68//����KEY��Ϣ�Ǽ���֡����
#define HSL_KEY_FRAME_WEP_LEN    76
#define HSL_KEY_FRAME_TKIP_LEN   88
#define HSL_KEY_FRAME_AES_LEN    84

#define HSL_DATA_FRAME_OPEN_LEN  74
#define HSL_DATA_FRAME_WEP_LEN   82
#define HSL_DATA_FRAME_TKIP_LEN  94
#define HSL_DATA_FRAME_AES_LEN   90

/* AES��ͬλ������ʱЯ��KEY��Ϣ�ֽڸ���,����IV������16�ֽں�KEY���ܺ�ĳ��� */
//#define HSL_KEY_DATA_LEN  32 //AES-128bit
//#define HSL_KEY_DATA_LEN  48 //AES-192bit
#define HSL_KEY_DATA_LEN  64 //AES-256bit
/* AES�ӽ���λ��128bit 192bit 256bit */
#define HSL_AES_BITS_LEN  256
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
typedef enum
{
    HSL_SUCC,
    HSL_FAIL,
    HSL_ERR_NULL,
    HSL_ERR_BUTT
}hsl_err_code_enum;
typedef unsigned char hsl_err_code_enum_uint8;

typedef enum
{
    HSL_DATA_STATUS_OK,
    HSL_DATA_STATUS_FINISH,
    HSL_DATA_STATUS_ERR,
    HSL_DATA_STATUS_BUTT
}hsl_data_status_enum;
typedef unsigned char hsl_data_status_enum_uint8;

typedef enum
{
    HSL_EMPTY,
    HSL_FILLED,
    HSL_BUTT
}hsl_fill_status_enum;

typedef enum
{
    HSL_CHANNEL_UNLOCK,
    HSL_CHANNEL_LOCK,
    HSL_CHANNEL_BUTT
}hsl_channel_status_enum;

typedef enum
{
    HSL_AUTH_TYPE_OPEN = 0,         /* OPEN */
    HSL_AUTH_TYPE_WEP = 1,          /* WEP */
    HSL_AUTH_TYPE_TKIP = 2,         /* WPA */
    HSL_AUTH_TYPE_AES = 3,          /* WPA2 */
    HSL_AUTH_TYPE_UNKNOWN = 4,      /* unknown */
    HSL_AUTH_TYPE_BUTT
}hsl_auth_type_enum;
typedef enum
{
    HSL_ONLINE_TYPE_UDP,
    HSL_ONLINE_TYPE_TCP,
    HSL_ONLINE_TYPE_BUTT
}hsl_online_type_enum;
typedef enum
{
    HSL_CRC_FAIL,
    HSL_CRC_SUCC,
    HSL_CRC_BUTT
}hsl_crc_status_enum;
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
    hsl_uint8  auc_ssid[HSL_SSID_LEN_MAX]; //AP��sssid
    hsl_uint8  auc_pwd[HSL_PWD_LEN_MAX];   //AP������
    hsl_uint8  auc_ip[IP_ADDR_LEN];        //�ֻ���IP��ַ
    hsl_uint8  uc_ssid_len;
    hsl_uint8  uc_pwd_len;
    hsl_uint16 us_port;                   //�ֻ���TCP��UDP���Ӷ˿ں�
    hsl_uint8  en_auth_mode;              //AP�ļ��ܷ�ʽ
    hsl_uint8  en_online_type;            //����֪ͨ��TCP����UDP
    hsl_uint8  auc_flag[2];               //���ӱ�ʶ
}hsl_result_stru;
typedef struct
{
    hsl_uint8   auc_data[256];      //��Ž��յ�������
    hsl_uint8   auc_fill_flag[128]; //�����Ӧλ�Ƿ����
    hsl_uint32  ul_data_len;        //��Ҫ�������ݳ���
    hsl_uint32  ul_fill_len;        //�Ѿ��������ݳ���
    hsl_uint8   uc_crc_flag;        //�����Ƿ�CRCУ��ͨ��
    hsl_uint8   auc_res[3];
}hsl_data_stru;
typedef struct
{
    hsl_uint32  ul_frame_num;       //����������Ч���ĸ���
    hsl_uint32  ul_sync_num;        //������ʼ���ĸ���
    hsl_uint32  ul_lock_time;       //���ŵ�ʱ��ϵͳʱ��
    hsl_uint8   uc_lock_flag;       //���ŵ����
    hsl_uint8   auc_res[3];
}hsl_status_stru;
typedef struct
{
   hsl_uint8  uc_index;     //MAC��ַЯ����ϢӦ������λ��
   hsl_uint8  auc_data[2];  //MAC��ַЯ���ļ�����Ϣ
   hsl_uint8  auc_res[1];
}hsl_mac_data_stru;

typedef struct
{
    hsl_uint16  bit_protocol_version    : 2,        /* Э��汾 */
                bit_type                : 2,        /* ֡���� */
                bit_sub_type            : 4,        /* ������ */
                bit_to_ds               : 1,        /* ����DS */
                bit_from_ds             : 1,        /* ����DS */
                bit_more_frag           : 1,        /* �ֶα�ʶ */
                bit_retry               : 1,        /* �ش�֡ */
                bit_power_mgmt          : 1,        /* ���ܹ��� */
                bit_more_data           : 1,        /* �������ݱ�ʶ */
                bit_protected_frame     : 1,        /* ���ܱ�ʶ */
                bit_order               : 1;        /* ����λ */
    hsl_uint16  bit_duration_value      : 15,
                bit_duration_flag       : 1;
    hsl_uint8   auc_addr1[6];
    hsl_uint8   auc_addr2[6];
    hsl_uint8   auc_addr3[6];
    hsl_uint16  bit_frag_num            : 4,
                bit_seq_num             : 12;
}hsl_frame_header_stru;
typedef struct
{
    hsl_data_stru   st_data;
    hsl_data_stru   st_key;
    hsl_status_stru st_status;
}hsl_context_stru;
/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
hsl_int32 hsl_get_ap_params (hsl_uint8* puc_manufacturer_id, hsl_uint8* puc_device_id,
                             hsl_int8* pc_device_ssid_out, hsl_uint32* ul_ssid_len_out, 
                             hsl_uint8* puc_password_out, hsl_uint32* ul_pass_len_out);

hsl_int32 hsl_get_online_packet(hsl_uint8* puc_data, hsl_uint32* pul_len);
hsl_int32 hsl_os_init(hsl_context_stru *pst_context);
hsl_int32 hsl_os_reset(void);
hsl_int32 hsl_get_result(hsl_result_stru* pst_result);
hsl_int32 hsl_parse_data(hsl_uint8* puc_data, hsl_uint32 ul_len);
hsl_int32 hsl_get_lock_status(void);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif //__HSLCONFIG_LIB_H__

