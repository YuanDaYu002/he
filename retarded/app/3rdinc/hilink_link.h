/*******************************************************************************
 *               Copyright (C) 2015, Huawei Tech. Co., Ltd.
 *                      ALL RIGHTS RESERVED
 *******************************************************************************/

/** @defgroup hilink.h

 *  @author  huawei
 *  @version 1.0
 *  @date    2015/11/26 \n

 *  history:\n
 *          1. 2015/11/26, huawei, create this file\n\n
 *
 *  ���ļ�����ϸ����...
 *
 * @{
 */

/* ע: mainpage���ڶ���ļ�����һ��chmʱ���ⲿ���ǿ�ѡ�ģ������������ϸ��������д�������� */
/*! \mainpage
 *
 *  ģ��˵��:\n

 *  \section intro_sec ��ģ���ṩ�Ľӿ�:
 *  - ::
 */

#ifndef __HILINK_LINK_H__
#define __HILINK_LINK_H__

/*******************************************************************************
 *   ���ļ���Ҫ����������ͷ�ļ�
*******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /*  __cpluscplus */
#endif /*  __cpluscplus */

#ifndef CFG_XIP_ENABLE
#define CFG_XIP_ENABLE 0
#endif

#if (CFG_XIP_ENABLE)
#define  XIP_ATTRIBUTE(x)    __attribute__ ((section(x)))
#else
#define  XIP_ATTRIBUTE(x)
#endif

#define  XIP_ATTRIBUTE_VAR    XIP_ATTRIBUTE (".xipsec1")
#define  XIP_ATTRIBUTE_FUN    XIP_ATTRIBUTE (".xipsec0")

/*******************************************************************************
 *   ö�����Ͷ�����
*******************************************************************************/
typedef enum tagHILINK_E_ERR_CODE
{
    HILINK_E_ERR_TOP = 100,
    HILINK_E_ERR_PARAM,           /**< invalid param */
    HILINK_E_ERR_MEM,             /**< mem operation error */
    HILINK_E_ERR_FRAME,           /**< invalid frame */
    HILINK_E_ERR_PARSE_ARRAY,     /**< parse array error*/
    HILINK_E_ERR_DERCYPT,         /**< decrypt error*/
    HILINK_E_ERR_DERCYPT_W,       /**< decrypt error*/
    HILINK_E_ERR_PARSE_CAP,       /**< parse cap error*/
    HILINK_E_ERR_PARSE_MIX,       /**< parse app ip port token error*/
    HILINK_E_ERR_PARSE_SSID,      /**< parse ssid error*/
    HILINK_E_ERR_PARSE_PW,        /**< parse password error*/
    HILINK_E_ERR_PARSE_DEVICESN,  /**< parse device sn error*/
    HILINK_E_ERR_PACK,            /**< pack buffer for online error*/
    HILINK_E_ERR_BUTT
}HILINK_E_ERR_CODE;

/*
 * hilink_link_parse()��������µķ���ֵ
 */
typedef enum tagHILINK_WIFI_STATUS
{
    HI_WIFI_STATUS_RECEIVING = 0,        /* �հ����� */
    HI_WIFI_STATUS_CHANNEL_LOCKED = 1,   /* ����wifi�ŵ� */
    HI_WIFI_STATUS_FINISH = 2,           /* �հ���� */
    HI_WIFI_STATUS_CHANNEL_UNLOCKED = 3, /* �ͷ�wifi�ŵ� */
}HILINK_WIFI_STATUS;

typedef enum tagHILINK_WIFI_ENCTYPE
{
    HI_WIFI_ENC_OPEN = 0,         /* OPEN */
    HI_WIFI_ENC_WEP = 1,          /* WEP */
    HI_WIFI_ENC_TKIP = 2,         /* WPA */
    HI_WIFI_ENC_AES = 3,          /* WPA2 */
    HI_WIFI_ENC_UNKNOWN = 4,      /* unknown */
    HI_WIFI_ENC_BUTT
}HILINK_WIFI_ENCTYPE;

/*******************************************************************************
 *   ���ݽṹ���ͺ����������Ͷ���
*******************************************************************************/
/*
 * hilink_link_init()��Ҫ����˲�����ȫ��ʹ��
 */
typedef struct
{
    unsigned char  chaos[1024];
} hilink_s_context;

/*
 * hilink_link_get_result()��Ҫ����˲�����ȡ���
 */
typedef struct
{
    unsigned char   SendIP[4];              /** SendIP */
    unsigned char   ssid[64];               /** wifi ssid */
    unsigned char   pwd[128];               /** wifi password */
    unsigned char   ssid_len;               /** wifi ssid length */
    unsigned char   pwd_len;                /** wifi pwd length */
    unsigned char   enc_type;               /** wifi enc type */
    unsigned char   sendtype;               /** send back by UDP1 TCP2  */
    unsigned short  SendPort;               /** SendPort */
    unsigned char   reserved[2];            /** reserved  */
} hilink_s_result;

/*
 * hilink_link_get_result()��Ҫ����˲���
 */
typedef struct
{
    unsigned char   len_open;
    unsigned char   len_wep;
    unsigned char   len_tkip;
    unsigned char   len_aes;
} hilink_s_pkt0len;

/*******************************************************************************
 *   ȫ�ֺ�������(extern)��
*******************************************************************************/
/**
* Begin ��Ҫ�ⲿ����ĺ���
*
* void *hilink_memset(void *s, int c, unsigned int n);
* void *hilink_memcpy(void *dest, const void *src, unsigned int n);
* int   hilink_memcmp(const void *s1, const void *s2, unsigned int n);
* int   hilink_printf(const char* format, ...);
* int   hilink_gettime(unsigned long * ms);
* ���豸���亯����Ac���ȹ̶�48�ֽڣ�AC�ļ����豸��HiLink��֤ʱ�ɻ�Ϊ��֤ʱ�ַ�
* int   hilink_sec_get_Ac(unsigned char* pAc, unsigned int ulLen)
* End �ⲿ���亯��
*/

/**
* ����wifi��ͬ�������͵�QOS Data��0��׼����
*  @param[in]  pst_pkt0len refer to struct hilink_s_pkt0len
*
*  @retval :: 0 success
*/
int hilink_link_set_pkt0len(hilink_s_pkt0len* pst_pkt0len) XIP_ATTRIBUTE_FUN;

/**
* ��ȡ�汾��
*/
const char* hilink_link_get_version(void) XIP_ATTRIBUTE_FUN;

/**
* ��ʼ��link��,hilink_s_context���ⲿʹ�������봫��
*  @param[in]  pcontext refer to struct hilink_s_context
*
*  @retval :: 0 success
*/
int hilink_link_init(hilink_s_context* pcontext) XIP_ATTRIBUTE_FUN;

/**
* �л��ŵ��Ժ󣬵��ñ��ӿ��建��
*/
int hilink_link_reset(void) XIP_ATTRIBUTE_FUN;

/** ���ŵ��Ƿ�ready״̬
*
*  @param[in]  none
*
*  @retval :: 0:�����л��ŵ�,��0�����л��ŵ�
*/
int hilink_link_get_lock_ready(void) XIP_ATTRIBUTE_FUN;


/** �����鲥����
*
*  @param[in]  frame    802.11֡
*  @param[in]  len      ���ĳ���
*
*  @retval :: refer to enum HILINK_WIFI_STATUS
*/
int hilink_link_parse(const void* frame, unsigned int len) XIP_ATTRIBUTE_FUN;

/** ��ȡ�鲥���Ľ��,��hilink_link_parse ���� HI_WIFI_STATUS_COMPLETE����
*
*  @in ackey ��Ȩ�� ��huawei����,�Ƿ�key�����ܻ�ý��
*  @in ackeylen ����Ϊ48�ֽ�
*  @param[out] pst_result  �����ɹ���Ľ��
*
*  @retval :: 0 success
*/
int hilink_link_get_result(hilink_s_result* pst_result) XIP_ATTRIBUTE_FUN;

/** ����ѡ�ӿڡ�����豸����AP���ܣ���ͨ���ýӿڻ�ȡ��AP�����ƣ������ֻ���ͨ�������ַ����豸��
    ����link�������Ϣ,����device��ssid
*
*  @param[in]   ssid_type           ssid���ͣ����ȱ���Ϊ2�ֽڣ����豸״̬�й�
*  @param[in]   device_id           HiLink�豸��֤�ţ�4�ֽ�
*  @param[in]   device_sn           �豸SN�ţ�SN�ĺ�1λ,1�ֽ�
*  @param[in]   sec_type            �豸֧�ֵļ���ģʽ,1�ֽ�
*  @param[in]   sec_key             key�ַ���22�ֽ�
*  @param[out]  device_ssid_out     SSID���֣������豸�Լ�����APʱ���ý�ȥ
*  @param[out]  ssid_len_out        SSID���ȣ��32�ֽ�
*
*/
int hilink_link_get_devicessid(const char* ssid_type,
                               const char* device_id,
                               const char* device_sn,
                               const char* sec_type,
                               const char* sec_key,
                               char* device_ssid_out, unsigned int* ssid_len_out) XIP_ATTRIBUTE_FUN;

/** �豸����WiFi����Ҫ֪ͨ�ֻ��豸���ߡ�ͨ�����ӿ���װ���͵�֪ͨ����
*
*  @param[out]  buffer              ������װ���bufer������socket����  ����128�ֽ�
*  @param[out]  buffer_len          buffer����
*
*/
int hilink_link_get_notifypacket(char* buffer_out, unsigned int* len_out) XIP_ATTRIBUTE_FUN;

#ifdef __cplusplus
#if __cplusplus
}
#endif /*  __cpluscplus */
#endif /*  __cpluscplus */


#endif  /* __HILINK_LINK_H__ */

/** @}*/
