

#include "CircularBuffer.h"


/*******************************************************************************
*@ Description    :循环缓冲 buf 创建
*@ Input          :<size> 缓冲区的大小
*@ Output         :
*@ Return         :成功：缓冲区的指针 
					失败：NULL
*@ attention      :
*******************************************************************************/
CircularBuffer_t* CircularBufferCreate(HLE_S32 size)
{
	HLE_S32 malloc_size = sizeof(CircularBuffer_t) + size;
	CircularBuffer_t * circular_buf = (CircularBuffer_t *)malloc(malloc_size);
	if(NULL == circular_buf)
	{
		CBUF_ERROR_LOG("malloc failed !\n");
		return NULL;
	}
	memset(circular_buf,0,malloc_size);

	circular_buf->bufStart = circular_buf + sizeof(CircularBuffer_t);
	circular_buf->bufSize = size;
	circular_buf->writePos = 0;
	circular_buf->readPos = 0;
	//memset 0 已经相当于初始化
	
	return circular_buf;
}

void CircularBufferFree(CircularBuffer_t *cBuf)
{
	if(NULL != cBuf)
	{
		cBuf->bufStart = NULL;//buf是一起的，bufStart不需要释放
		free (cBuf);
		cBuf = NULL;
		
	}
}


/*
void CircularBufferReset(CircularBuffer_t * cBuf)
{

}

HLE_S32 CircularBufferGetCapacity(CircularBuffer_t * cBuf)
{

}

HLE_S32 CircularBufferGetSize(CircularBuffer_t * cBuf)
{

}
*/


/*******************************************************************************
*@ Description    :存放一帧video/audio帧到缓存
*@ Input          :<cBuf>缓存指针
					<data> 帧数据指针
					<length>帧数据长度
*@ Output         :
*@ Return         :成功：0
					失败：-1
*@ attention      :写操作要注意 “写指针踩踏读指针” 的问题
*******************************************************************************/
static HLE_S32 CircularBufferWriteOneFrame(CircularBuffer_t * cBuf,void *data,HLE_S32 length)
{
	if(NULL == cBuf || NULL == data || length <= 0)
	{
		CBUF_ERROR_LOG("Illegal parameter!\n");
		return -1;
	}

	HLE_S32 writableLen = length;//要写入的数据之长度
    void *pSrc = data; //要写入的数据

	/*---#对写入的数据进行长度安全控制------------------------------------------------------------*/
    if(writableLen > cBuf->bufSize)//如果传入的数据已经超过了缓存区的总大小，部分写入(实际不存在这么大的帧)
    {
    	CBUF_ERROR_LOG("writableLen > cBuf->size!\n");
		return -1;
    }
    
    /*---# 记录帧信息------------------------------------------------------------*/
	FrameInfo_t FrameInfo = {0};
	FrameInfo.frmStartPos = cBuf->writePos;
	FrameInfo.frmLength = length;
//	FrameInfo.PTS = 后续待定
//	FrameInfo.flag = 
//	FrameInfo.hour = 
//	FrameInfo.min = 
//	FrameInfo.sec = 

	/*---#填充帧索引列表 FrmList ------------------------------------------------------------*/
	if(cBuf->FrmList_w == cBuf->FrmList_r)// FrmList 发生踩踏，要进行 "跳帧操作"
	{	
		CBUF_DEBUG_LOG("FrmList_r Pointer stampede! Jump frame operation...\n");
		//1.IFrmIndex_r 跳转到下一个I帧位置的下标处
		cBuf->IFrmIndex_r += 1
		if(cBuf->IFrmIndex_r >= MAX_I_F_NUM)
			cBuf->IFrmIndex_r = 0;
		
		//2.FrmList_r 跳转到 帧信息数组（IFrmIndex）的下一个I帧位置
		HLE_U32 index = cBuf->IFrmIndex[cBuf->IFrmIndex_r];
		cBuf->FrmList_r = index;

		//3.帧数据缓存buffer的读指针 readPos 跳转到该FrmList_r对应帧的偏移位置 
		HLE_U32 old_readPos = cBuf->readPos;
		cBuf->readPos = cBuf->FrmList[cBuf->FrmList_r].frmStartPos;
		HLE_U32 jump_size = old_readPos < cBuf->readPos ?cBuf->readPos - old_readPos:cBuf->bufSize - (old_readPos - cBuf->readPos);

		cBuf->totalIFrm -= 1;
		cBuf->occupiedSize -= jump_size;
		
	}
	
	cBuf->FrmList[cBuf->FrmList_w] = FrameInfo;
	cBuf->FrmList_w ++;
	if(cBuf->FrmList_w >= MAX_FRM_NUM)
		cBuf->FrmList_w = 0;
	
	cBuf->totalFrm ++;
	
	/*---#如果是I帧还需要填充I帧索引列表 IFrmIndex_w------------------------------------------------------------*/
	if(/*是I帧*/)
	{
		if(cBuf->IFrmIndex_w == cBuf->IFrmIndex_r)// IFrmIndex_r 发生踩踏，要进行 "跳帧操作"
		{
			CBUF_DEBUG_LOG("IFrmIndex_r Pointer stampede! Jump frame operation...\n");
			//1.IFrmIndex_r 跳转到下一个I帧位置的下标处
			cBuf->IFrmIndex_r += 1
			if(cBuf->IFrmIndex_r >= MAX_I_F_NUM)
				cBuf->IFrmIndex_r = 0;
			
			//2.FrmList_r 跳转到 帧信息数组（IFrmIndex）的下一个I帧位置
			HLE_U32 index = cBuf->IFrmIndex[cBuf->IFrmIndex_r];
			cBuf->FrmList_r = index;

			//3.帧数据缓存buffer的读指针 readPos 跳转到该FrmList_r对应帧的偏移位置 
			HLE_U32 old_readPos = cBuf->readPos;
			cBuf->readPos = cBuf->FrmList[cBuf->FrmList_r].frmStartPos;
			HLE_U32 jump_size = old_readPos < cBuf->readPos ?cBuf->readPos - old_readPos:cBuf->bufSize - (old_readPos - cBuf->readPos);
			
			cBuf->totalIFrm -= 1;
			cBuf->occupiedSize -= jump_size;
		}
		
		cBuf->IFrmIndex[cBuf->IFrmIndex_w] = cBuf->FrmList_w;	
		cBuf->IFrmIndex_w ++;
		if(cBuf->IFrmIndex_w >= MAX_I_F_NUM)
			cBuf->IFrmIndex_w = 0;
		cBuf->totalIFrm ++;
	
	}

	/*---# 写入帧数据到缓存------------------------------------------------------------*/
	/*在帧缓存 buffer 写满重绕时，帧信息数组 FrmList 没有写满（最大存储帧数量不匹配）不会
	产生任何影响，只要保证好 buffer 在重绕踩到“读指针”时进行“跳帧操作”就好了，因为这样
	“写指针”踩的永远是已经过时的帧数据，不会对接下来要读的帧数据产生影响。	
	*/
    HLE_U8 resetReadPos = false; //标记读指针是否被覆盖
    if(cBuf->writePos + writableLen < cBuf->bufSize) //1.写入这次数据后 circle buffer 不会写到最尾部
    {
       //memcpy(&cBuf->bufStart[cBuf->writePos + 1], pSrc, writableLen);
		memcpy(cBuf->bufStart + cBuf->writePos,pSrc,writableLen);
        if((cBuf->writePos < cBuf->readPos) && (cBuf->writePos + writableLen >= cBuf->readPos))
            resetReadPos = true;
        cBuf->writePos += writableLen;
    }
    else//2.写入这次数据后 circle buffer 会写到最尾部（溢出）（需要重新循环）
    {
    	#if 0  //不舍弃尾部这部分缓存，在读帧数据时就需要判断是否为 buffer 的最后一帧，要拷贝两次才能读完
	    	//先写尾巴上的这部分
	        HLE_U32 remainSize = cBuf->bufSize - cBuf->writePos; //the remain size
			memcpy(cBuf->bufStart + cBuf->writePos,pSrc,remainSize);
			
			//再写剩余的数据到buf头（会覆盖之前头部的数据）
	        HLE_U32 coverSize = writableLen - remainSize; //size of data to be covered from the beginning
	        memcpy(cBuf->bufStart, pSrc+remainSize, coverSize);
        #else //舍弃尾部这部分缓存，重新在开头处写，这样每次读帧都不需要判断是否为 buffer 的最后一帧，一次拷贝就能读完
			memcpy(cBuf->bufStart,pSrc,writableLen);
		
		#endif
		
        if(cBuf->writePos < cBuf->readPos)// ----------w-----r------>该情况下发生的重绕，r必被踩，需要进行跳帧
            resetReadPos = true;
        else 	//----------r[w]-----w------>该情况下发生的重绕,只有写入的数据长度重绕后踩到r才需要进行重置r（进行跳帧）
        {
            if(writableLen>cBuf->readPos)
                resetReadPos = true;
        }
        cBuf->writePos = writableLen;
		
    }

	/*---# 跳帧操作（数据 buffer 读指针被踩，需要重置）------------------------------------------------------------*/
    if(resetReadPos)
    {
		//“读指针” 指向 “写指针” 的下一个I帧位置
		HLE_U32 diff_I_num = 0;
		HLE_U32 IFrmIndex_r_next_pos = cBuf->IFrmIndex_w + 1;
		if(IFrmIndex_r_next_pos >= MAX_I_F_NUM)//----------------------r---w> r被踩后的情况
		{
			IFrmIndex_r_next_pos = 0;
			diff_I_num = (MAX_I_F_NUM - 1) + 1 - cBuf->IFrmIndex_r;
		}
		else //---------------r---w---> r被踩后的情况
		{
			diff_I_num = IFrmIndex_r_next_pos - cBuf->IFrmIndex_r;
		}
		//1.IFrmIndex_r 跳转到读指针 IFrmIndex_w 的下一个I帧位置的下标处
		cBuf->IFrmIndex_r = IFrmIndex_r_next_pos;
		
		//2.FrmList_r 跳转到 帧信息数组（IFrmIndex）的下一个I帧位置
		cBuf->FrmList_r = cBuf->IFrmIndex[IFrmIndex_r_next_pos];
		
		//3.帧数据缓存buffer的读指针 readPos 跳转到该FrmList_r对应帧的偏移位置 
		cBuf->readPos = cBuf->FrmList[cBuf->FrmList_r].frmStartPos;
		
		cBuf->totalIFrm -= diff_I_num;
		cBuf->occupiedSize = cBuf->bufSize;//"帧缓存buffer的读指针" 被踩表明 buffer 里边已经存满了数据，占用已经达到最大值cBuf->size
     
    }
    else
    {
        if(cBuf->writePos >= cBuf->readPos) //----r[w]----------w---------->
            cBuf->occupiedSize = cBuf->writePos - cBuf->readPos;
        else	//---w-----r----------> 
            cBuf->occupiedSize = cBuf->bufSize- (cBuf->readPos - cBuf->writePos);
    }

	return 0;
	
}

/*******************************************************************************
*@ Description    :从循环buffer中获取一帧数据
*@ Input          :<cBuf>缓存buffer指针
*@ Output         :<dataOut>帧首地址
					<length帧数据长度
*@ Return         :成功：0
					失败：-1
*@ attention      :
*******************************************************************************/
HLE_S32 CircularBufferReadOneFrame(CircularBuffer_t * cBuf, void **dataOut, HLE_S32 *length)
{
	if(NULL == cBuf ||NULL == dataOut || NULL == length)
	{
		CBUF_ERROR_LOG("Illegal parameter!\n");
		return -1;
	}

	int wait_count = 0;
	while(cBuf->occupiedSize == 0)//无数据可读
	{
		usleep(3333);//该时间只是保证正常情况下肯定能有新帧产生
		wait_count++;
		if(wait_count >=30)//最多等待一秒（大约）
		{
			CBUF_ERROR_LOG("CircularBufferReadOneFrame time out!\n");
			return -1;
		}
	}        
    
   
	FrameInfo_t FrameInfo = cBuf->FrmList[cBuf->FrmList_r];
	cBuf->readPos = FrameInfo.frmStartPos;
	*dataOut = (void*)(cBuf->bufStart + FrameInfo.frmStartPos);
	*length = FrameInfo.frmLength;
	cBuf->occupiedSize -= FrameInfo.frmLength;
	
	cBuf->FrmList_r ++;
	if(cBuf->FrmList_r >= MAX_FRM_NUM)
		cBuf->FrmList_r = 0;
	cBuf->totalFrm --;

	if(FrameInfo.flag == 1)//为I帧
	{
		cBuf->IFrmIndex_r = (cBuf->IFrmIndex_r + 1 >= MAX_I_F_NUM) ? 0 : (cBuf->IFrmIndex_r + 1);
		cBuf->totalIFrm --;
	}
		
   
	return 0;
}






