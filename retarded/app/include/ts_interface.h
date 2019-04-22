 
/***************************************************************************
* @file:ts_interface.h 
* @author:   
* @date:  4,16,2019
* @brief:  
* @attention:
***************************************************************************/
#ifndef _TS_INTERFACE_H
#define _TS_INTERFACE_H


/*---#-对外开放的 audio/video 部分参数配置-----------------------------------------------------------*/
 typedef struct _ts_audio_init_t
 {
	 long long int		 ID;						 //MPEG 标示符。0表示MPEG-4，1表示MPEG-2
	 long long int		 profile;					 //标识使用哪个级别的AAC(值再减1)。1: AAC Main 2:AAC LC (Low Complexity) 3:AAC SSR (Scalable Sample Rate) 4:AAC LTP (Long Term Prediction) 
	 long long int		 sampling_frequency_index;	 //标识使用的采样率的下标（0x8			   : 16000 /0x5 ：32000  ）
	 unsigned  int		 sample_rate;				 //采样率 
	 unsigned  int		 n_ch;						 //声道数 
 }ts_audio_init_t;

 typedef struct _ts_video_init_t
 {
 	int 		frame_rate;		//帧率
 	
 }ts_video_init_t;


 typedef struct _ts_recoder_init_t
 {
	ts_video_init_t	 video_config;//输入的video配置信息
	ts_audio_init_t	 audio_config;//输入的audio配置信息				 
 }ts_recoder_init_t;


 int TS_recoder_init(ts_recoder_init_t *config);
/*---# 循环放入帧数据---------------------------------------------*/
 int TsAEncode(void*frame,int frame_len);
 int TsVEncode(void*frame,int frame_len);
/*---#------------------------------------------------------------*/
 int  TS_remux_video_audio(void **out_buf,int* out_len);
 void TS_recoder_exit(void);

 #endif

