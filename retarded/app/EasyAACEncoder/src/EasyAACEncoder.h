/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/* 
 * File:   EasyAACEncoder.h
 * Author: Wellsen@easydarwin.org
 *
 * Created on 2015年4月11日, 上午11:44
 */

#ifndef EasyAACEncoder_H
#define	EasyAACEncoder_H

#define HLE_MODIFY   //用于标记修改的地方

#include "audio_buffer.h"
#include "IDecodeToPcm.h"
#include "PcmToAac.h"

class G7ToAac
{
public:
    G7ToAac();
    virtual ~G7ToAac();
    
	bool init();
	bool init(InAudioInfo info);
    
    int aac_encode(unsigned char* inbuf, unsigned int inlen, unsigned char* outbuf, unsigned int* outlen);

private:
	int aac_encode_obj(unsigned char* inbuf, unsigned int inlen, unsigned char* outbuf, unsigned int* outlen );
    
	bool CreateDecodePcm();
	bool CreateEncodeAac();
	bool CreateBuffer();
private:        
    int nRet;
    int nTmp;		//PCM帧剩余的字节数（当缓存不足以放下一帧数据时，部分放入后还剩余的字节数）
    int nCount;	  	//pcm缓存中已存的数据字节数。
    int nStatus;  	//进行AAC编码的标志
    int nPCMRead;
 


    int m_nPCMBufferSize;
    unsigned char *m_pbPCMBuffer;

    unsigned long m_nMaxOutputBytes;
    unsigned char *m_pbAACBuffer;

    int m_nPCMSize;   				 //一帧pcm帧 长度
    unsigned char *m_pbPCMTmpBuffer; //一帧PCM帧缓存 buf

	unsigned char *m_pbG7FrameBuffer;
	unsigned long m_nG7FrameBufferSize;

    audio_buffer *m_audio_buffer_;
	//------
	InAudioInfo m_inAudioInfo;

	IDecodeToPcm* m_pDecodeToPcm;
	PcmToAac* m_pPCMToAAC;
};

#endif	/* EasyAACEncoder_H */




