/******************************************************************************
  Copyright (C), 2004-2050, Hisilicon Tech. Co., Ltd.
******************************************************************************
  File Name     : keepalive_demo.c
  Version       : Initial Draft
  Author        : Hisilicon WiFi software group
  Created       : 2016-6-16
  Last Modified :
  Description   : tcp keepalive sample code
  Function List :
  History       :
  1.Date        : 2016-6-16
  Author        :
  Modification  : Created file
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "driver_hisi_lib_api.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "los_task.h"
#include "driver_hisi_lib_api.h"
#include "unistd.h"
#include "stdlib.h"
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define INTERVAL_TIMER        60000   //60s�������������
#define RETRY_INTERVAL_TIMER  3000    //3s������ش�����
#define RETRY_TIMES           5       //������ش�����
#define KEEPALIVE_TCP_NUM     4       //����TCP��·������
#define KEEPALIVE_BUF         "sleep" //���������
/*****************************************************************************
  3 ȫ�ֱ�������
*****************************************************************************/
int           al_sockfd[KEEPALIVE_TCP_NUM] = {0}; //��¼tcp���ӵ�socket������
unsigned char guc_index = 0;                      //��¼�Ѵ���TCP��·����
unsigned char keepalive_switch;                   //�����
/*****************************************************************************
  4 ��������
*****************************************************************************/
int keepalive_demo_send_tcp_params(unsigned char uc_index);
/*****************************************************************************
  5 ����ʵ��
*****************************************************************************/

/*****************************************************************************
 �� �� ��  : keepalive_demo_build
 ��������  : ����TCP����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��7��23��
    ��    ��   : 
    �޸�����   : �����ɺ���

*****************************************************************************/
int keepalive_demo_build(unsigned char *puc_ip, unsigned short us_port)
{
    struct sockaddr_in serveraddr;
    struct sockaddr_in clientaddr;
    unsigned char      uc_index;
    char               buf[] = "tcp_keepalive";

    HISI_PRINT_INFO("ip:%s",puc_ip);
    HISI_PRINT_INFO("us_port:%d",us_port);
    uc_index = guc_index;
    guc_index++;
    al_sockfd[uc_index] = socket(PF_INET, SOCK_STREAM, 0);
    if (0 > al_sockfd[uc_index])
    {
        HISI_PRINT_ERROR("%s[%d]:create [%d]tcp socket fail",__func__,__LINE__,uc_index);
        return -HISI_EFAIL;
    }
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family      = AF_INET;
    serveraddr.sin_port        = htons(us_port);//TCP�������˵Ķ˿ں�
    serveraddr.sin_addr.s_addr = inet_addr(puc_ip);//��������IP��ַ
    if (0 != connect(al_sockfd[uc_index], (struct sockaddr *)&serveraddr, sizeof(serveraddr)))
    {
        HISI_PRINT_ERROR("%s[%d]:tcp client[%d] connect sever fail",__func__,__LINE__,uc_index);
        close(al_sockfd[uc_index]);
        return -HISI_EFAIL;
    }
    while(!keepalive_switch)
    {
        send(al_sockfd[uc_index], buf, strlen(buf), 0);
        sleep(2);
    }
    return HISI_SUCC;
}

/*****************************************************************************
 �� �� ��  : keepalive_demo_send_tcp_params
 ��������  : �·�TCP�������
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��7��23��
    ��    ��   : 
    �޸�����   : �����ɺ���

*****************************************************************************/
int keepalive_demo_send_tcp_params(unsigned char uc_index)
{
    struct tcpip_conn    st_sockt_info;
    hisi_tcp_params_stru st_tcp_params;
    unsigned int         ul_ret;
    int                  l_ret;
    char                *pc_buf;
    unsigned char       *puc_src_ip;
    unsigned char       *puc_dst_ip;

    l_ret = lwip_get_conn_info(al_sockfd[uc_index], &st_sockt_info);
    if (0 > l_ret)
    {
        HISI_PRINT_ERROR("%s[%d]:get sockt info fail",__func__,__LINE__);
        return -HISI_EFAIL;
    }
    /* ׼������ */
    memcpy(st_tcp_params.auc_dst_mac,&st_sockt_info.dst_mac,6);
    st_tcp_params.ul_sess_id  = uc_index + 1;
    st_tcp_params.us_src_port = st_sockt_info.srcport;
    st_tcp_params.us_dst_port = st_sockt_info.dstport;
    st_tcp_params.ul_seq_num  = st_sockt_info.seqnum + 1;
    st_tcp_params.ul_ack_num  = st_sockt_info.acknum;
    st_tcp_params.us_window   = st_sockt_info.tcpwin;
    st_tcp_params.ul_interval_timer       = INTERVAL_TIMER;  //60�뷢һ��
    st_tcp_params.ul_retry_interval_timer = RETRY_INTERVAL_TIMER;
    st_tcp_params.us_retry_max_count      = RETRY_TIMES;
    pc_buf = (char*)malloc(sizeof(char) * sizeof(KEEPALIVE_BUF));
    memset(pc_buf, 0, sizeof(char) * sizeof(KEEPALIVE_BUF));
    memcpy(pc_buf, KEEPALIVE_BUF, sizeof(char) * sizeof(KEEPALIVE_BUF));
    st_tcp_params.puc_tcp_payload         = pc_buf;
    st_tcp_params.ul_payload_len          = sizeof(KEEPALIVE_BUF);
    puc_src_ip = st_tcp_params.auc_src_ip;
    puc_dst_ip = st_tcp_params.auc_dst_ip;
    puc_src_ip[0] = ip4_addr1((unsigned char*)&st_sockt_info.src_ip);
    puc_src_ip[1] = ip4_addr2((unsigned char*)&st_sockt_info.src_ip);
    puc_src_ip[2] = ip4_addr3((unsigned char*)&st_sockt_info.src_ip);
    puc_src_ip[3] = ip4_addr4((unsigned char*)&st_sockt_info.src_ip);
    puc_dst_ip[0] = ip4_addr1((unsigned char*)&st_sockt_info.dst_ip);
    puc_dst_ip[1] = ip4_addr2((unsigned char*)&st_sockt_info.dst_ip);
    puc_dst_ip[2] = ip4_addr3((unsigned char*)&st_sockt_info.dst_ip);
    puc_dst_ip[3] = ip4_addr4((unsigned char*)&st_sockt_info.dst_ip);
    printf("src_ip:%d.%d.%d.%d\n",puc_src_ip[0],puc_src_ip[1],puc_src_ip[2],puc_src_ip[3]);
    printf("dst_ip:%d.%d.%d.%d\n",puc_dst_ip[0],puc_dst_ip[1],puc_dst_ip[2],puc_dst_ip[3]);
    printf("src_port:%d\n",st_tcp_params.us_src_port);
    printf("dst_port:%d\n",st_tcp_params.us_dst_port);
    printf("ul_seq_num:%d\n",st_tcp_params.ul_seq_num);
    printf("ul_ack_num:%d\n",st_tcp_params.ul_ack_num);
    ul_ret = hisi_wlan_set_tcp_params(&st_tcp_params);
    if (0 != ul_ret)
    {
        HISI_PRINT_ERROR("%s[%d]:set tcp params fail",__func__,__LINE__);
        return -HISI_EFAIL;
    }
    return HISI_SUCC;
}
/*****************************************************************************
 �� �� ��  : keepalive_demo_get_lose_tcp_id
 ��������  : TCP����������Ѻ��ѯ����TCP id
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��7��23��
    ��    ��   : 
    �޸�����   : �����ɺ���

*****************************************************************************/
int keepalive_demo_get_lose_tcp_id(void)
{
    int l_ret;
    l_ret = hisi_wlan_get_lose_tcpid();
    printf("lose tcp id:%d\n",l_ret);
    return HISI_SUCC;
}
/*****************************************************************************
 �� �� ��  : keepalive_demo_set_switch
 ��������  : ����keepalive����
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��7��23��
    ��    ��   : 
    �޸�����   : �����ɺ���

*****************************************************************************/
int keepalive_demo_set_switch(int argc, char *argv[])
{
   unsigned char  uc_index;
   unsigned int   ul_ret;
   unsigned int   ul_keepalive_num;
   if ((0 == argc) || (2 < argc))
   {
       HISI_PRINT_ERROR("%s[%d]:argc=%d is error",__func__,__LINE__,argc);
       return -HISI_EFAIL;
   }
   keepalive_switch = atoi(argv[0]);
   if(0 == keepalive_switch)
   {
       guc_index = 0;
       ul_ret = hisi_wlan_set_keepalive_switch(HISI_KEEPALIVE_OFF, 0);
       if (HISI_SUCC != ul_ret)
       {
           HISI_PRINT_ERROR("%s[%d]:set keepalive switch fail[%d]",__func__,__LINE__,ul_ret);
           return -HISI_EFAIL;
       }
   }
   else
   {
       for(uc_index = 0; uc_index < guc_index; uc_index++)
       {
           keepalive_demo_send_tcp_params(uc_index);
       }
       ul_keepalive_num = atoi(argv[1]);
       hisi_wlan_set_keepalive_switch(HISI_KEEPALIVE_ON, ul_keepalive_num);
   }
   HISI_PRINT_INFO("%s[%d]:keepalive_switch=%d",__func__,__LINE__,keepalive_switch);
   return HISI_SUCC;
}
char  ac_ip[20] = {0}; //����IP��ַ
/*****************************************************************************
 �� �� ��  : keepalive_demo_main
 ��������  : ����TCP��·
 �������  :
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��7��23��
    ��    ��   : 
    �޸�����   : �����ɺ���

*****************************************************************************/
int keepalive_demo_main(int argc, char *argv[])
{
    TSK_INIT_PARAM_S    tcpTask;
    unsigned int        tcptaskid;
    unsigned int        ul_ret;

    if (4 <= guc_index)
    {
       HISI_PRINT_ERROR("%s[%d]:tcp is out of 4",__func__,__LINE__);
       return -HISI_EFAIL;
    }

    /* �����Զ˵�IP��ַ */
    memset(ac_ip, 0, sizeof(char) * 20);
    if (strlen(argv[0]) >= sizeof(ac_ip))
    {
        printf("input length fail\n");
        return -HISI_EFAIL;
    }
    strcpy(ac_ip, argv[0]);
    memset(&tcpTask, 0, sizeof(TSK_INIT_PARAM_S));
    tcpTask.pfnTaskEntry = (TSK_ENTRY_FUNC)keepalive_demo_build;
    tcpTask.uwStackSize  = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    tcpTask.pcName = "tcp_keepalive";
    tcpTask.usTaskPrio = 10;
    tcpTask.auwArgs[0] = (unsigned int)ac_ip;
    tcpTask.auwArgs[1] = atoi(argv[1]);
    tcpTask.uwResved   = LOS_TASK_STATUS_DETACHED;
    ul_ret = LOS_TaskCreate(&tcptaskid, &tcpTask);
    if (0 != ul_ret)
    {
        printf("create task fail\n");
        return -HISI_EFAIL;
    }
    return HISI_SUCC;
}
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
