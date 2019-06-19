#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

#include "typeport.h"
#include "faac.h"
#include "hi_comm_aio.h"


#ifdef __cplusplus
extern "C"
{
#endif


//对讲音频编解码格式信息

typedef struct
{
    HLE_U8 enc_type; //音频编码类型，具体见E_AENC_STANDARD
    HLE_U8 sample_rate; //采样频率，具体见E_AUDIO_SAMPLE_RATE
    HLE_U8 bit_width; //采样位宽，具体见E_AUDIO_BIT_WIDTH
    HLE_U8 sound_mode; //单声道还是立体声，具体见E_AUDIO_SOUND_MODE
} TLK_AUDIO_FMT;


/*
    function:  audioin_get_chns
    description:  获取视频伴音输入通道数
    args:
        无
    return:
        >=0, 成功，返回伴音通道数
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int audioin_get_chns(void);


/*
    function:  talkback_get_audio_fmt
    description:  获取对讲音频编码格式以及支持的解码格式
    args:
        TLK_AUDIO_FMT *aenc_fmt[out]    编码格式
        TLK_AUDIO_FMT *adec_fmt[out]    解码格式
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_get_audio_fmt(TLK_AUDIO_FMT *aenc_fmt, TLK_AUDIO_FMT *adec_fmt);

/*
    function:  talkback_start
    description:  开始对讲接口
    args:
        int getmode[in] 1:捕获数据 0:不捕获
        int putmode[in] 1:开启音频解码 0:不开启
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_start(int getmode, int putmode);

/*
    function:  talkback_stop
    description:  结束对讲接口
    args:
        无
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_stop(void);

/*
    function:  talkback_get_data
    description:  对讲获取本地数据接口
    args:
        void *data[in]，数据缓冲地址
        int *size[in,out]，传入的是缓冲区的长度，传出的是实际采集到的数据长度
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_get_data(void *data, int *size);

/*
    function:  talkback_put_data
    description:  对讲数据播放接口
    args:
        void *data[in]，数据缓冲地址
        int size[in]，数据长度
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_put_data(void *data, int size);

typedef void (*PLAY_END_CALLBACK)(void* para);
int play_tip(char* fn, PLAY_END_CALLBACK cbFunc, void* cbfPara);
int hal_audio_init(void);
int hal_audio_exit(void);
//int hal_sonic_wave_init(void);
//int hal_sonic_wave_exit(void);
//int sonic_aio_disable(void);


/***************************************************************************************
								AAC编码部分
采样率：16KHZ
每帧样本数：n (160 、320 、 480 ) 
帧率：16000/n
***************************************************************************************/

/*0:使用AAC编码      1:使用海思音频编码         总控开关*/
#define USE_HISI_AENC 0
  
#define PCM_SAMPLE_RATE     (16000)                     //PCM源数据的采样率，用于AAC编码参数设置
#define AAC_INPUT_SAMPLES   AUDIO_PTNUMPERFRM           //输入给AAC编码的样本数（1帧PCM数据样本数）
#define PCM_FRAME_SIZE      (AAC_INPUT_SAMPLES * 2)     //一个PCM音频帧的字节数
//采用自动获取的方式（faacEncOpen函数）获得 #define PCM_BUF_SIZE        (2048)          //用于凑数据进行AAC编码的PCM 缓存大小
typedef struct _aac_config_t
{
    faacEncHandle   EncHandle;          //音频文件描述符
    unsigned long   SampleRate;         //音频采样数
    unsigned int    Channels;           //音频声道数
    unsigned int    PCMBitSize;         //音频采样精度（位宽）
    unsigned long   InputSamples;       //每次调用编码时所应接收的原始数据采样点个数（凑数据后的 FRAME_LEN*numChannels）
    unsigned long   MaxOutputBytes;     //每次调用编码时生成的AAC数据的最大长度
    unsigned int    PCMBuffer_size;     //每次传给faac库（凑数据后）进行AAC编码的数据长度（每次调用并不是都返回AAC帧，只有凑足数据了才会进行AAC编码返回）
    unsigned char*  PCMBuffer;          //pcm数据（用于faac库凑PCM样本数）
    unsigned char*  AACBuffer;          //aac数据
}aac_config_t;

/*******************************************************************************
*@ Description    :初始化AAC编码
*@ Input          :
*@ Output         :
*@ Return         :成功：配置好的AAC参数指针（是个全局变量的地址，不需要释放）
					失败：NULL
*@ attention      :
*******************************************************************************/
aac_config_t* AAC_encode_init(void);

/*******************************************************************************
*@ Description    :发送一帧audio数据给AAC编码线程编码
*@ Input          :<pstFrm >音频帧结构体指针
*@ Output         :
*@ Return         :	成功 ： 0
					失败： -1
*@ attention      :
*******************************************************************************/
int AAC_AENC_SendFrame( const AUDIO_FRAME_S *pstFrm);

/*******************************************************************************
*@ Description    :获取一帧编码后的AAC数据（编码业务在该函数内进行）
*@ Input          :
*@ Output         :<pstStream>(输出)编码后AAC数据存放buf的首地址
*@ Return         :成功： AAC编码帧的实际长度
					失败：-1
*@ attention      :
*******************************************************************************/
int AAC_AENC_getFrame(AUDIO_STREAM_S *pstStream);



/*******************************************************************************
*@ Description    :编码一帧AAC帧
*@ Input          :<inputBuffer> 输入的PCM数据buf
					<samplesInput>输入的PCM数据所包含采样的样本数
*@ Output         :<outputBuffer>编码后的AAC数据指针
					（全局缓冲空间，单次调用不需要释放，AAC编码退出统一释放）
					<bufferSize>编码后的AAC数据大小
*@ Return         :成功：返回编码成AAC帧的字节数（大于0）
					失败： 小于等于0的数
*@ attention      :
*******************************************************************************/
int AAC_encode(int * inputBuffer,
                    unsigned int samplesInput,
                    unsigned char *outputBuffer,
                    unsigned int bufferSize);

/*******************************************************************************
*@ Description	  :退出AAC编码
*@ Input		  :
*@ Output		  :
*@ Return		  :成功：0
					失败：-1
*@ attention	  :
*******************************************************************************/
int  AAC_encode_exit(void);




#ifdef __cplusplus
}
#endif

#endif


