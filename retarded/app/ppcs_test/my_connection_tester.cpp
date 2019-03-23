//// ConnectionTester.cpp 
//// Author: Zheng.B.C
////	To test PPCS connection with a specified DID device, from an Internet Host
////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#ifdef LINUX
#include <unistd.h> 
#include <fcntl.h>
#include <pthread.h> 
#include <sys/types.h>
#include <signal.h> 
#include <netinet/in.h>
#include <netdb.h> 
#include <net/if.h>
#include <sched.h>
#include <stdarg.h>		// va_start ...
#include <dirent.h>
#include <arpa/inet.h> 	// INADDR_ANY, htonl, htons, ...
#endif
#if defined(WIN32DLL) || defined(WINDOWS)
//#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#endif

#include "PPCS_API.h"
#include "PPCS_Error.h"
#include "PPCS_Type.h"
#include "./media_server_signal_def.h"
#include "typeport.h"

#define ERROR_NotLogin				-1
#define ERROR_InvalidParameter		-2	
#define ERROR_SocketCreateFailed	-3	
#define ERROR_SendToFailed			-4	
#define ERROR_RecvFromFailed		-5	
#define ERROR_UnKnown				-99

#define SERVER_NUM			3	// WakeUp Server Number
#define THREAD_NUM			3 //3	// Connect Thread Number
#define SIZE_DID 			30
#define SIZE_INITSTRING 	256
#define SIZE_WAKEUP_KEY 	17

#if defined(WIN32DLL) || defined(WINDOWS)
typedef	 	DWORD					my_threadid;
typedef		HANDLE	 				my_Thread_mutex_t;
#define		my_Mutex_Lock(mutex) 	WaitForSingleObject(mutex, INFINITE) 
#define		my_Mutex_UnLock(mutex) 	ReleaseMutex(mutex) 
#define		my_Thread_exit(a)  		return(a)
#elif defined(LINUX) 
typedef 	pthread_t				my_threadid;
typedef		pthread_mutex_t  		my_Thread_mutex_t;
#define		my_Mutex_Lock(mutex)	pthread_mutex_lock(&mutex)
#define		my_Mutex_UnLock(mutex)	pthread_mutex_unlock(&mutex)
#define		my_Thread_exit(a)		pthread_exit(a) 		
#endif

// show info 开关 -> 终端打印调试信息
static int g_ST_INFO_OPEN = 1;
// debug log 开关 -> 输出到本地log文件
static int g_DEBUG_LOG_FILE_OPEN = 0;
const char *LogFileName = "./ConnectionTester.log";

// 63->Lan, 60->P2P, 94->RLY 
char bEnableLanSearch[THREAD_NUM] = {63, 60, 94}; 

#define BMODE(i)	(63==bEnableLanSearch[i])?"LAN":((60==bEnableLanSearch[i])?"P2P":"RLY")

static int g_Session = -99;		// Save the fastest connection session value  

static int gArgc = 0;
my_Thread_mutex_t	thread_mutex_Conn;

// this struct is used for mode=7
// Connect success: get one returned value of Three threads connect at the same time 
// Three Threads Connect failed: to save the error code value of the P2P thread connection
typedef struct 
{
	int index;
	int ConnectRet;
	int CheckRet;
	st_PPCS_Session Sinfo;
	unsigned long Tick_Begin_mSec;
	unsigned long Tick_End_mSec;
	char bEnableLanSearch;
} st_Conn_Info;
st_Conn_Info gConnInfo;

#define ST_TIME_USED	(int)(TimeEnd.TimeTick_mSec-TimeBegin.TimeTick_mSec)
typedef struct{
	int Year;
	int Mon;
	int Day;
	int Week;
	int Hour;
	int Min;
	int Sec;
	int mSec;
	unsigned long TimeTick_mSec;
} st_Time_Info;

void my_GetCurrentTime(st_Time_Info *Time)
{
#if  defined(WINDOWS) || defined(WIN32DLL)
	SYSTEMTIME mTime = {0};
	GetLocalTime(&mTime);
	Time->Year = mTime.wYear;
	Time->Mon = mTime.wMonth;
	Time->Day = mTime.wDay;
	Time->Week = mTime.wDayOfWeek;
	Time->Hour = mTime.wHour;
	Time->Min = mTime.wMinute;
	Time->Sec = mTime.wSecond;
	Time->mSec = mTime.wMilliseconds;
	Time->TimeTick_mSec = GetTickCount();
#elif defined(LINUX)
	struct timeval mTime;
	int ret = gettimeofday(&mTime, NULL);
	if (0 != ret)
	{
		printf("gettimeofday failed!! errno=%d\n", errno);
		memset(Time, 0, sizeof(st_Time_Info));
		return ;
	}
	//struct tm *ptm = localtime((const time_t *)&mTime.tv_sec); 
	struct tm st_tm = {0};
	struct tm *ptm = localtime_r((const time_t *)&mTime.tv_sec, &st_tm); 
	if (!ptm)
	{
		printf("localtime_r failed!!\n");
		memset(Time, 0, sizeof(st_Time_Info));
		Time->TimeTick_mSec = mTime.tv_sec*1000 + mTime.tv_usec/1000;
	}
	else
	{
		Time->Year = st_tm.tm_year+1900;
		Time->Mon = st_tm.tm_mon+1;
		Time->Day = st_tm.tm_mday;
		Time->Week = st_tm.tm_wday;
		Time->Hour = st_tm.tm_hour;
		Time->Min = st_tm.tm_min;
		Time->Sec = st_tm.tm_sec;
		Time->mSec = (int)(mTime.tv_usec/1000);
		Time->TimeTick_mSec = mTime.tv_sec*1000 + mTime.tv_usec/1000;
	}
#endif
}

unsigned long getTickCount(void)
{
#if defined(WIN32DLL) || defined(WINDOWS) 
	return GetTickCount();
#elif defined(LINUX)
	struct timeval current;
	gettimeofday(&current, NULL);
	return current.tv_sec*1000 + current.tv_usec/1000;
#endif
}

void st_info(const char *format, ...) 
{
	//if (1 == g_ST_INFO_OPEN) 
	//{
		va_list ap;
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		va_end(ap);
	//}
	if (1 == g_DEBUG_LOG_FILE_OPEN) 
	{
		FILE *pFile = fopen(LogFileName, "a");
		if (!pFile)
		{
			fprintf(stderr, "Error: Can not Open %s file!\n", LogFileName);
			return ;
		}
		va_list ap;
		va_start(ap, format);
		vfprintf(pFile, format, ap);
		va_end(ap);
		fclose(pFile);
	}
}
void st_debug(const char *format, ...) 
{
	if (1 == g_ST_INFO_OPEN) 
	{
		va_list ap;
		va_start(ap, format);
		vfprintf(stderr, format, ap);
		va_end(ap);
	}
	if (1 == g_DEBUG_LOG_FILE_OPEN) 
	{
		FILE *pFile = fopen(LogFileName, "a");
		if (!pFile)
		{
			fprintf(stderr, "Error: Can not Open %s file!\n", LogFileName);
			return ;
		}
		va_list ap;
		va_start(ap, format);
		vfprintf(pFile, format, ap);
		va_end(ap);
		fclose(pFile);
	}
}

void mSleep(UINT32 ms)
{
#if defined(WIN32DLL) || defined(WINDOWS)
	Sleep(ms);
#elif defined LINUX
	usleep(ms * 1000);
#endif
}

void error(const char *msg) 
{
    perror(msg);
    exit(0);
}

const char *getP2PErrorCodeInfo(int err)
{
    if (0 < err) 
	{
		return "NoError";
	}		
    switch (err)
    {
	case 0: return "ERROR_P2P_SUCCESSFUL";
	case -1: return "ERROR_P2P_NOT_INITIALIZED";
	case -2: return "ERROR_P2P_ALREADY_INITIALIZED";
	case -3: return "ERROR_P2P_TIME_OUT";
	case -4: return "ERROR_P2P_INVALID_ID";
	case -5: return "ERROR_P2P_INVALID_PARAMETER";
	case -6: return "ERROR_P2P_DEVICE_NOT_ONLINE";
	case -7: return "ERROR_P2P_FAIL_TO_RESOLVE_NAME";
	case -8: return "ERROR_P2P_INVALID_PREFIX";
	case -9: return "ERROR_P2P_ID_OUT_OF_DATE";
	case -10: return "ERROR_P2P_NO_RELAY_SERVER_AVAILABLE";
	case -11: return "ERROR_P2P_INVALID_SESSION_HANDLE";
	case -12: return "ERROR_P2P_SESSION_CLOSED_REMOTE";
	case -13: return "ERROR_P2P_SESSION_CLOSED_TIMEOUT";
	case -14: return "ERROR_P2P_SESSION_CLOSED_CALLED";
	case -15: return "ERROR_P2P_REMOTE_SITE_BUFFER_FULL";
	case -16: return "ERROR_P2P_USER_LISTEN_BREAK";
	case -17: return "ERROR_P2P_MAX_SESSION";
	case -18: return "ERROR_P2P_UDP_PORT_BIND_FAILED";
	case -19: return "ERROR_P2P_USER_CONNECT_BREAK";
	case -20: return "ERROR_P2P_SESSION_CLOSED_INSUFFICIENT_MEMORY";
	case -21: return "ERROR_P2P_INVALID_APILICENSE";
	case -22: return "ERROR_P2P_FAIL_TO_CREATE_THREAD";
	default: return "Unknown, something is wrong!";
    }
}

void showErrorInfo(int ret)
{
	if (0 <= ret) 
	{
		return ;
	}
	switch (ret)
	{
	case ERROR_PPCS_NOT_INITIALIZED:
		st_info("API didn't initialized\n"); 
		break;		
	case ERROR_PPCS_TIME_OUT:
		st_info("Connection time out, probably the device is off line now\n"); 
		break;			
	case ERROR_PPCS_INVALID_ID:
		st_info("Invalid Device ID\n"); 
		break;
	case ERROR_PPCS_INVALID_PREFIX:
		st_info("Prefix of Device ID is not accepted by Server\n"); 
		break;
	case ERROR_PPCS_DEVICE_NOT_ONLINE:
		st_info("Device is not on line now, nor login in the past 5 minutes\n"); 
		break;
	case ERROR_PPCS_NO_RELAY_SERVER_AVAILABLE:
		st_info("No relay server can provide service\n"); 
		break;
	case ERROR_PPCS_SESSION_CLOSED_REMOTE:
		st_info("Session close remote.\n"); 
		break;
	case ERROR_PPCS_SESSION_CLOSED_TIMEOUT:
		st_info("Session close timeout.\n"); 
		break;
	case ERROR_PPCS_MAX_SESSION:
		st_info("Exceed max Session\n"); 
		break;
	case ERROR_PPCS_UDP_PORT_BIND_FAILED:
		st_info("The specified UDP port can not be binded\n"); 
		break;
	case ERROR_PPCS_USER_CONNECT_BREAK:
		st_info("connect break is called\n"); 
		break;
	default: st_info("%s\n", getP2PErrorCodeInfo(ret));
		break;
	} // switch	
}

void showNetwork(st_PPCS_NetInfo NetInfo)
{
	st_info("-------------- NetInfo: -------------------\n");
	st_info("Internet Reachable     : %s\n", (NetInfo.bFlagInternet == 1) ? "YES":"NO");
	st_info("P2P Server IP resolved : %s\n", (NetInfo.bFlagHostResolved == 1) ? "YES":"NO");
	st_info("P2P Server Hello Ack   : %s\n", (NetInfo.bFlagServerHello == 1) ? "YES":"NO");
	switch(NetInfo.NAT_Type)
	{
	case 0: st_info("Local NAT Type         : Unknow\n"); break;
	case 1: st_info("Local NAT Type         : IP-Restricted Cone\n"); break;
	case 2: st_info("Local NAT Type         : Port-Restricted Cone\n"); break;
	case 3: st_info("Local NAT Type         : Symmetric\n"); break;
	}
	st_info("My Wan IP : %s\n", NetInfo.MyWanIP);
	st_info("My Lan IP : %s\n", NetInfo.MyLanIP);
	st_info("-------------------------------------------\n");
}

int iPN_StringEnc(const char *keystr, const char *src, char *dest, unsigned int maxsize)
{
	int Key[17] = {0};
	unsigned int i;
	unsigned int s, v;
	if (maxsize < strlen(src) * 2 + 3)
	{
		return -1;
	}
	for (i = 0 ; i < 16; i++)
	{
		Key[i] = keystr[i];
	}
	srand((unsigned int)time(NULL));
	s = abs(rand() % 256);
	memset(dest, 0, maxsize);
	dest[0] = 'A' + ((s & 0xF0) >> 4);
	dest[1] = 'a' + (s & 0x0F);
	for (i = 0; i < strlen(src); i++)
	{
		v = s ^ Key[(i + s * (s % 23))% 16] ^ src[i];
		dest[2 * i + 2] = 'A' + ((v & 0xF0) >> 4);
		dest[2 * i + 3] = 'a' + (v & 0x0F);
		s = v;
	}
	return 0;
}

int iPN_StringDnc(const char *keystr, const char *src, char *dest, unsigned int maxsize)
{
	int Key[17] = {0};
	unsigned int i;
	unsigned int s, v;
	if ((maxsize < strlen(src) / 2) || (strlen(src) % 2 == 1))
	{
		return -1;
	}
	for (i = 0 ; i < 16; i++)
	{
		Key[i] = keystr[i];
	}	
	memset(dest, 0, maxsize);
	s = ((src[0] - 'A') << 4) + (src[1] - 'a');
	for (i = 0; i < strlen(src) / 2 - 1; i++)
	{
		v = ((src[i * 2 + 2] - 'A') << 4) + (src[i * 2 + 3] - 'a');
		dest[i] = v ^ Key[(i + s * (s % 23))% 16] ^ s;
		if (dest[i] > 127 || dest[i] < 32) 
		{
			return -1; // not a valid character string
		}	
		s = v;
	}
	return 0;
}

//// ret=0 OK, ret=-1: Invalid Parameter, ret=-2: No such Item
int GetStringItem(	const char *SrcStr, 
					const char *ItemName, 
					const char Seperator, 
					char *RetString, 
					const int MaxSize)
{
    if (!SrcStr || !ItemName || !RetString || 0 == MaxSize)
	{
		return -1;
	}		
	const char *pFand = SrcStr;
    while (1)
    {
        pFand = strstr(pFand, ItemName);
        if (!pFand) 
		{
			return -2;
		}			
        pFand += strlen(ItemName);
        if ('=' != *pFand)
		{
			continue;  
		}   
        else 
		{
			break;
		}			 
    }
    pFand += 1;
    int i = 0;
    while (1)
    {
        if (Seperator == *(pFand + i) 
			|| '\0' == *(pFand + i) 
			|| i >= (MaxSize - 1))
		{
			break;
		}			
        else 
		{
			*(RetString + i) = *(pFand + i);   
		}		 
        i++;
    }
    *(RetString + i) = '\0';
    return 0;
}

int getMinNumFromLastLogin(const int pLastLogin[], const unsigned char Length)
{
	if (!pLastLogin)
	{
		//printf("getMinNumFromLastLogin - Invalid parameter!!\n");
		return ERROR_UnKnown;
	}
	int min = pLastLogin[0];
	for (int i = 0; i < Length; i++)
	{
		//printf("pLastLogin[%d]=%d\n", i, pLastLogin[i]);
		if (0 > pLastLogin[i]) 		// LastLogin<0: -99 or -1. min: -1,-99 or >= 0
		{
			if (min < pLastLogin[i])// min<0:-1,-99
			{
				min = pLastLogin[i];// min:-1
			}				
		}
		else if (0 > min || min > pLastLogin[i])// LastLogin>=0, min: unknown
		{
			min = pLastLogin[i];	// min>=0
		}			
	}
	return min;	
}

int WakeUp_Query(const struct sockaddr_in *address, 
				const unsigned char NumberOfServer,
				const char *Cmd, 
				const char *WakeupKey,
				const int tryCount,
				const unsigned int timeout_ms,
				int *LastLogin1,
				int *LastLogin2,
				int *LastLogin3)
{
	if (!address)
	{
		st_debug("Invalid address!!\n");
		return ERROR_InvalidParameter;
	}
	if (0 == NumberOfServer)
	{
		st_debug("Invalid NumberOfServer!! NumberOfServer=0\n");
		return ERROR_InvalidParameter;
	}
	if (!WakeupKey || 0 == strlen(WakeupKey))
	{
		st_debug("Invalid WakeupKey!!\n");
		return ERROR_InvalidParameter;
	}
	if (!Cmd || 0 == strlen(Cmd))
	{
		st_debug("Invalid CMD!!\n");
		return ERROR_InvalidParameter;
	}			
	
	struct sockaddr_in myAddr[SERVER_NUM];
	struct sockaddr_in fromAddr;
	memset(myAddr, 0, sizeof(myAddr));
	memset(&fromAddr, 0, sizeof(fromAddr));
	unsigned int sin_len = sizeof(struct sockaddr_in);
	
	char dest[20];
	memset(dest, 0, sizeof(dest));
	for (int i = 0; i < NumberOfServer; i++)
	{
		memcpy((char*)&myAddr[i], (char*)&address[i], sin_len);
		//st_debug("%d-%s:%d\n", i, inet_ntop(myAddr[i].sin_family, (char *)&myAddr[i].sin_addr.s_addr, dest, sizeof(dest)), ntohs(myAddr[i].sin_port), ntohs(myAddr[i].sin_port));
	}
	st_debug("\n");
	
	char CMD[60];
	memset(CMD, 0, sizeof(CMD));
	memcpy(CMD, Cmd, strlen(Cmd));
	
	int flag[SERVER_NUM];
	memset(&flag, 0, sizeof(flag));
	int LastLogin[SERVER_NUM];
	for (int i = 0; i < SERVER_NUM; i++)
	{
		LastLogin[i] = ERROR_UnKnown;
	}
		
	int count = 0;
	int counter = 0;
	int timeOutCount = 0;
	int size;
	fd_set readfds;
	struct timeval timeout;
	char recvBuf[256];
	char Message[128];
	
	// 创建 UDP socket
	int skt = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 > skt)
	{
		st_debug("create UDP socket failed\n");
		return ERROR_SocketCreateFailed;
	}
	
	while (tryCount > timeOutCount)
	{
		count = 0;
		counter = 0;
		for (int i = 0; i < NumberOfServer; i++)
		{	
			memset(dest, 0, sizeof(dest));
			//inet_ntop(myAddr[i].sin_family, (char *)&myAddr[i].sin_addr.s_addr, dest, sizeof(dest));
			//if (0 == strcmp(dest, "0.0.0.0"))
			if (0 == strcmp(inet_ntoa(myAddr[i].sin_addr), "0.0.0.0"))
			{
				counter++;
				continue;
			}				
			if (0 == flag[i])
			{
				size = sendto(skt, CMD, strlen(CMD), 0, (struct sockaddr *)&myAddr[i], sizeof(struct sockaddr_in));
				if (0 > size)
				{
					st_debug("Sendto Error!\n");
					close(skt);
					return ERROR_SendToFailed;
				}
				st_debug("%d-Send CMD(%u Byte) to Wakeup_Server-%d %s:%d\n", 
						i, 
						(unsigned)strlen(CMD), 
						i, 
						inet_ntoa(myAddr[i].sin_addr), 
						ntohs(myAddr[i].sin_port));
			}
			else if (1 == flag[i])
			{
				count++;
				st_debug("%d-Done LastLogin=%d\n", i, LastLogin[i]);	
			}	
		} // for
		if (NumberOfServer == counter || NumberOfServer == count)
		{
			break;
		}
		st_debug("\n");
		
		FD_ZERO(&readfds);
		FD_SET(skt, &readfds);
		timeout.tv_sec = (timeout_ms-(timeout_ms%1000))/1000;
		timeout.tv_usec = (timeout_ms%1000)*1000;
		
		int result = select(skt + 1, &readfds, (fd_set *)NULL, (fd_set *)NULL, &timeout);
		
		switch (result)
		{
		case 0: st_debug("-------------(timeout, Counter=%d)", timeOutCount++);	break;
		case -1:
			{
				st_debug("select error!\n");
				close(skt);
				return getMinNumFromLastLogin(LastLogin, SERVER_NUM);
			}
		default:
			if (FD_ISSET(skt, &readfds)) 
			{
				memset(recvBuf, 0, sizeof(recvBuf));
				memset(Message, 0, sizeof(Message));
				
				size = recvfrom(skt, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&fromAddr, (socklen_t*)&sin_len);
				if (0 > size)  
				{  
					st_debug("WakeUp_Query-RecvFrom error");  
					close(skt);
					return ERROR_RecvFromFailed;
				}
				recvBuf[size] = '\0';
				st_debug("recv data: %s, Size: %u Byte\n", recvBuf, (unsigned)strlen(recvBuf));
				
				if (0 > iPN_StringDnc(WakeupKey, recvBuf, Message, sizeof(Message))) 
				{
					st_debug("WakeUp_Query-iPN_StringDnc failed.\n");
					close(skt);
					return getMinNumFromLastLogin(LastLogin, SERVER_NUM);
				}
				
				counter = 0;
				for (int i = 0; i < NumberOfServer; i++)
				{
					if (fromAddr.sin_addr.s_addr == myAddr[i].sin_addr.s_addr && fromAddr.sin_port == myAddr[i].sin_port)
					{
						//st_debug("%d-Recv %s, Data: %s\n", i, inet_ntop(fromAddr.sin_family, (char *)&fromAddr.sin_addr.s_addr, dest, sizeof(dest)), Message);
						st_debug("%d-Recv %s, Data: %s\n", i, inet_ntoa(fromAddr.sin_addr), Message);
						
						int lastLogin = ERROR_UnKnown;
						char buf[8];
						memset(buf, 0, sizeof(buf));
						if (0 > GetStringItem(Message, "LastLogin", '&', buf, sizeof(buf)))
						{
							st_debug("can not get LastLogin Item!\n");
						}
						else if (0 != strlen(buf))
						{
							lastLogin = atoi(buf);
						}
						flag[i] = 1;
						LastLogin[i] = lastLogin;
					}
					if (1 == flag[i])
					{
						counter++;
					}
				} // for
			}
			else 
			{
				st_debug("FD_ISSET error, readfds no data!!\n");
			} 				
		} // switch
		if (NumberOfServer == counter)
		{
			break; // break while
		}
		st_debug("\n");
	} // while
	close(skt);
		
	int MinLastLogin = getMinNumFromLastLogin(LastLogin, SERVER_NUM);
	st_debug("\n** LastLogin[");
	for (int i = 0; i < SERVER_NUM; i++)
	{
		st_debug("%d%c", LastLogin[i], (i < SERVER_NUM-1)?',':']');
	}	
	st_debug(", Result: LastLogin= %d %s.\n", MinLastLogin, (0<=MinLastLogin)? "sec before":((-1==MinLastLogin)?"NotLogin":"UnKnown"));
	
	//st_debug("\n** LastLogin[%d, %d, %d], Result: LastLogin = %d %s\n", LastLogin[0],LastLogin[1], LastLogin[2], MinLastLogin, (0<=MinLastLogin)? "sec before":((-1==MinLastLogin)?"NotLogin":"UnKnown"));
	st_debug("Value: [>=0]: LastLogin Second, [-1]: NotLogin, [%d]: LastLogin UnKnown.\n\n", ERROR_UnKnown);
	
	if (NULL != LastLogin1) *LastLogin1 = LastLogin[0];
	if (NULL != LastLogin2) *LastLogin2 = LastLogin[1];
	if (NULL != LastLogin3) *LastLogin3 = LastLogin[2];
	
	return MinLastLogin;
}



void showArg(CHAR **argv)
{
	printf("Usage: %s Mode DID InitString [Repeat] [DelaySec] [WakeupKey] [IP1] [IP2] [IP3]\n", argv[0]);
	printf("\tMode: 0->No local LAN search, P2P then Relay for remote.\n");
	printf("\t      1->Local LAN search, P2P then Relay for remote.\n");
	printf("\t      2->No local LAN search, force Relay for remote.\n");
	printf("\t      3->Local LAN search, force Relay for remote.\n");
	printf("\t      4->Do Network Detect Only.\n");
	printf("\t      5->Dedicated connection mode for wakeup devices.\n");
	printf("\t      6->No local LAN search, force Server Relay for remote.\n");
	printf("\t      7->Three Thread Mode(LAN,P2P,RLY) to Connect.\n");
	printf("\tRepeat: Connection Times (eg: 100), If not specified, connect once.\n");
	printf("\tDelaySec: Delay time in sec.(eg: 100), If not specified, default DelaySec=3 sec, minimum DelaySec is 1 sec,if you set 0, the program will reset it to 1.\n");
	printf("\tWakeupKey: Wakeup Server String Key.\n");
	printf("\t[IP1],[IP2],[IP3]: Three Wakeup Server IP or domain name.\n");
}

/*
使用：./ConnectionTester64    7 RTOS-000236-STWDB EFGBFFBJKEJKGGJJEEGFFHELHHNNHONHGLFNBHCCAEJDLNLPDDAGCIOFGDLGJMLAAOMOKCDLOONOBICJJIMM 1 3 1234567890ABCDEF 112.74.108.149
	printf("Usage: %s Mode DID InitString [Repeat] [DelaySec] [WakeupKey] [IP1] [IP2] [IP3]\n", argv[0]);
*/


//advanced p2p 参数（支持唤醒.适用RTOS系统,LiteOS属于RTOS系统）
#define _DID_ 				"RTOS-000236-STWDB"
#define APILICENSE 			"ATTWPQ"
#define CRCKEY 				"EasyView"
#define INITSTRING 			"EFGBFFBJKEJKGGJJEEGFFHELHHNNHONHGLFNBHCCAEJDLNLPDDAGCIOFGDLGJMLAAOMOKCDLOONOBICJJIMM "
#define WAKEUPKEY 			"1234567890ABCDEF"			
#define IP_LENGTH			16
#define TCP_PORT			12306
#define UDP_PORT			12305
#define SERVER_IP1			"112.74.108.149"
#define SERVER_IP2			"112.74.108.149"
#define SERVER_IP3			"112.74.108.149"

#define CH_CMD				0	//信令传输通道（P2P）
#define CH_STREAM			1	//编码流传输通道（P2P）
#define CH_DATA				2	//其他数据传输通道（P2P）



//音视频帧头标志

typedef struct
{
    HLE_U8 sync_code[3]; /*帧头同步码，固定为0x00,0x00,0x01*/
    HLE_U8 type; /*帧类型, 0xF8-视频关键帧，0xF9-视频非关键帧，0xFA-音频帧*/
} FRAME_HDR;

//关键帧描述信息
typedef struct
{
    HLE_U8 enc_std; //编码标准，具体见E_VENC_STANDARD
    HLE_U8 framerate; //帧率
    HLE_U16 reserved;
    HLE_U16 pic_width;
    HLE_U16 pic_height;
    HLE_SYS_TIME rtc_time; //当前帧时间戳，精确到秒，非关键帧时间戳需根据帧率来计算
    HLE_U32 length;
    HLE_U32 pts_msec; //毫秒级时间戳，一直累加，溢出后自动回绕
} IFRAME_INFO;

//非关键帧描述信息
typedef struct
{
    HLE_U32 pts_msec; //毫秒级时间戳，一直累加，溢出后自动回绕
    HLE_U32 length;
} PFRAME_INFO;

//音频帧描述信息
typedef struct
{
    HLE_U8 enc_type; //音频编码类型，具体见E_AENC_STANDARD
    HLE_U8 sample_rate; //采样频率，具体见E_AUDIO_SAMPLE_RATE
    HLE_U8 bit_width; //采样位宽，具体见E_AUDIO_BIT_WIDTH
    HLE_U8 sound_mode; //单声道还是立体声，具体见E_AUDIO_SOUND_MODE
    HLE_U32 length;
    HLE_U32 pts_msec;
} AFRAME_INFO;


/*
关闭实时流传输命令
*/
int send_cmd_close_living(void)
{
	char * buf = (char*)malloc(sizeof(STRUCT_CLOSE_LIVING_REQUEST));
	memset(buf,0,sizeof(STRUCT_CLOSE_LIVING_REQUEST));

	//填充CMD头部
	cmd_header_t* mycmd =(cmd_header_t*) buf;
	mycmd->command = CMD_CLOSE_LIVING;
	mycmd->head = HLE_MAGIC;
	mycmd->type = 1;
	mycmd->length = sizeof(STRUCT_CLOSE_LIVING_REQUEST)-sizeof(cmd_header_t);

	//填充CMD body部
	//STRUCT_CLOSE_LIVING_REQUEST *cmd_body = (STRUCT_CLOSE_LIVING_REQUEST*)buf;

	
	/*
	发送命令   
	PPCS_write 发送数据可以区分不同channel，有专门的命令channel，有专门的码流channel，这样不容易混淆，造成数据错乱
	注意：  	channel 是CH_CMD
	*/
	int ret = PPCS_Write(g_Session, CH_CMD, (char*)mycmd, sizeof(STRUCT_CLOSE_LIVING_REQUEST));
	if (ret < 0)
	{
		st_info("PPCS_Write: gSessionID=%d, CH=%d, ret=%d [%s]\n", g_Session, 0, ret, getP2PErrorCodeInfo(ret));
		return -1;
	}
	else // Select the test options according to the Mode
	{	
		printf("PPCS_Write CMD_CLOSE_LIVING  ok !");

	}
	
	free(buf);
	return 0;
	
}


INT32 main(INT32 argc, CHAR **argv)
{	
	// 1. get P2P API Version
	UINT32 APIVersion = PPCS_GetAPIVersion();
	st_info("P2P API Version: %d.%d.%d.%d\n",
				(APIVersion & 0xFF000000)>>24, 
				(APIVersion & 0x00FF0000)>>16, 
				(APIVersion & 0x0000FF00)>>8, 
				(APIVersion & 0x000000FF)>>0);
	
	
	char DID[SIZE_DID];
	char InitString[SIZE_INITSTRING];
	char WakeupKey[SIZE_WAKEUP_KEY];
	struct sockaddr_in serveraddr[SERVER_NUM];
	memset(DID, 0, sizeof(DID));
	memset(InitString, 0, sizeof(InitString));
	memset(WakeupKey, 0, sizeof(WakeupKey));
	memset(serveraddr, 0, sizeof(serveraddr));
	
	int DelaySec = 3;

	gArgc = argc;

	strcpy(DID,_DID_);
	strcpy(InitString,INITSTRING);
	strcpy(WakeupKey,WAKEUPKEY);

	serveraddr[1].sin_family = AF_INET;
	serveraddr[1].sin_port = htons(UDP_PORT);
	serveraddr[1].sin_addr.s_addr = inet_addr(SERVER_IP1);

	struct hostent *Host = gethostbyname(SERVER_IP1);
	if (!Host)
	{
		st_info("ERROR, no such host as %s\n", SERVER_IP1);
		perror("gethostbyname failed");
	}
	else
	{
		// build the server's Internet address
		serveraddr[1].sin_family = Host->h_addrtype;
		serveraddr[1].sin_port = htons(UDP_PORT);
		serveraddr[1].sin_addr.s_addr = *((unsigned int*)Host->h_addr_list[0]);
	}
		
	st_debug("Host:%s\n", inet_ntoa(serveraddr[1].sin_addr));
	st_debug("DID = %s\n", DID);
	st_debug("InitString = %s\n", InitString);
	st_debug("DelaySec = %d sec\n", DelaySec);	
	st_debug("WakeupKey = %s\n", WakeupKey);

	
	
	
	// 2. P2P 初始化
	st_debug("PPCS_Initialize(%s) ...\n", InitString);
	int  ret = PPCS_Initialize((char *)InitString);
	st_debug("PPCS_Initialize done! ret=%d\n", ret);
	if (ERROR_PPCS_SUCCESSFUL != ret && ERROR_PPCS_ALREADY_INITIALIZED != ret)
	{
		st_info("PPCS_Initialize failed!! ret=%d: %s\n", ret, getP2PErrorCodeInfo(ret));
		return 0;
	}
	
	// 3. 网络侦测
	st_PPCS_NetInfo NetInfo;
	ret = PPCS_NetworkDetect(&NetInfo, 0);
	if (0 > ret) 
	{
		st_info("PPCS_NetworkDetect failed: ret=%d\n", ret);
	}
	
	showNetwork(NetInfo);
		

	/**/

	int LastSleepLogin = ERROR_UnKnown;
	int LastLogin[3] = {ERROR_UnKnown, ERROR_UnKnown, ERROR_UnKnown};
	
	char CMD[30];
	memset(CMD, 0, sizeof(CMD));
	char CMD_Enc[60];

	snprintf(CMD, sizeof(CMD), "DID=%s&", DID);
	st_debug("Cmd: %s, size=%u Byte\n", CMD, (unsigned)strlen(CMD));
	
	// 加密查询指令
	if (0 > iPN_StringEnc(WakeupKey, CMD, CMD_Enc, sizeof(CMD_Enc))) 
	{
		st_info("***WakeUp Query Cmd StringEncode failed!\n");
		PPCS_DeInitialize();
		return 0;
	}
	st_debug("[%s] %u Byte -> [%s] %u Byte\n\n", CMD, (unsigned)strlen(CMD), CMD_Enc, (unsigned)strlen(CMD_Enc));

	
	#define TIME_USED	(t_End-t_Start)
	
	unsigned char NumberOfServer = 1;
	
	
			LastSleepLogin = WakeUp_Query(
									serveraddr, 
									NumberOfServer, 
									CMD_Enc, 
									WakeupKey, 
									1, 2000, //repeat=3, timeout=2sec, total timeout:3*2=6sec
									&LastLogin[0], 
									&LastLogin[1], 
									&LastLogin[2]);
			
			// -1: device not login, -99: UnKnown, other value<0: query failed
			if (0 > LastSleepLogin && -1 != LastSleepLogin && ERROR_UnKnown != LastSleepLogin)	
			{
				st_info("WakeUp_Query failed: %d\n", LastSleepLogin);
				PPCS_DeInitialize();
				return 0;
			}



		// if bEnableLanSearch = 0x7F --> Connect() is used to detect if Device is on-line
		ret = PPCS_Connect(DID, 60, 0);// 63->Lan, 60->P2P, 94->RLY 		
		if (ret < 0)//PPCS_Connect连接失败
		{
			st_debug("- PPCS_Connect failed : [%s]\n", getP2PErrorCodeInfo(ret));
		
		}
		else
		{
			st_debug("-------------------------PPCS_Connect success: %d\n", ret);

			st_debug("-------------------------PPCS_Check ...\n");
			st_PPCS_Session Sinfo;
			INT32 Check_Ret = PPCS_Check(ret, &Sinfo);
			st_debug("-------------------------PPCS_Check : %d\n", Check_Ret);
			if (ERROR_PPCS_SUCCESSFUL == Check_Ret)
			{
				st_debug("%s:%d [Success]\n", inet_ntoa(Sinfo.RemoteAddr.sin_addr), ntohs(Sinfo.RemoteAddr.sin_port));
			}
			else 
			{
				st_debug("Unknown (remote closed:%d)\n", Check_Ret);
			}
			
	
			g_Session = ret;

			memcpy(&gConnInfo.Sinfo, &Sinfo, sizeof(Sinfo));

			printf("into here 001\n");

				/****发送命令获取实时流********************************************/
				// 5. do job ...
				// 填充信令数据结构
				
				#if 0  //升级命令
				printf("into update !\n");
				char * buf = (char*)malloc(sizeof(STRUCT_SET_UPDATE_REQUEST));
				memset(buf,0,sizeof(STRUCT_SET_UPDATE_REQUEST));

				//填充CMD头部
				cmd_header_t* mycmd =(cmd_header_t*) buf;
				mycmd->command = CMD_SET_UPDATE;
				mycmd->head = HLE_MAGIC;
				mycmd->type = 1;
				mycmd->length = sizeof(STRUCT_SET_UPDATE_REQUEST)-sizeof(cmd_header_t);

				//填充CMD body部
				STRUCT_SET_UPDATE_REQUEST *cmd_body = (STRUCT_SET_UPDATE_REQUEST*)buf;
				char *packageVersion = "0.0.0.1";
				//URL[256];
				//char *url = "http://soft1.xzstatic.com/KMSpico_Install_DownZa.Cn.rar";
				char *url = "http://speedtest.tokyo.linode.com/100MB-tokyo.bin";
				strcpy((char*)cmd_body->packageVersion,(char*)packageVersion);
				strcpy((char*)cmd_body->URL,(char*)url);
				
				#endif	

				#if 1  //请求实时流命令
				printf("into update !\n");
				char * buf = (char*)malloc(sizeof(STRUCT_OPEN_LIVING_REQUEST));
				memset(buf,0,sizeof(STRUCT_OPEN_LIVING_REQUEST));

				//填充CMD头部
				cmd_header_t* mycmd =(cmd_header_t*) buf;
				mycmd->command = CMD_OPEN_LIVING;
				mycmd->head = HLE_MAGIC;
				mycmd->type = 1;
				mycmd->length = sizeof(STRUCT_OPEN_LIVING_REQUEST)-sizeof(cmd_header_t);

				//填充CMD body部
				STRUCT_OPEN_LIVING_REQUEST *cmd_body = (STRUCT_OPEN_LIVING_REQUEST*)buf;
				cmd_body->videoType = MAIN_STREAM;
				cmd_body->openAudio = 1;
				
				#endif	
				
				/*
				发送命令   
				PPCS_write 发送数据可以区分不同channel，有专门的命令channel，有专门的码流channel，这样不容易混淆，造成数据错乱
				注意：  	channel 是CH_CMD
				*/
				ret = PPCS_Write(g_Session, CH_CMD, (char*)mycmd, sizeof(STRUCT_OPEN_LIVING_REQUEST));
				if (ret < 0)
				{
					st_info("PPCS_Write: gSessionID=%d, CH=%d, ret=%d [%s]\n", g_Session, 0, ret, getP2PErrorCodeInfo(ret));
					goto ERR;
				}
				else // Select the test options according to the Mode
				{	
					printf("PPCS_Write ok ! need write(%ld)Bytes-----> actually write(%d)Bytes\n",sizeof(STRUCT_OPEN_LIVING_REQUEST),ret);
 
				}

				/*---接收编码流-----------------------------*/
				#define STREAM_BUF_SIZE (1024*1024*1)   //1M空间
				char*stream_buf = (char*)malloc(STREAM_BUF_SIZE);
				if(NULL == stream_buf)
				{
					perror("malloc failed !\n");
					goto ERR;
				}
				
				FILE* fd = fopen("./ppcs_recode_video.h264","wb+");
				if( NULL == fd)
				{
					perror("fopen error !\n");
					goto ERR;
				}
				
				#define MAX_RECODE_SIZE (1024*1024*1)  //最大录制的字节数，达到该值后自动发送“关闭实时流传输命令”给设备，并退出
				unsigned int count_size = 0;//记录已经写入文件的数据大小
				while(1)
				{
					int read_size = 1024*5;//每次读的字节数
					//注意：PPCS_Read 的返回值成功为0 ，实际读到的字节数会返回在传进去的变量ReadSize中。
					ret = PPCS_Read(g_Session, CH_STREAM , stream_buf,&read_size,2000);//最大等待延迟1000ms
					if(ret < 0)
					{
						perror("PPCS_Read failed !\n");
						goto ERR;
					}
					DEBUG_LOG("PPCS_Read (%d)Bytes\n",read_size);			
					ret = fwrite(stream_buf, 1, read_size , fd);
					if(ret != read_size)
					{
						perror("write file failed !\n");
						goto ERR;
					}
					count_size += ret;
					
					if(count_size >= MAX_RECODE_SIZE)
					{
						if(send_cmd_close_living() < 0)
						{
							perror("send_cmd_close_living failed !\n");
							goto ERR;
						}
						DEBUG_LOG("buf is full!  call close living .....\n");
						DEBUG_LOG("send_cmd_close_living success!\n");
						break;
					}
					
					
				}
				
				fclose(fd);
				
				/***********************************************/

		}
				
ERR:
	mSleep(50);
	PPCS_Close(g_Session);
	st_debug("-PPCS_Close(%d)\n", ret);


	//释放
	PPCS_DeInitialize();
	st_debug("PPCS_DeInitialize() done!!\n");

	return 0;
}






