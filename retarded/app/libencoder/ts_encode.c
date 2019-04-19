
/*******************************************************************************
*@ Description    :TS文件录制
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "typeport.h"
#include "encoder.h"
#include "ts_interface.h"
#include "ts_encode.h"


 
#define  MP4_DETAILS_ALL     0xFFFFFFFF
#define  NALU_SPS  0
#define  NALU_PPS  1
#define  NALU_I    2
#define  NALU_P    3
#define  NALU_SET  4

/*
    判断nalu 包的类型
    返回：
        NALU_SPS  0
        NALU_PPS  1
        NALU_I    2
        NALU_P    3
        NALU_SET  4
        
*/
extern int nalu_type(unsigned char* naluData, int naluSize);

//用来以十六进制字节流打印box
extern void print_array(unsigned char* box_name,unsigned char*start,unsigned int length);

#define RECODE_STREAM_ID 0     //进行录像的流id       ： 0或者1
#define VIDEO_RECORD_TIME 15   //录像时长 单位：S    
#define FIND_IDR_MAX_NUM  200  //为了寻找IDR的的最大循环次数

#define AUDIO_SAMPLING_RATE (16000) //音频的原始采样率（PCM）
#define V_FRAME_RATE  (15)  //视频帧帧率
/*-----------------------------------------------------------------
1.使用 EasyAACEncoder库: 
    44.1KHZ:A_FRAME_RATE = 15 帧/秒
    8KHZ ： A_FRAME_RATE  = 8  帧/秒 
2.使用 faac库：
    44.1KHZ A_FRAME_RATE = 27 帧/秒 (数据帧较EasyAACEncoder的小很多)
    16KHZ   A_FRAME_RATE = 14 帧/秒 (数据帧较EasyAACEncoder的小很多)
注意:
    实测，在16KHZ、44.1KHZ下使用EasyAACEncoder库编码出来的音频有
    倍速效果（不推荐），但8KHZ下比较正常。
----------------------------------------------------------------*/
#define A_FRAME_RATE  (14)  //aac音频数据帧率

/*******************************************************************************
*@ Description    : 录制TS文件
*@ Input          :
*@ Output         :<out_buf>ts文件缓存buf
                    <out_len>ts文件缓存buf长度
                    <recode_time>录制时长（s）
*@ Return         :成功：0；失败：-1
*@ attention      :out_buf 需要上层free
*******************************************************************************/
int ts_record(void **out_buf,int* out_len,int recode_time)
{
    printf("\n\n******start ts_record************************************************************************\n");
    pthread_detach(pthread_self());  

    //sleep(5); //如若刚开机，不要太快开始录像
 

    /*---# init ------------------------------------------------------------*/
    ts_recoder_init_t init_info = {0};
    init_info.audio_config.ID = 0;
    init_info.audio_config.profile = 1; // low
    init_info.audio_config.sampling_frequency_index = 0x8; //16000HZ
    init_info.audio_config.sample_rate = 16000;
    init_info.audio_config.n_ch = 1;
    init_info.video_config.frame_rate = 15;//video 15帧/s
    if(TS_recoder_init(&init_info) < 0)
    {
        ERROR_LOG("fmp4 encode init failed!\n");
        goto ERR;
    }
        
    /*---#------------------------------------------------------------*/
    
    unsigned int skip_len = 0;
    unsigned int frame_len = 0;
    struct timeval start_record_time = {0};
    struct timeval tmp_time = {0};
    ENC_STREAM_PACK *pack = NULL;
    FRAME_HDR *header  = NULL;
    int  stream_id;
   
    //--request stream--------------------------------------
    stream_id = encoder_request_stream(0, RECODE_STREAM_ID, 1);
    
    while(1)  //保证第一帧是视频关键帧
    {
        printf("search to IDR frame...\n");
        pack = encoder_get_packet(stream_id);
        header = (FRAME_HDR *) pack->data;
        
        if(header->type != 0xF8)//寻找IDR帧
        {
            printf("release pack ...");
            encoder_release_packet(pack);
            continue;
        }
        else
        {
            printf("IDR frame finded*******!\n");
            break;
        }       
    }

    skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
    frame_len = pack->length - skip_len;   
    char * IDR_frame = (char*)pack->data + skip_len;
    DEBUG_LOG("IDR_frame[]= %x %x %x %x %x %x\n",IDR_frame[0],IDR_frame[1],IDR_frame[2],IDR_frame[3],IDR_frame[4],IDR_frame[5]);           
    
    //---debug 部分-------------------------------------------------------------------------
    #if 0   //保存找到的第一帧I帧数据到文件
        int tmp_fd = open("/jffs0/IDR_NALU.bin",O_RDWR|O_CREAT|O_TRUNC,0777);
        if(tmp_fd < 0)
        {
            ERROR_LOG("open failed !\n");
            goto other_error;
        }

        skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
        frame_len = pack->length - skip_len;
            
        int tmp_ret = write(tmp_fd, pack->data + skip_len ,frame_len);
        if(tmp_ret != frame_len)
        {
            ERROR_LOG("write error!\n");
            close(tmp_fd);
            goto other_error;
        }
        close(tmp_fd);
    #endif

    gettimeofday(&start_record_time,NULL);
    memcpy(&tmp_time,&start_record_time,sizeof(start_record_time));
    printf("seconds:%ld  microseconds:%ld\n",start_record_time.tv_sec,start_record_time.tv_usec);
    int cucle_num = 0;
    unsigned int A_count = 0;
    unsigned int V_count = 0;
    
    IFRAME_INFO * start_info = (IFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
    unsigned long long int start_time = start_info->pts_msec;
    unsigned long long int cur_time = start_time;
    printf("start_time = (%llu) cur_time = (%llu)\n",start_time,cur_time);
     struct timeval audio_time_start = {0};
     struct timeval video_time_start = {0};
     struct timeval audio_time_current = {0};
     struct timeval video_time_current = {0};
     gettimeofday(&audio_time_start,NULL);
     gettimeofday(&video_time_start,NULL);
   // while(tmp_time.tv_sec - start_record_time.tv_sec < VIDEO_RECORD_TIME)
   while(1)//录制的时长用视频帧的帧头时间来进行计时比较准确
   {
        //找到IDR帧，开始录制
         gettimeofday(&audio_time_current,NULL);
        if(audio_time_current.tv_sec - audio_time_start.tv_sec >=1)//1s 打印一次
        {
            printf("count Aframe = %d\n",A_count);
            memcpy(&audio_time_start,&audio_time_current,sizeof(struct timeval));
            A_count = 0;
        }
         gettimeofday(&video_time_current,NULL);
        if(video_time_current.tv_sec - video_time_start.tv_sec >=1)//1s 打印一次
        {
            printf("count Vframe = %d\n",V_count);
            memcpy(&video_time_start,&video_time_current,sizeof(struct timeval));
            V_count = 0;
        }
        
         if (header->type == 0xFA)//0xFA-音频帧
        {
            A_count ++;
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (AFRAME_INFO);
                frame_len = pack->length - skip_len; 
                AFRAME_INFO * A_info = (AFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                // printf("Audio pts : %lld\n",A_info->pts_msec);
                    
                if(TsAEncode((unsigned char*)pack->data + skip_len,frame_len/* A_FRAME_RATE,A_info->pts_msec*/))
                {
                    ERROR_LOG("TsAEncode failed !\n");
                    goto ERR;
                }
            
            #endif

        }
        else if(header->type == 0xF8)   //0xF8-视频关键帧 
        {
            V_count ++;
            // printf("I frame ========\n");
            skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
            frame_len = pack->length - skip_len;    
            IFRAME_INFO * V_info = (IFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
            cur_time = V_info->pts_msec;
            if(TsVEncode((unsigned char*)pack->data + skip_len,frame_len/*V_FRAME_RATE,V_info->pts_msec*/))
            {
                ERROR_LOG("TsVEncode failed !\n");
                goto ERR;
            }

        }
        else if(header->type == 0xF9)//0xF9-视频非关键帧
        {
            V_count ++;
            // printf("P frame\n");
            skip_len = sizeof (FRAME_HDR) + sizeof (PFRAME_INFO);
            frame_len = pack->length - skip_len;    
            PFRAME_INFO * V_info = (PFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
            cur_time = V_info->pts_msec;
            if(TsVEncode((unsigned char*)pack->data + skip_len,frame_len/*V_FRAME_RATE,V_info->pts_msec*/))
            {
                ERROR_LOG("Fmp4VEncode failed !\n");
                goto ERR;
            }
            
        }
        else
        {
            ERROR_LOG("unknown frame type!\n");
        }

        encoder_release_packet(pack);
        GET_PACK:
        pack = encoder_get_packet(stream_id);
        header = (FRAME_HDR *) pack->data;
        //printf("cur_time(%llu)  start_time(%llu)\n",cur_time,start_time);
        if(cur_time - start_time >= recode_time*1000 ) 
        {
            if(pack)  encoder_release_packet(pack);
            break; //录制完成
        }
            
    }

    DEBUG_LOG("End of recording time!\n");

    struct timeval time_start = {0};
    struct timeval time_end = {0};
    gettimeofday(&time_start,NULL);
    
    if(TS_remux_video_audio(out_buf,out_len) < 0)
    {
        ERROR_LOG("TS_remux_video_audio failed!\n");
        goto ERR;
    }
    gettimeofday(&time_end,NULL);
    printf("TS_remux_video_audio use time = %d(s) %d(us) \n",time_end.tv_sec - time_start.tv_sec,time_end.tv_usec - time_start.tv_usec);

    
    encoder_free_stream(stream_id);
    TS_recoder_exit();
    printf("\n******END ts_record************************************************************************\n\n");
    return 0;
    
ERR:
    encoder_free_stream(stream_id);
    TS_recoder_exit();
    printf("\n******ERR fmp4_record !!!************************************************************************\n\n");
    return -1;
}






