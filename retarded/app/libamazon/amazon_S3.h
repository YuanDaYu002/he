 
/***************************************************************************
* @file: amazon_S3.h
* @author:   
* @date:  3,27,2019
* @brief:  
* @attention:
***************************************************************************/
#ifndef _AMAZON_S3_H_
#define _AMAZON_S3_H_

#define MP4_FILE_PATH 	"/jffs0/"   //MP4文件的缓冲绝对路径
#define M3U8_FILE_PATH 	"/jffs0/"   //m3u8文件的缓冲绝对路径
#define TS_FILE_PATH 	"/jffs0/"   //TS文件的缓冲绝对路径
#define JPG_FILE_PATH 	"/jffs0/"   //JPEG文件的缓冲绝对路径




typedef struct
{
	int 	today_req;					//0/1 当天是否已经更新 “图片视频上传接口” 信息
	int 	record_en;					//0:上传缩略图,6s视频,长视频 1:只上传缩略图,6s视频 
	char 	date[12];  					//日期  
	char 	aws_access_keyid[24];		//公匙
	char 	secret_aceess_keyid[44];	//私匙
	char 	host[128];					//上传域名
	char 	record_name[128];			//TS文件名（或者fmp4文件名，目前(6s预览/告警视频)文件都会被转换成TS文件推送）   
	char 	jpg_filename[128];			//JPG文件名（app端展示为缩略图）
	char 	deviceid[64];				//设备 id
	char 	bucket[128]; 				//文件存储桶
	char 	regon[32];					//区域（国家）
}AMAZON_INFO;

/*---推送文件类型定义-----------------------------*/
typedef enum _file_type_e
{
	TYPE_JPG = 1,		//JPEG图片
	TYPE_FMP4,			//fmp4文件
	TYPE_TS,			//TS文件
	TYPE_M3U			//m3u8文件   
}file_type_e;


typedef enum _ts_flag_e
{
	TS_FLAG_JPG = -1, 	//jpg直接上传
	TS_FLAG_6S	= 1,	//6s预览视频的ts文件（目前切片成一个TS片,相当于 TS_FLAG_ONE）	
	TS_FLAG_START,		//第一个ts文件
	TS_FLAG_MID	,		//中间部分ts文件
	TS_FLAG_END	,		//最后一个ts文件
	TS_FLAG_ONE			//只有一个ts文件
	
}ts_flag_e;

/*---# amazon 云推送文件之信息结构----------*/
typedef struct _put_file_info_t
{
	int 		file_tlen; 					//文件的时长	
	file_type_e file_type; 					//文件类型： 6s预录TS/jpeg/告警视频TS 
	ts_flag_e 	ts_flag;					//ts文件的flag标志（详见 ts_flag_e 枚举变量）
	char		file_name[62];				//文件名
	char 		m3u8name[64];				//对应的m3u8文件名(内容由S3程序自动生成)
	char 		datetime[24];				//时间信息
	
}put_file_info_t;


/*******************************************************************************
*@ Description    :  负责更新 g_amazon_info （设备图片视频上传接口）的信息
*@ Input          :
*@ Output         :
*@ Return         :
*******************************************************************************/
void amazon_S3_req_thread(void);

/*******************************************************************************
*@ Description    :  设备图片视频上传线程(上传结束会自动退出)
*@ Input          : <put_file> 需要上传的文件（JPEG/video6s/alarm_video）描述信息
*@ Output         :
*@ Return         :
*******************************************************************************/
void amazon_put_even_thread(put_file_info_t *put_file);


int is_amazon_info_update(void);


#endif




