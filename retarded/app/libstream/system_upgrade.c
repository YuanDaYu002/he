 
/***************************************************************************
* @file:system_upgrade.c 
* @author:   
* @date:  5,5,2019
* @brief:  系统升级部分函数
* @attention: 如若该部分分区有修改，则uboot也需要做同步的修改，主要涉及文件：
				u-boot-2010.06/common/cmd_hle_double_system_bootm.c
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include "spinor.h"
#include "typeport.h"
#include "system_upgrade.h"



 /*
 分区划分：
 nor flash：共16M
	 |------1M-----|--------7M---------|----------7M-------|----64K---|-----960K-----------|
	 |-----uboot---|---[0]Liteos+app---|---[1]Liteos+app---|--config--|------jffs0---------|
	 0x0	0x100000			 0x800000		0xF00000	0xF10000	 0x1000000
 
 */
	 // 0: kernel + app 分区
#define		IMAGE0_REGION_START		0x100000
#define		IMAGE0_REGION_SIZE		0x700000	//7M
	 // 1: kernel + app 分区
#define		IMAGE1_REGION_START		0x800000
#define		IMAGE1_REGION_SIZE		0x700000	//7M
	 // config 分区
#define 	CONFIG_REGION_START		0xF00000
#define		CONFIG_REGION_SIZE		0x10000		//64K
	 // jffs2 分区
#define 	CONFIG_JFFS2_START		0xF10000
#define 	CONFIG_JFFS2_SIZE		0xF0000		//960K
	 
	 
#define 	HLE_MAGIC 	0xB1A75   	//HLE ASCII(76 72 69-->767269-->0xB1A75)
#define 	FLAG_BAD  	0x97048c 	//损坏标记(“bad”的ASCII码：9897100-->0x97048c)
#define 	FLAG_OK		0x1b203		//正常标记(“ok”的ASCII码：111107--->0x1b203)

typedef  unsigned long   ulong ;

//kernel + app的镜像文件描述
typedef struct _hle_image_info_t
{
	 int	 image_version; 	 //image镜像的版本描述（软件版本，后续再做调整）
	 int	 damage_flag;		 //损坏标记(FLAG_BAD 或者 FLAG_OK)
	 ulong	 start_address; 	 //内核的加载地址
}hle_image_info_t;
 
//config 分区结构
typedef struct _config_info_t
{
	 int				 magic_flag;	 //魔数，匹配上才能执行操作，否则使用默认配置
	 hle_image_info_t	 image_info[2]; //两个分区镜像的描述信息 
}config_info_t;


	
void printf_config_info(config_info_t* info)
{
	 printf("---CONFIG REGION INFO:---------------------------------------\n");
	 printf("info->magic_flag = %#x\n",info->magic_flag);
	 printf("info->image_info[0].image_version = %d\n",info->image_info[0].image_version);
	 printf("info->image_info[0].damage_flag   = %#x\n",info->image_info[0].damage_flag);
	 printf("info->image_info[0].start_address = %#lx\n",info->image_info[0].start_address);
	 
	 printf("info->image_info[1].image_version = %d\n",info->image_info[1].image_version);
	 printf("info->image_info[1].damage_flag   = %#x\n",info->image_info[1].damage_flag);
	 printf("info->image_info[1].start_address = %#lx\n",info->image_info[1].start_address);
	 printf("-------------------------------------------------------------\n");
}

/*******************************************************************************
*@ Description    :选择需要升级的系统分区
*@ Input          :
*@ Output         :<addr>返回分区的地址
					<region>升级的分区号（0/1）
*@ Return         :状态信息（错误码）
*@ attention      :
*******************************************************************************/
static config_info_t config_info = {0};
int selet_upgrade_region(unsigned long* addr,int* region)
{
	memset(&config_info,0,sizeof(config_info));
	
	hispinor_read(&config_info , CONFIG_REGION_START, sizeof(config_info)/*CONFIG_REGION_SIZE*/);
	if(config_info.magic_flag != HLE_MAGIC)
	{
		ERROR_LOG("config_info.magic_flag  not equal HLE_MAGIC\n");
		return -1;
	}

	/*打印两个分区的详细信息*/
	printf_config_info(&config_info);
	
	//情况1：两个分区都为坏的，返回版本低的分区
	if(config_info.image_info[0].damage_flag != FLAG_OK &&
	   config_info.image_info[1].damage_flag != FLAG_OK)
	{
		*addr = (config_info.image_info[0].image_version > config_info.image_info[1].image_version)?
				config_info.image_info[1].start_address : config_info.image_info[0].start_address;
		*region = (config_info.image_info[0].image_version > config_info.image_info[1].image_version)?1:0;
		return 0;
	}

	//情况2：两个都是好的,返回版本低的分区
	if(config_info.image_info[0].damage_flag == FLAG_OK &&
	   config_info.image_info[1].damage_flag == FLAG_OK) 
	{
		
		if(config_info.image_info[0].image_version > 
		   config_info.image_info[1].image_version)
		{
			*addr = config_info.image_info[1].start_address;
			*region = 1;
		}
		else
		{
			*addr = config_info.image_info[0].start_address;
			*region = 0;
		}
		return 0;
	}
	   
	//情况3：只有一个是好的，返回坏的这个分区
	if(config_info.image_info[0].damage_flag == FLAG_OK)
	{
		DEBUG_LOG("##Just image[0] is OK!!\n");
		*addr = config_info.image_info[1].start_address;
		*region = 1;
		return 0;
	}
	else //if(config_info.image_info[1].damage_flag == FLAG_OK)
	{
		DEBUG_LOG("##Just image[1] is OK!!\n");
		*addr = config_info.image_info[0].start_address;
		*region = 0;
		return 0;
	}
	
	return -1;
}


/*******************************************************************************
*@ Description    :升级业务，写分区部分（双系统）
*@ Input          :<buf> 升级文件的buf位置
					<buf_lem>升级文件的buf大小（注意不能超过 IMAGE0/1 分区大小）
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
upgrade_status_e upgrade_write_norflash(void *buf,unsigned int buf_len)
{
	if(NULL == buf || buf_len <= 0)
	{
		return  UP_ILLEGAL_PARAMETER;
	}
	
	int ret = 0;


	//CRC校验

	//版本校验


	//选择 IMAGE 分区
	unsigned long upgrade_addr = 0;
	int region = 0; //IMAGE 分区序号
	ret = selet_upgrade_region(&upgrade_addr,&region);
	if(ret < 0)
	{
		ERROR_LOG("selet_upgrade_region failed !\n");
		return UP_SELECT_REGION_FAILED;
	}
	
	DEBUG_LOG("erase region is : %d  addr:%#x\n",region,upgrade_addr);
	
	unsigned long region_size = 0;
	if(0 == region)
		region_size = IMAGE0_REGION_SIZE;
	else
		region_size = IMAGE1_REGION_SIZE;

	if(buf_len > region_size)
	{
		ERROR_LOG("upgrade buf is too long , upgrade region overflow!!\n");
		return UP_REGION_OVERFLOW;
	}

	//1.擦除   CONFIG 分区
	//DEBUG_LOG("hispinor_erase CONFIG...\n");
	hispinor_erase((unsigned long)CONFIG_REGION_START,(unsigned long)CONFIG_REGION_SIZE);
	
	//2.写 CONFIG 分区（非升级分区的参数）
	if(0 == region)
	{
		hispinor_write(&config_info.magic_flag,CONFIG_REGION_START,sizeof(config_info.magic_flag));
		//空出中间 IMAGE0 的部分
		unsigned long start = CONFIG_REGION_START + sizeof(config_info.magic_flag) + sizeof(hle_image_info_t);
		hispinor_write(&config_info.image_info[1],start,sizeof(hle_image_info_t));
	}
	else
	{
		hispinor_write(&config_info,CONFIG_REGION_START,sizeof(config_info)-sizeof(hle_image_info_t));
		//空出后部分 IMAGE1 的部分
	}
	
	//3.擦除 IMAGE 分区（升级文件）
	DEBUG_LOG("hispinor_erase IMAGE ... \n");
	hispinor_erase(upgrade_addr,region_size);
	
	//4.写 IMAGE 分区 （升级文件）
	DEBUG_LOG("hispinor_write IMAGE ... \n");
	hispinor_write(buf,upgrade_addr, buf_len);

	//5.写 CONFIG 分区（升级分区的参数）
	config_info.image_info[region].image_version = config_info.image_info[1 - region].image_version + 1;
	config_info.image_info[region].damage_flag = FLAG_OK;
	if(0 == region)
	{
		config_info.image_info[region].start_address = IMAGE0_REGION_START;
		//写中间 IMAGE0 的部分
		unsigned long start = CONFIG_REGION_START + sizeof(config_info.magic_flag);
		hispinor_write(&config_info.image_info[0],start,sizeof(hle_image_info_t));
	}
	else
	{
		config_info.image_info[region].start_address = IMAGE1_REGION_START;
		//写后部分 IMAGE1 的部分
		unsigned long start = CONFIG_REGION_START + sizeof(config_info.magic_flag) + sizeof(hle_image_info_t);
		hispinor_write(&config_info.image_info[1],start,sizeof(hle_image_info_t));
	}
	
	
	return 0;

}












