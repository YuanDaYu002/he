 
/***************************************************************************
* @file:parameter.c 
* @author:   
* @date:  7,9,2019
* @brief:  
* @attention:参数文件保存+提取+解析相关函数
***************************************************************************/
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>




#include "parameter.h"

#define SYS_PARAM_FILE		"/jffs0/sys_param.config"
/*---# 系统参数缓存结构 ------------------------------------------------------------*/
static system_parameter_t 	g_sys_param;  
pthread_mutex_t 			g_sys_param_lock;//g_sys_param的读写锁


/*******************************************************************************
*@ Description    :初始化系统参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0 ，失败：-1
*@ attention      :
*******************************************************************************/
int sys_param_init(void)
{
	/*---#读取文件数据，初始化参数变量g_sys_param------------------------------------------------------------*/

	int fd = -1;
	int ret = 0;

	pthread_mutex_init(&g_sys_param_lock,NULL);
	
	if(access(SYS_PARAM_FILE,F_OK) != 0)//文件不存在，生成默认参数文件
	{
		fd = open(SYS_PARAM_FILE, O_RDWR|O_CREAT);
	    if (fd < 0) {
	        ERROR_LOG("create file %s failed!\n", SYS_PARAM_FILE);
	        return -1;
	    }

		/*---#无线（默认）参数部分------------------------------------------------------------*/
		g_sys_param.Net_param.current_flag = 0;
		strncpy(g_sys_param.Net_param.ssid,"hle666",strlen("hle666")+1);
		strncpy(g_sys_param.Net_param.password,"lanhe666",strlen("lanhe666")+1);
		g_sys_param.Net_param.mode = AVIOTC_WIFIAPMODE_ADHOC;	
		g_sys_param.Net_param.enctype = AVIOTC_WIFIAPENC_WPA2_PSK_AES;//默认使用 WPA2_PSK_TKIP 和 WPA2_PSK_AES

		/*---#有线（默认）参数部分------------------------------------------------------------*/	
		strncpy(g_sys_param.Net_param.ip,"192.168.3.22",strlen("192.168.3.22")+1);
		strncpy(g_sys_param.Net_param.mask,"255.255.255.0",strlen("255.255.255.0")+1);
		strncpy(g_sys_param.Net_param.gateway,"192.168.3.1",strlen("192.168.3.1")+1);
		strncpy(g_sys_param.Net_param.dns_server,"114.114.114.114",strlen("114.114.114.114")+1);

		//g_sys_param.P2P_param 部分
		//g_sys_param.MD_param 部分
		//g_sys_param.Acodec_param 部分
		//g_sys_param.Vcodec_param 部分
		
		/*---#LED灯（默认）参数部分------------------------------------------------------------*/
		g_sys_param.Light_param.onff = 0;
		g_sys_param.Light_param.brightness = 50;
		g_sys_param.Light_param.mode = 1;//自动模式
		g_sys_param.Light_param.lux = 50;
		g_sys_param.Light_param.pir_time = 50;
		g_sys_param.Light_param.pir_len = 30;
		g_sys_param.Light_param.pir_alarm = 1;
		g_sys_param.Light_param.auto_start = 0;
		g_sys_param.Light_param.auto_end = 0;
		g_sys_param.Light_param.shimmer_start = 0;
		g_sys_param.Light_param.shimmer_end = 0;
		g_sys_param.Light_param.timing_start = 0;
		g_sys_param.Light_param.timing_end = 0;
		
		/*---#将参数写入到配置文件------------------------------------------------------------*/
		ret = write(fd,&g_sys_param,sizeof(g_sys_param));
		if(ret <= 0)
		{
			ERROR_LOG("write file : %s failed!\n",SYS_PARAM_FILE);
			close(fd);
			return -1;
		}

		close(fd);
		return 0;
		
	}
	else //文件存在,读取文件，初始化参数变量
	{
		fd = open(SYS_PARAM_FILE, O_RDWR);
	    if (fd < 0) {
	        ERROR_LOG("can't open file: %s\n", SYS_PARAM_FILE);
	        return -1;
	    }

		ret = read(fd,&g_sys_param,sizeof(g_sys_param));
		if(ret != sizeof(g_sys_param))
		{
			ERROR_LOG("read file : %s error!\n",SYS_PARAM_FILE);
			close(fd);
			return -1;
		}

		close(fd);
		return 0;
	}
		
}

/*******************************************************************************
*@ Description    :获取系统参数（包括所有参数）
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_sys_param(system_parameter_t* sys_param)
{
	if(NULL == sys_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(sys_param,&g_sys_param,sizeof(g_sys_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
}

/*******************************************************************************
*@ Description    :保存系统参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int save_sys_param(system_parameter_t* sys_param)
{
	if(NULL == sys_param)
	{
		return -1;
	}

	pthread_mutex_lock(&g_sys_param_lock);
	if(sys_param != &g_sys_param)//如果传入的地址已经是&g_sys_param，则不需要再拷贝
	{
		memcpy(&g_sys_param,sys_param,sizeof(g_sys_param));
	}

	int fd = open(SYS_PARAM_FILE, O_RDWR|O_TRUNC);
	if (fd < 0) 
	{
	      ERROR_LOG("can't open file: %s\n", SYS_PARAM_FILE);
		  pthread_mutex_unlock(&g_sys_param_lock);
	      return -1;
	}

	int ret = write(fd,&g_sys_param,sizeof(g_sys_param));
	if(ret <= 0)
	{
		ERROR_LOG("write file : %s failed!\n",SYS_PARAM_FILE);
		pthread_mutex_unlock(&g_sys_param_lock);
		close(fd);
		return -1;
	}

	close(fd);
	pthread_mutex_unlock(&g_sys_param_lock);
	
	return 0;
	
}

/*******************************************************************************
*@ Description    :获取网络参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_Net_param(Net_parameter_t* Net_param)
{
	if(NULL == Net_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(Net_param,&g_sys_param.Net_param,sizeof(Net_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
}

/*******************************************************************************
*@ Description    :保存网络参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int save_Net_param(Net_parameter_t* Net_param)
{
	if(NULL == Net_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(&g_sys_param.Net_param,Net_param,sizeof(Net_param));
	if(save_sys_param(&g_sys_param) < 0)
	{
		ERROR_LOG("save_sys_param failed!\n");
		pthread_mutex_unlock(&g_sys_param_lock);
		return -1;
	}
	pthread_mutex_unlock(&g_sys_param_lock);

	return 0;
	
}


/*******************************************************************************
*@ Description    :获取P2P参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_P2P_param(P2P_parameter_t* P2P_param)
{
	if(NULL == P2P_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(P2P_param,&g_sys_param.P2P_param,sizeof(P2P_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
}


/*******************************************************************************
*@ Description    :保存P2P参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int save_P2P_param(P2P_parameter_t* P2P_param)
{
	if(NULL == P2P_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(&g_sys_param.P2P_param,P2P_param,sizeof(P2P_param));
	if(save_sys_param(&g_sys_param) < 0)
	{
		ERROR_LOG("save_sys_param failed!\n");
		pthread_mutex_unlock(&g_sys_param_lock);
		return -1;
	}
	pthread_mutex_unlock(&g_sys_param_lock);
	
	return 0;
	
}


/*******************************************************************************
*@ Description    :获取MD参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_MD_param(MD_parameter_t* MD_param)
{
	if(NULL == MD_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(MD_param,&g_sys_param.MD_param,sizeof(MD_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
}

/*******************************************************************************
*@ Description    :保存MD参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int save_MD_param(MD_parameter_t* MD_param)
{
	if(NULL == MD_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(&g_sys_param.MD_param,MD_param,sizeof(MD_param));
	if(save_sys_param(&g_sys_param) < 0)
	{
		ERROR_LOG("save_sys_param failed!\n");
		pthread_mutex_unlock(&g_sys_param_lock);
		return -1;
	}
	pthread_mutex_unlock(&g_sys_param_lock);

	return 0;
	
}


/*******************************************************************************
*@ Description    :获取audio编码参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_Acodec_param(Acodec_parameter_t* Acodec_param)
{
	if(NULL == Acodec_param)
	{
		return -1;
	}
	
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(Acodec_param,&g_sys_param.Acodec_param,sizeof(Acodec_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
}

/*******************************************************************************
*@ Description    :保存audio编码参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int save_Acodec_param(Acodec_parameter_t* Acodec_param)
{
	if(NULL == Acodec_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(&g_sys_param.Acodec_param,Acodec_param,sizeof(Acodec_param));
	if(save_sys_param(&g_sys_param) < 0)
	{
		ERROR_LOG("save_sys_param failed!\n");
		pthread_mutex_unlock(&g_sys_param_lock);
		return -1;
	}
	pthread_mutex_unlock(&g_sys_param_lock);

	return 0;
	
}



/*******************************************************************************
*@ Description    :获取video编码参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_Vcodec_param(Vcodec_parameter_t* Vcodec_param)
{
	if(NULL == Vcodec_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(Vcodec_param,&g_sys_param.Vcodec_param,sizeof(Vcodec_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
}

/*******************************************************************************
*@ Description    :
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int save_Vcodec_param(Vcodec_parameter_t* Vcodec_param)
{
	if(NULL == Vcodec_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(&g_sys_param.Vcodec_param,Vcodec_param,sizeof(Vcodec_param));
	if(save_sys_param(&g_sys_param) < 0)
	{
		ERROR_LOG("save_sys_param failed!\n");
		pthread_mutex_unlock(&g_sys_param_lock);
		return -1;
	}
	pthread_mutex_unlock(&g_sys_param_lock);

	return 0;
	
}


/*******************************************************************************
*@ Description    :获取LED灯参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int get_Light_param(Light_parameter_t* Light_param)
{
	if(NULL == Light_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(Light_param,&g_sys_param.Light_param,sizeof(Light_param));
	pthread_mutex_unlock(&g_sys_param_lock);
	return 0;
	
}

/*******************************************************************************
*@ Description    :保存LED灯参数
*@ Input          :
*@ Output         :
*@ Return         :成功：0；失败：-1
*@ attention      :
*******************************************************************************/
int save_Light_param(Light_parameter_t* Light_param)
{
	if(NULL == Light_param)
	{
		return -1;
	}
	pthread_mutex_lock(&g_sys_param_lock);
	memcpy(&g_sys_param.Light_param,Light_param,sizeof(Light_param));
	if(save_sys_param(&g_sys_param) < 0)
	{
		ERROR_LOG("save_sys_param failed!\n");
		pthread_mutex_unlock(&g_sys_param_lock);
		return -1;
	}
	pthread_mutex_unlock(&g_sys_param_lock);

	return 0;
	
}



int sys_param_exit(void)
{

	//参数有更新时都立即写入到文件，退出操作不需要做什么。
	pthread_mutex_destroy(&g_sys_param_lock);
}

 




