 
/***************************************************************************
* @file: system_upgrade.h
* @author:   
* @date:  5,5,2019
* @brief:  系统升级相关函数头文件
* @attention:
***************************************************************************/
#ifndef SYSTEM_UPGRADE_H
#define SYSTEM_UPGRADE_H

//升级状态
typedef enum _upgrade_status_e
{
	UP_SELECT_REGION_FAILED = -5,  //选择升级分区失败
	UP_REGION_OVERFLOW = -2,	   //区域溢出（升级文件太大，系统分区放不下）
	UP_ILLEGAL_PARAMETER = -1,	   //参数非法 
	UP_OK = 0
}upgrade_status_e;

upgrade_status_e upgrade_write_norflash(void *buf,unsigned int buf_len);



#endif


