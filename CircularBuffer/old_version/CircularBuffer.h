 
/***************************************************************************
* @file: CircularBuffer.h 
* @author:   
* @date:  5,18,2019
* @brief:  循环缓冲池头文件
* @attention:
***************************************************************************/
#ifndef _CIRCULAR_BUFFER_H
#define _CIRCULAR_BUFFER_H

#ifndef false 
#define false  0
#endif

#ifndef true 
#define true  1
#endif



#define BUFFER_MAIN_SIZE	1024*1024*4		/*主循环缓冲buf的大小*/
#define BUFFER_SECN_SIZE	1024*1024*2		/*次循环缓冲buf的大小*/

#define RECODE_TIME		40		//最大缓存音视频时长（秒）
#define MAX_I_F_NUM		(RECODE_TIME / 2)  //最大I帧数量（gop:30帧; 帧率:15帧/s ; I帧:2s/I帧，必须是偶数，否则管理结构字节无法对齐） 	
#define MAX_A_F_NUM		(RECODE_TIME * 15)	//最大的audio帧数量（audio: 15帧/s , aac帧）
#define MAX_V_F_NUM		(RECODE_TIME * 15) //最大的 video帧数量（video：15帧/s ）
#define MAX_FRM_NUM     (MAX_A_F_NUM + MAX_V_F_NUM)

#define MAX_USER_NUM	6	//缓存池支持的最大同时 “读” 用户个数

//放到编码头文件定义
/*
typedef struct _venc_stream_t 
{
	venc_pkt_t pkt[MAX_PKT_NUM_IN_STREAM];
	unsigned pkt_cnt; 
	unsigned seq;
} venc_stream_t;
*/


//一个 video/audio 帧在缓冲池的信息结构体
typedef struct  _FrameInfo_t
{
	HLE_U32			frmStartPos;	/*此帧在buffer中的偏移*/
	HLE_U32			frmLength;  	/*此帧的有效数据长度*/
	HLE_U64			PTS;			/*如果是视频帧，则为此帧的时间戳*/
	HLE_U8			flag;			/* 1:I 帧, 2:P 帧, 3:音频帧*/      
	HLE_U8			hour;			/*产生此帧的时间*/
	HLE_U8			min;
	HLE_U8			sec;
	//venc_stream_t	venc_stream; 	/* video帧可能包含多个NAL单元，类型在编码头文件定义 */
}FrameInfo_t;

/*循环缓冲区头结构
	
	注意：虽然网络情况好的情况下读写指针应该几乎同步，不会差距太大，不会发生如下的问题。
		但在网络情况差的情况下：
		@ 要考虑好 “帧数据缓存部分的buffer 能缓存的帧数[1]”与 “FrmList 数组元素个数[2]” 以及
			“IFrmIndex 数组元素个数[3]”的匹配问题。
		@ 要考虑好[1][2][3]不同时发生 “写指针踩踏读指针” 的问题。
		例如：
			当 [1]比[2]大很多时，[2]将比[1]更加容易发生“写指针踩踏读指针”，同理[3]也是，如此就会 
			发生混乱。比如，[1]没有发生踩踏，而[2]发生了踩踏，踩踏会改变[2]中读指针对应的
			帧描述信息，此时如果还继续按照之前的读指针去读，将会读到别的帧，这样会导致帧数据错乱，
			为避免这样的意外发生，采取的措施如下：
			1.虽然不可能做做到完全匹配，但要尽量保证[1][2][3]之间元素个数匹配，不能差异太大
			2.将[1][2][3]都引入“跳帧操作”当其中一个发生踩踏时，都会联动其他的两个进行跳帧操作。
*/
typedef struct _CircularBuffer_t 
{
	HLE_U8			*bufStart;				/*媒体数据buf 起始地址*/
	HLE_U32			bufSize;				/*buf 空间大小*/
	HLE_U32			occupiedSize;			/*实际已使用空间大小*/
	HLE_U32 		writePos;				/*写指针偏移*/
//	HLE_U32 		readPos;				/*读指针偏移,放到用户信息中了*/

	FrameInfo_t		FrmList[MAX_FRM_NUM];	/*buf 中存储的帧列表信息*/
	HLE_U16		 	FrmList_w;				/*写指针，在 FrmList 中的下标*/
	HLE_U16		 	FrmList_r;				/*读指针，在 FrmList 中的下标,该变量的跳帧操作由readPos变量进行联动*/		
	HLE_U16		 	totalFrm;				/*总帧数（有效帧，除去跳帧）*/
	HLE_U16		 	reserved1;
	
	HLE_U32 		IFrmIndex[MAX_I_F_NUM];	/*buf中I帧在 FrmList 数组中的下标信息*/
	HLE_U16			IFrmIndex_w;			/*写指针所属的I帧,在 IFrmIndex 中的索引*/
	HLE_U16			IFrmIndex_r;			/*读指针所属的I帧,在 IFrmIndex 中的索引*/
	HLE_U16			totalIFrm;				/*当前buffer中总的有效的I帧数目*/
	HLE_U16		 	reserved2;
	
}CircularBuffer_t;


//用户读缓存的信息结构（支持多个用户同时读缓存，用户只能读）
typedef struct _UserInfo_t
{
	HLE_U32 		readPos;				/*当前用户的读指针偏移*/
	HLE_U32			dropFramcount;			/*上一次跳帧时丢帧的个数。由上层来清零*/
}UserInfo_t;

static UserInfo_t		userArray[MAX_USER_NUM];/*正在“读缓存”的用户信息描述数组*/


#endif


