



#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
//#include <sys/reboot.h>
#include <sys/time.h>

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "hi_comm_video.h"

#include "typeport.h"
#include "hal.h"
#include "encoder.h"
//#include "misc.h"
#include "watchdog.h"
#include "hal_def.h"
#include "video.h"
//#include "record.h"
#include "sdp.h"
#include "audio.h"
#include "motion_detect.h"
#include "media_server_interface.h"
#include "hi_comm_aio.h"
#include "fmp4_encode.h"
#include "itfEncoder.h"
#include "amazon_S3.h"
#include "hls_main.h"
#include "fmp4_interface.h"
#include "ts_encode.h"






#if 0

const char * hal_get_project_name(void)
{
    return HAL_PROJECT_NAME;
}
#endif

int hal_get_max_preview_count(void)
{
    return sdp_max_prev_count();
}

static void mpp_sys_exit(void)
{
    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();
}

static int calc_pic_vbblk_size(int width, int height, int align)
{
    if (16 != align && 32 != align && 64 != align) {
        return -1;
    }


    return (CEILING_2_POWER(width, align) * CEILING_2_POWER(height, align) * 3 / 2);
}

static int mpp_sys_init(void)
{
#define SYS_ALIGN_WIDTH  16

    HLE_S32 ret;

    mpp_sys_exit();

    VB_CONF_S vb_conf; /* vb config define */
    memset(&vb_conf, 0, sizeof (VB_CONF_S));
    vb_conf.u32MaxPoolCnt = VB_MAX_POOLS;
    vb_conf.astCommPool[0].u32BlkSize = calc_pic_vbblk_size(1920, 1088, SYS_ALIGN_WIDTH);
    vb_conf.astCommPool[0].u32BlkCnt = 6;
    //vb_conf.astCommPool[1].u32BlkSize = calc_pic_vbblk_size(960, 544, SYS_ALIGN_WIDTH);
    //vb_conf.astCommPool[1].u32BlkCnt = 6;
    vb_conf.astCommPool[1].u32BlkSize = calc_pic_vbblk_size(480, 272, SYS_ALIGN_WIDTH);
    vb_conf.astCommPool[1].u32BlkCnt = 6;
    ret = HI_MPI_VB_SetConf(&vb_conf);
    if (HLE_RET_OK != ret) {
        ERROR_LOG("HI_MPI_VB_SetConf fail: %#x.\n", ret);
        return HLE_RET_ERROR;
    }

    ret = HI_MPI_VB_Init();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("HI_MPI_VB_Init fail: %#x.\n", ret);
        return HLE_RET_ERROR;
    }

    MPP_SYS_CONF_S sys_conf;
    sys_conf.u32AlignWidth = SYS_ALIGN_WIDTH;
    ret = HI_MPI_SYS_SetConf(&sys_conf);
    if (HLE_RET_OK != ret) {
        ERROR_LOG("HI_MPI_SYS_SetConf fail: %#x.\n", ret);
        return HLE_RET_ERROR;
    }

    ret = HI_MPI_SYS_Init();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("HI_MPI_SYS_Init fail: %#x.\n", ret);
        return HLE_RET_ERROR;
    }

    DEBUG_LOG("success\n");
    return HLE_RET_OK;
}

static int max_frame_rate;

int hal_set_video_std(VIDEO_STD_E video_std)
{
    if (!((video_std == VIDEO_STD_NTSC) || (video_std == VIDEO_STD_PAL))) {
        ERROR_LOG("invalid para!\n");
        return HLE_RET_EINVAL;
    }

    max_frame_rate =
            (video_std == VIDEO_STD_NTSC) ? NTSC_MAX_FRAME_RATE : PAL_MAX_FRAME_RATE;

    return HLE_RET_OK;
}

int hal_set_capture_size(int size)
{
    return HLE_RET_ENOTSUPPORTED;
}

int hal_get_max_frame_rate(void)
{
    return max_frame_rate;
}

int rtc_init(void);
int hal_video_init(void);
int hal_encoder_init(HLE_S32 pack_count);
int hal_encoder_exit(void);
int hal_video_exit(void);
int rtc_exit(void);

int hal_init(HLE_S32 pack_count, VIDEO_STD_E video_std)
{

    if (HLE_RET_OK != hal_set_video_std(video_std))
        goto mode_set_failed;

    if (HLE_RET_OK != rtc_init())
        ERROR_LOG("RTC init failed\n");

    if (HLE_RET_OK != mpp_sys_init())
        goto mpp_init_failed;

    if (HLE_RET_OK != hal_video_init())
        goto video_init_failed;

    if (HLE_RET_OK != hal_audio_init())
        goto audio_init_failed;

    if (HLE_RET_OK != hal_encoder_init(pack_count))
        goto encoder_init_failed;

    if (HLE_RET_OK != motion_detect_init())
        goto motion_init_failed;

    //if (HLE_RET_OK != blind_detect_init())
    //  goto blind_init_failed;

    //if (HLE_RET_OK != misc_init())
    //    ERROR_LOG("misc init failed\n");

    DEBUG_LOG("success\n");
    return HLE_RET_OK;

    //blind_init_failed:
    //    blind_detect_exit();
motion_init_failed:
    motion_detect_exit();
audio_init_failed:
    hal_audio_exit();
encoder_init_failed:
    hal_encoder_exit();
video_init_failed:
    hal_video_exit();
mpp_init_failed :
    mpp_sys_exit();
mode_set_failed:
    return HLE_RET_ERROR;
}

void hal_exit(void)
{
    //misc_exit();
    //blind_detect_exit();
    motion_detect_exit();
    hal_encoder_exit();
    hal_audio_exit();
    hal_video_exit();
    mpp_sys_exit();
    rtc_exit();
}

int hal_get_time(HLE_SYS_TIME *sys_time, int utc, HLE_U8* wday)
{
    time_t time_cur;
    time(&time_cur);
    struct tm cur_sys_time;
    if (utc) {
        gmtime_r(&time_cur, &cur_sys_time);
    }
    else {
        localtime_r(&time_cur, &cur_sys_time);
    }

    sys_time->tm_year = cur_sys_time.tm_year + 1900;
    sys_time->tm_mon = cur_sys_time.tm_mon + 1;
    sys_time->tm_mday = cur_sys_time.tm_mday;
    sys_time->tm_msec = ((cur_sys_time.tm_hour) * 3600 +
            (cur_sys_time.tm_min) * 60 + cur_sys_time.tm_sec)*1000;
    *wday = cur_sys_time.tm_wday;

    return HLE_RET_OK;
}

int hal_set_time(HLE_SYS_TIME *sys_time, int utc)
{
    int get_rtc_time(struct tm *tm_time);
    int set_rtc_time(struct tm *tm_time);
    
    if (sys_time->tm_year < 1970 || sys_time->tm_year > 2037
        || sys_time->tm_mon < 1 || sys_time->tm_mon > 12
        || sys_time->tm_mday < 1 || sys_time->tm_mday > 31
        || sys_time->tm_msec > 60 * 60 * 24 * 1000)
        return HLE_RET_EINVAL;

    struct tm tm_time = {0};
    tm_time.tm_year = sys_time->tm_year - 1900;
    tm_time.tm_mon = sys_time->tm_mon - 1;
    tm_time.tm_mday = sys_time->tm_mday;
    tm_time.tm_hour = (sys_time->tm_msec / 1000) / 3600;
    tm_time.tm_min = ((sys_time->tm_msec / 1000) % 3600) / 60;
    tm_time.tm_sec = (sys_time->tm_msec / 1000) % 60;

    if (utc) {
        // set rtc clock
        struct tm rtc;
        int ret = get_rtc_time(&rtc);
        if ((HLE_RET_OK != ret) || (mktime(&tm_time) != mktime(&rtc))) {
            set_rtc_time(&tm_time);
            DEBUG_LOG("Set RTC time : %04d-%02d-%02d %02d:%02d:%02d UTC\n",
                      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        }

        //设置系统时间
        struct timeval tv = {0};
        char* oldtz = getenv("TZ");
        //setenv("TZ", "UTC0", 1);
        tv.tv_sec = mktime(&tm_time);
        if (oldtz) {
            //setenv("TZ", oldtz, 1);
        }
        else {
            //unsetenv("TZ");
        }
        time_t curr_time = time(NULL);
        if (tv.tv_sec != curr_time) {
            //只有当需要设置的时间和当前时间之间有1秒以上的偏差才做真正的校时动作
            settimeofday(&tv, NULL);
            DEBUG_LOG("Set Sys time :%s", ctime(&tv.tv_sec));
        }
    }
    else {
        // set rtc clock
        struct timeval tv = {0};
        tv.tv_sec = mktime(&tm_time);
        gmtime_r(&tv.tv_sec, &tm_time);
        struct tm rtc;
        int ret = get_rtc_time(&rtc);
        if ((HLE_RET_OK != ret) || (mktime(&tm_time) != mktime(&rtc))) {
            set_rtc_time(&tm_time);
            DEBUG_LOG("Set RTC time : %04d-%02d-%02d %02d:%02d:%02d UTC\n",
                      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        }

        //设置系统时间
        time_t curr_time = time(NULL);
        if (tv.tv_sec != curr_time) {
            //只有当需要设置的时间和当前时间之间有1秒以上的偏差才做真正的校时动作
            settimeofday(&tv, NULL);
            DEBUG_LOG("Set Sys time :%s", ctime(&tv.tv_sec));
        }
    }

    return HLE_RET_OK;
}

void hal_set_date_format(E_DATE_DISPLAY_FMT date_fmt)
{
    void encoder_set_date_format(E_DATE_DISPLAY_FMT date_fmt);
    
    if ((date_fmt != DDF_YYYYMMDD) && (date_fmt != DDF_MMDDYYYY) &&
        (date_fmt != DDF_DDMMYYYY)) {
        ERROR_LOG("date_string_format unsupport\n");
        return;
    }
    encoder_set_date_format(date_fmt);
}

int hal_get_defaultkey_status(void)
{
    //return detect_defaultkey_status();
    return 0;
}

#if 0
#define EEPROM_PAGE_FOR_SN     0
#define EEPROM_PAGE_FOR_MAC    1
#define EEPROM_PAGE_FOR_KEY    2

int hal_get_unique_id(unsigned char *unique_id, unsigned int *unique_id_len)
{
    return get_encrypt_chip_id(unique_id, unique_id_len);
}

int hal_write_serial_no(unsigned char *sn_data, unsigned int sn_len)
{
    return eeprom_write_page(EEPROM_PAGE_FOR_SN, sn_data, sn_len);
}

int hal_read_serial_no(unsigned char *sn_data, unsigned int sn_len)
{
    return eeprom_read_page(EEPROM_PAGE_FOR_SN, sn_data, sn_len);
}

int hal_write_mac_addr(unsigned char *mac_data, unsigned int mac_len)
{
    return eeprom_write_page(EEPROM_PAGE_FOR_MAC, mac_data, mac_len);
}

int hal_read_mac_addr(unsigned char *mac_data, unsigned int mac_len)
{
    return eeprom_read_page(EEPROM_PAGE_FOR_MAC, mac_data, mac_len);
}

int hal_write_encrypt_key(unsigned char *encrypt_key_data, unsigned int encrypt_key_len)
{
    return eeprom_write_page(EEPROM_PAGE_FOR_KEY, encrypt_key_data, encrypt_key_len);
}

int hal_read_encrypt_key(unsigned char *encrypt_key_data, unsigned int encrypt_key_len)
{
    return eeprom_read_page(EEPROM_PAGE_FOR_KEY, encrypt_key_data, encrypt_key_len);
}

int hal_get_temperature(int *val)
{
    *val = 0;
    return HLE_RET_ERROR;
}

void __suicide();

int hal_reboot()
{
    //do hal cleanup stuff

    __suicide();
    sleep(2);
    reboot(RB_AUTOBOOT); //2秒后如果看门狗还没有把系统复位，就使用reboot进行软重启
    return 0;
}

int hal_shutdown()
{
    //do hal cleanup stuff

    watchdog_disable();
    return reboot(RB_HALT_SYSTEM);
}
#endif

/*
功能：
    读指定大小的数据，如果出错则返回-1。
    如果读到文件末尾，返回的大小有可能小于指定的大小
参数：
    @fd : 文件描述符
    @vptr ：数据缓冲位置
    @n : 需要读的字节大小
返回：
    成功：读到的字节数
    失败：-1;
 */
ssize_t hal_readn(int fd, void *vptr, size_t n)
{
    ssize_t left;
    ssize_t read_len;
    char *ptr;

    ptr = vptr;
    left = n;
    while (left > 0) {
        read_len = read(fd, ptr, left);
        if ((read_len) < 0) {
            if (errno == EINTR)
                read_len = 0; /* and call read() again */
            else
                return -1; /* error */
        }
        else if (read_len == 0)
            break;
        left -= read_len;
        ptr += read_len;
    }
    return (n - left);
}

/*
功能：写指定大小的数据，如果出错则返回-1。
参数：
    @fd ： 文件描述符
    @vptr ：源数据缓存区位置
    @n: 要写入数据的大小
返回：
    成功 ：写入的字节数
    失败 ： -1
 */
ssize_t hal_writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return (-1); /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

/*mediaserver 回调函数--->JPEG 抓拍*/
HLE_S32 get_one_JPEG_frame(const void*frame_addr,HLE_S32 length)
{
    frame_addr = encoder_request_jpeg(0, &length, IMAGE_SIZE_1920x1080);
    if(NULL == frame_addr)
        return HLE_RET_ERROR;
    else
        return HLE_RET_OK;
        
}


static char audio_status = 1; //标记是否需要音频编码(默认有音频)
HLE_S32 MS_encoder_get_packet(HLE_S32 queue_id,HLE_S8 have_audio, void **pack_addr, void**frame_addr,HLE_S32* frame_length)
{

    if(NULL == pack_addr || NULL == frame_addr || NULL == frame_length)
    {
        ERROR_LOG("Illegal parameter!\n");
        return HLE_RET_EINVAL;
    }
       
    /*
    if(audio_status != have_audio)//audio 标记有变化再修改
    {
        audio_status = have_audio;
        //encoder_if_have_audio(stream_id,have_audio);//应该是直接判断包是否为音频包，进行过滤
    }
    */
    ENC_STREAM_PACK *pack = NULL;
    while(1)
    {
        pack = encoder_get_packet(queue_id);
        if(NULL == pack)
        {
            ERROR_LOG("encoder_get_packet failed ! queue_id = %d\n",queue_id);
            return HLE_RET_ERROR;
        }
        
        FRAME_HDR *header = (FRAME_HDR *) pack->data;
        if( !(have_audio) && header->type == 0xFA)//不需要audio帧 ,则过滤
        {
            encoder_release_packet(pack);
            continue;
        }
        else  //不需要audio帧 ，则不需处理
        {
            break;
        }
    }

    
    *pack_addr = pack;
    *frame_addr = pack->data;
    *frame_length = pack->length;
   // printf("get pack pack_addr(%p) frame_addr(%p) frame_length(%d) bytes\n",*pack_addr,*frame_addr,*frame_length);

    return HLE_RET_OK;
}



/*
    media server 回调函数，编码帧（包）的引用计数减1
*/
/*
extern int spm_dec_pack_ref(ENC_STREAM_PACK *pack);
int dec_frame_ref(void *pack_addr)
{

    if(NULL == pack_addr)
        return HLE_RET_EINVAL;
    
    //DEBUG_LOG("dec_frame_ref free pack addr = %p\n\n",pack_addr);

    spm_dec_pack_ref((ENC_STREAM_PACK *)pack_addr);
    
    return HLE_RET_OK;
}
*/

/*
media server 释放一个包（回调接口）
返回：
    失败：-1
    成功：0
*/
int MS_encoder_release_packet(void* pak)
{
    if(NULL == pak)
    {
        ERROR_LOG("the pak is NULL!\n");
        return -1;
    }
    
    ENC_STREAM_PACK * pack = (ENC_STREAM_PACK*)pak;
    return encoder_release_packet(pack);
    
}

int media_server_module_init(void)
{
    med_ser_init_info_t med_ser_init_info;
    memset(&med_ser_init_info,0,sizeof(med_ser_init_info));

    med_ser_init_info.encoder_request_stream = encoder_request_stream;
    med_ser_init_info.encoder_get_packet = MS_encoder_get_packet;
    med_ser_init_info.encoder_release_packet = MS_encoder_release_packet;
    med_ser_init_info.encoder_free_stream = encoder_free_stream;
   
    med_ser_init_info.encoder_force_iframe = encoder_force_iframe;
    med_ser_init_info.get_one_JPEG_frame = get_one_JPEG_frame;
    med_ser_init_info.use_wireless_network = 1;
    //其余的项暂时不初始化，以后需要再加
    
    int ret =  media_server_init(&med_ser_init_info, sizeof(med_ser_init_info_t));
    if(HLE_RET_OK == ret)
        return HLE_RET_OK;
    else
        return HLE_RET_ERROR;
}


int file_size(char* filename)  
{  
    FILE *fp=fopen(filename,"r");  
    if(!fp) return -1;  
    fseek(fp,0L,SEEK_END);  
    int size=ftell(fp);  
    fclose(fp);  
      
    return size;  
}  



extern void fmp4_record_exit(fmp4_out_info_t *info);
/*******************************************************************************
*@ Description    :  MD 告警响应函数（fmp4录像 + TS切片 + 推送amazon云）
*@ Input          :
*@ Output         :
*@ Return         :
*******************************************************************************/
int recode_mp4_done = 1; //标记 mp4 文件录制是否结束 (0:没结束 1：结束)
pthread_mutex_t MD_func_mut; //MD 告警 响应线程 锁 
#define OUT_FILE_BUF_SIZE (2*1024*1024) //初始化 3M大小空间（15S音视频）
#define MD_ALARM_RECODE_TIME  6  //MD告警录制时长
/*---# m3u索引文件类型------------------------------------------------------------*/
#define M3U_TS_FILE     1
#define M3U_FMP4_FILE   2
#define M3U_INDEX_FILE_TYPE  M3U_TS_FILE
void* MD_alarm_response_func(void* args) //文件模式 版本
{
    /*---#目前支持的有: 15s录制视频切片上传------------------------------------------------------------*/
    
    printf("\n\n=======start MD_alarm_response_func==================================================================\n");
    pthread_detach(pthread_self());
    
    void *  recode_data  = NULL; //最终录制的文件数据buf
    unsigned int  recode_data_len = 0; //最终录制的文件数据buf长度 
    fmp4_out_info_t out_mp4_info = {0};
    
    if(M3U_INDEX_FILE_TYPE == M3U_TS_FILE)
    {
        void* out_buf = NULL;
        int out_len = 0;
        if(ts_record(&out_buf,&out_len,MD_ALARM_RECODE_TIME) < 0)
        {
            ERROR_LOG("ts_record failed!\n");
            pthread_exit(NULL);
        }
        recode_data = out_buf;
        recode_data_len = (int)out_len;

        /***DEBUG 整体再写入到文件*********************************/
        #if 1
        char* debug_file_name = "/jffs0/MD_alarm_20190420.ts";
        if(0 == access(debug_file_name,F_OK))
        {
            if(0 == remove(debug_file_name))
            {
                DEBUG_LOG("remove old file success!\n");
            }
            else
            {
                ERROR_LOG("remove old file failed!\n");
                goto ERR;
            }
        }

        FILE*debug_file = fopen(debug_file_name, "wb+");
        if(NULL == debug_file )
        {
            ERROR_LOG("debug open fmp4 file failed!\n");
            goto ERR;
        }
        DEBUG_LOG("debug open file success!\n");

        int debug_ret = fwrite(recode_data,1,recode_data_len,debug_file);
        if(debug_ret < 0)
        {
            ERROR_LOG("write file error!\n");
            fclose(debug_file);
            goto ERR;
        }
        DEBUG_LOG("----fewite file size(%d)----\n",debug_ret);
        fclose(debug_file);
        #endif
        /*****************************************/
        
        if(recode_data) {free(recode_data);recode_data = NULL;}
        
    }
    else //if(M3U_INDEX_FILE_TYPE == M3U_FMP4_FILE)
    {
        /*---# MP4 文件录制--------------------------------------------------*/
        out_mp4_info.recode_time = MD_ALARM_RECODE_TIME;
        /*---#配置 buf存储 模式-----*/
        out_mp4_info.buf_mode.buf_start = (char*)calloc(OUT_FILE_BUF_SIZE,sizeof(char));
        if(NULL == out_mp4_info.buf_mode.buf_start)
        {
            ERROR_LOG("calloc failed!\n");
             pthread_exit(NULL);
        }
        out_mp4_info.buf_mode.buf_size = OUT_FILE_BUF_SIZE;
        out_mp4_info.buf_mode.w_offset = 0;
        out_mp4_info.file_mode.file_name = NULL; //不采用 文件模式 liteos 的 ramfs 延时太大
        
        /*---#开始录制------------*/
        if(NULL == fmp4_record(&out_mp4_info))
        {
            ERROR_LOG("call fmp4_record failed!\n");
            fmp4_record_exit(&out_mp4_info);
            pthread_exit(NULL);
        }
        recode_data = out_mp4_info.buf_mode.buf_start;
        recode_data_len = out_mp4_info.buf_mode.w_offset;
        /*---# END------------------------------------------------------------*/
    }
    
    
    
    /*---# 推送到 amazon 云---------------------------------------------------*/
    #if 0
        #if 0
        while(1 != is_amazon_info_update()) //该处逻辑还是欠妥，后续改进
        {
            DEBUG_LOG("wait g_amazon_info update .....\n");
            sleep(2);
        }
        #endif
    int i= 0;
    for(i = 0 ; i < tmp_info->ts_num ; i++)
    {
        #if 1
        put_file_info_t file_info = {0};
        file_info.mode = 2; //缓存buf模式
        file_info.file_tlen = MD_ALARM_RECODE_TIME;
        if(M3U_INDEX_FILE_TYPE == M3U_TS_FILE) 
            file_info.file_type = TYPE_TS;
        else //if(M3U_INDEX_FILE_TYPE == M3U_FMP4_FILE) //内部传输逻辑和 TS 一样
            file_info.file_type = TYPE_TS;  
        
        file_info.ts_flag = TS_FLAG_ONE;  //目前按照只有一个15 s TS文件的方案去实现？？？？？15S内发生了二次告警就接着录制，但内存很可能不够，后续考虑
        strcpy(file_info.file_name,tmp_info->ts_array[i].ts_name);
        file_info.file_buf = tmp_info->ts_array[i].ts_buf;
        file_info.file_buf_len = tmp_info->ts_array[i].ts_buf_size;
        strcpy(file_info.m3u8name,tmp_info->m3u_name);
        
        time_t timer = time(NULL);
        //struct tm *lctv = localtime(&timer);
        memcpy(&file_info.datetime,&timer,sizeof(timer));
        amazon_put_even_thread(&file_info);
       
        #endif
        
        #if 0  //DEBUG 写到本地端
            int fd = -1;
            int ret = 0;
            /*
            if(0 == i)
            {   printf("out_ts_info->m3u_name = %s\n",ts_info->m3u_name);
                fd = open(ts_info->m3u_name , O_CREAT | O_WRONLY | O_TRUNC, 0664);
                ret = write(fd, ts_info->m3u_buf, ts_info->m3u_buf_size);
                if(ret != ts_info->m3u_buf_size)
                {
                    ERROR_LOG("write error!\n");
                    close(fd);
                    goto ERR;
                }
                close(fd);
            }
            */
            printf("out_ts_info->ts_array[%d].ts_name = %s\n",i,tmp_info->ts_array[i].ts_name);
            fd = open(tmp_info->ts_array[i].ts_name , O_CREAT | O_WRONLY | O_TRUNC, 0664);
            ret = write(fd, tmp_info->ts_array[i].ts_buf, tmp_info->ts_array[i].ts_buf_size);
            if(ret != tmp_info->ts_array[i].ts_buf_size)
            {
                ERROR_LOG("write error!\n");
                close(fd);
                goto ERR;
            }
            close(fd);
            printf("****write file success!****************************************************\n");
        #endif
    }
    
    #endif
    
ERR:
    if(recode_data) {free(recode_data);recode_data = NULL;}
    if(out_mp4_info.buf_mode.buf_start) fmp4_record_exit(&out_mp4_info);
    recode_mp4_done = 1;//标记结束
    printf("=======END MD_alarm_response_func==================================================================\n");
    pthread_exit(NULL);
    
}


extern int is_amazon_info_update(void);
//该函数只供单独测试 amazon 用，不需要启动 sample 主线程
int test_amazon(int argc, char *argv[])
{
    #if 1  //amazon 云服务部分
       amazon_S3_req_thread(); //"设备图片视频上传接口" 信息更新线程

       put_file_info_t file_info = {0};
       char* file_name = "/jffs0/fmp4_1.ts";
       char* m3u8_name = "/jffs0/fmp4_m3u8.m3u8";
      DEBUG_LOG("file_name = %s \n",file_name);
      DEBUG_LOG("m3u8_name = %s \n",m3u8_name);
      
       file_info.file_tlen = 6;    //6S时长
       file_info.file_type = TYPE_TS;
       file_info.ts_flag = TS_FLAG_6S;
       printf("sizeof(file_info.ts_flag) = %d\n",sizeof(file_info.ts_flag));//4
       strncpy(file_info.file_name,file_name,strlen(file_name));
       strncpy(file_info.m3u8name,m3u8_name,strlen(m3u8_name)); 
       time_t timer = time(NULL);
       //struct tm *lctv = localtime(&timer);
       memcpy(&file_info.datetime,&timer,sizeof(timer));
       while(1 != is_amazon_info_update())
       {
            printf("wait g_amazon_info update .....\n");
            sleep(2);
       }
      
       amazon_put_even_thread(&file_info);//临时调试放在这，调试好后放到该放的地方去

      
        sleep(5);//等待线程退出，不然 file_name等栈空间会被释放
        DEBUG_LOG("test_amazon exit!\n");
       
       return 0;
   #endif


}




#if 1
//#define USE_AI_AENC_BIND  1   // 打开该宏 ：AI绑定到AENC          注释该宏：AI直接输出到AO  
extern int AI_to_AO_start;
int app_main(int argc, char *argv[])
{
    int ret;


    ret = hal_init(200, VIDEO_STD_NTSC);
    if (HLE_RET_OK != ret) {
        ERROR_LOG("hal_init fail!\n");
        return -1;
    }
    

    #if 0 //P2P 媒体服务程序部分
        ret = media_server_module_init();
        if (HLE_RET_OK != ret) {
            ERROR_LOG("media_server_module_init fail!\n");
            return -1;
        }
        
        ret = media_server_start_thread();
        if (HLE_RET_OK != ret) {
            ERROR_LOG("media_server_start_thread fail!\n");
            return -1;
        }
    #endif

    #if 0  //start amazon info update thread
        amazon_S3_req_thread(); 
    #endif

    
    int i, snap;
    int id[STREAMS_PER_CHN];
    int fd[STREAMS_PER_CHN];
    int aud;

    for (i = 0; i < STREAMS_PER_CHN; ++i) 
    {
        ENC_STREAM_ATTR attr;
        encoder_get_optimized_config(0, i, &attr);
        encoder_config(0, i, &attr);

        
        //获取queue_id
        id[i] = encoder_request_stream(0, i, 1);
        
        DEBUG_LOG("stream id[%d] = %d\n",i, id[i]);
        char buf[50];
        snprintf(buf, sizeof(buf), "/jffs0/test%d.h264", i);
       // fd[i] = open(buf, O_CREAT | O_WRONLY | O_TRUNC, 0664);

    }

    


    OSD_BITMAP_ATTR osdAttr;
    osdAttr.enable = 1;
    osdAttr.x = 100;
    osdAttr.y = 100;
    osdAttr.width = 8 * 20; //使用矢量字体库时无效
    osdAttr.height = 16;    //使用矢量字体库时无效
    osdAttr.fg_color = 0xffffffff;
    osdAttr.bg_color = 0;
    osdAttr.raster = NULL;
    videoin_set_osd(0, TIME_OSD_INDEX, &osdAttr);
    osdAttr.x = 100;
    osdAttr.y = 600;
    osdAttr.width = 8 * 20; //使用矢量字体库时无效
    osdAttr.height = 16;    //使用矢量字体库时无效
    videoin_set_osd(0, RATE_OSD_INDEX, &osdAttr);

    ENC_JPEG_ATTR jpgAttr;
    jpgAttr.img_size = IMAGE_SIZE_1920x1080;
    jpgAttr.level = 0;
    encoder_config_jpeg(0, &jpgAttr);

    
    #if 0
    
    play_tip("/jffs0/001.pcm",NULL,NULL);
    #else
    AI_to_AO_start = 1;//开始AI直接输出到AO模式
    #endif
    
    snap = 0;
    int cycle_num = 0;
    int skip_len = 0;
    int snap_count = 2;//定义抓拍保存的图片数量

    #if 0  //debug
    pthread_t threadID_Idle;
    HLE_S32 err = pthread_create(&threadID_Idle, NULL, &fmp4_record, NULL);
    if (0 != err) 
    {
        ERROR_LOG("create media_server_start failed!\n");
        return HLE_RET_ERROR;
    }   
    DEBUG_LOG("fmp4_record start success!\n");
    #endif
    

    for (;;) 
    {
         for (i = 0; i < STREAMS_PER_CHN; ++i) 
        {
      
            ENC_STREAM_PACK *pack = encoder_get_packet(id[i]);
            FRAME_HDR *header = (FRAME_HDR *) pack->data;
            if (header->sync_code[0] != 0 || header->sync_code[1] != 0\
            || header->sync_code[2] != 1) {
                printf("------BUG------\n");
                continue;
            }

            //0xF8-视频关键帧
            if (header->type == 0xF8) {
                skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
                //write(fd[i], pack->data + skip_len, pack->length - skip_len);
            }
            //0xF9-视频非关键帧
            else if (header->type == 0xF9) {
                skip_len = sizeof (FRAME_HDR) + sizeof (PFRAME_INFO);
                //write(fd[i], pack->data + skip_len, pack->length - skip_len);
            }
            //0xFA-音频帧
            else if (header->type == 0xFA){
                 skip_len = sizeof (FRAME_HDR) + sizeof (AFRAME_INFO);
                //写音频文件放到了开始音频编码的线程了
            }
            else {
                printf("------BUG------\n");
                continue;
            }
            
            encoder_release_packet(pack);
            //spm_pack_print_ref(pack);
    
        }
     
        /*---# MD告警录视频制 + 上传到 amazon云 部分----------------------*/
        #if 1
            int motion = 0;
            motion_detect_get_state(&motion);
            if(motion && recode_mp4_done)//有告警产生 + 上一次录像已经结束
            {
                recode_mp4_done = 0;//标记开始
                DEBUG_LOG("---Alarm---------------------------------------------------------!\n");
                pthread_t threadID;
                HLE_S32 err = pthread_create(&threadID, NULL, &MD_alarm_response_func, NULL);
                if (0 != err) 
                {
                    ERROR_LOG("create media_server_start failed!\n");
                    continue;
                } 
            }
            
        #endif
        /*---#------------------------------------------------------------*/
  
        #if 0
            ++snap;
            if ((snap % 30) == 0) 
            {
                snap = 0;
                int size;
                char * jpg = encoder_request_jpeg(0, &size, IMAGE_SIZE_1920x1080);
                if (jpg) 
                {
                    #if 0
                    if( snap_count > 0)
                    {
                        snap_count --;
                        char buf[50];
                        snprintf(buf, sizeof(buf), "/jffs0/snap%d.jpg", snap);
                        int fd = open(buf, O_CREAT | O_WRONLY | O_TRUNC, 0664);
                        write(fd, jpg, size);
                        close(fd);
                    }
                    #endif
                    encoder_free_jpeg(jpg);
                }
            }
        #endif

    }

    for (i = 0; i < STREAMS_PER_CHN; ++i) {
        encoder_free_stream(id[i]);
        close(fd[i]);
    }
    
    hal_exit();
    return 0;
}

//simple test, disable it if not needed, don't remove it
#else //原来调试通过的代码
int app_main(int argc, char *argv[])
{
    int ret = hal_init(200, VIDEO_STD_NTSC);
    if (HLE_RET_OK != ret) {
        ERROR_LOG("hal_init fail!\n");
        return -1;
    }
    
    int i, cnt, snap;
    int id[STREAMS_PER_CHN];
    int fd[STREAMS_PER_CHN];
    int aud;

    for (i = 0; i < STREAMS_PER_CHN; ++i) {
      //  if(1 == i)continue;

        ENC_STREAM_ATTR attr;
        encoder_get_optimized_config(0, i, &attr);
        encoder_config(0, i, &attr);
        id[i] = encoder_request_stream(0, i, 1);

        char buf[50];
        snprintf(buf, sizeof(buf), "/jffs0/test%d.h264", i);
        fd[i] = open(buf, O_CREAT | O_WRONLY | O_TRUNC, 0664);

    }


    aud = open("rawaudio", O_CREAT | O_WRONLY | O_TRUNC, 0664);

    OSD_BITMAP_ATTR osdAttr;
    osdAttr.enable = 1;
    osdAttr.x = 100;
    osdAttr.y = 100;
    osdAttr.width = 8 * 20;
    osdAttr.height = 16;
    osdAttr.fg_color = 0xffffffff;
    osdAttr.bg_color = 0;
    osdAttr.raster = NULL;
    videoin_set_osd(0, TIME_OSD_INDEX, &osdAttr);
    osdAttr.x = 100;
    osdAttr.y = 600;
    osdAttr.width = 8 * 20;
    osdAttr.height = 16;
    videoin_set_osd(0, RATE_OSD_INDEX, &osdAttr);

    ENC_JPEG_ATTR jpgAttr;
    jpgAttr.img_size = IMAGE_SIZE_1920x1080;
    jpgAttr.level = 0;
    encoder_config_jpeg(0, &jpgAttr);

    snap = 0;
    for (cnt = 0; cnt < 15 * 60; ++cnt) {
        for (i = 0; i < STREAMS_PER_CHN; ++i) {
            ENC_STREAM_PACK *pack = encoder_get_packet(id[i]);
            FRAME_HDR *header = (FRAME_HDR *) pack->data;
            if (header->sync_code[0] != 0 || header->sync_code[1] != 0
                || header->sync_code[2] != 1) {
                printf("------BUG------\n");
                continue;
            }

            //0xF8-视频关键帧
            if (header->type == 0xF8) {
                int skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
                write(fd[i], pack->data + skip_len, pack->length - skip_len);
            }
            //0xF9-视频非关键帧
            else if (header->type == 0xF9) {
                int skip_len = sizeof (FRAME_HDR) + sizeof (PFRAME_INFO);
                write(fd[i], pack->data + skip_len, pack->length - skip_len);
            }
            //0xFA-音频帧
            else if (header->type == 0xFA) {
                int skip_len = sizeof (FRAME_HDR) + sizeof (AFRAME_INFO);
                write(aud, pack->data + skip_len, pack->length - skip_len);
            }
            else {
                printf("------BUG------\n");
            }
            encoder_release_packet(pack);
        }

#if 1
        ++snap;
        if ((snap % 30) == 0) {
            int size;
            char * jpg = encoder_request_jpeg(0, &size, IMAGE_SIZE_1920x1080);
            if (jpg) {
                char buf[50];
                snprintf(buf, sizeof(buf), "snap%d.jpg", snap);
                int fd = open(buf, O_CREAT | O_WRONLY | O_TRUNC, 0664);
                write(fd, jpg, size);
                close(fd);
                encoder_free_jpeg(jpg);
            }
        }
#endif

    }

    for (i = 0; i < STREAMS_PER_CHN; ++i) {
        encoder_free_stream(id[i]);
        close(fd[i]);
    }
    hal_exit();
    return 0;
}

#endif










































