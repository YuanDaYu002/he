/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <malloc.h>



extern int do_spi_flash_probe(int argc, char *argv[]);
extern int do_spi_flash_read_write(int argc, char *argv[]);
extern int do_go (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*---#分区双备分，选择性启动 -----------------------------------------------------------*/
/*
分区划分：
nor flash：共16M
	|------1M-----|--------7M---------|----------7M-------|----64K---|-----960K-----------|
	|-----uboot---|---[0]Liteos+app---|---[1]Liteos+app---|--config--|------jffs0---------|
	0x0	   0x100000             0x800000	   0xF00000    0xF10000		0x1000000

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

//kernel + app的镜像文件描述
typedef struct _hle_image_info_t
{
    int 	image_version;  	//image镜像的版本描述（软件版本，后续再做调整）
    int 	damage_flag;		//损坏标记(FLAG_BAD 或者 FLAG_OK)
    ulong 	start_address;		//内核的加载地址
}hle_image_info_t;

//config 分区结构
typedef struct _config_info_t
{
	int 				magic_flag;		//魔数，匹配上才能执行操作，否则使用默认配置
   	hle_image_info_t 	image_info[2]; //两个分区镜像的描述信息 
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


extern int do_spi_flash_erase(int argc, char *argv[]);
/*******************************************************************************
*@ Description    :	初始化config分区(设备烧片后初次启动)
*@ Input          :
*@ Output         :
*@ Return         :成功：0  	失败：-1
*@ attention      :
*******************************************************************************/
int init_config_info(void)
{
	printf("---INIT CONFIG REGION INFO----------------------------------------\n");

	config_info_t * config_info = (config_info_t*)malloc(CONFIG_REGION_SIZE);
	if(NULL == config_info)
	{
		printf("\033[1;31m<ERR!!>init_config_info error!!\n\033[0m");
		return -1;
	}
	memset(config_info,0,CONFIG_REGION_SIZE);

	//出厂状态，默认只有 IMAGE0 分区有镜像文件
	config_info->magic_flag = HLE_MAGIC;
	config_info->image_info[0].image_version = 1;
	config_info->image_info[0].damage_flag = FLAG_OK;
	config_info->image_info[0].start_address = IMAGE0_REGION_START;

	config_info->image_info[1].image_version = 0;
	config_info->image_info[1].damage_flag = FLAG_BAD;
	config_info->image_info[1].start_address = IMAGE1_REGION_START;


	//擦除并写入norflash 的 config 分区
	char erase_off [32] = {0};
	char erase_size [32] = {0};
	sprintf(erase_off,"0x%x",CONFIG_REGION_START);
	sprintf(erase_size,"0x%x",CONFIG_REGION_SIZE);
	printf("## config: erase_off[] = %s\n",erase_off);
	printf("## config: erase_size[] = %s\n",erase_size);

	char* argv_erase[3] = {"erase",(char*)&erase_off,(char*)&erase_size};
	do_spi_flash_erase(3,argv_erase);
	
	//sf write 
	char write_from_addr[32] = {0};
	sprintf(write_from_addr,"0x%x",(int)config_info);
	char write_offset[32] = {0};
	sprintf(write_offset,"0x%x",CONFIG_REGION_START);
	char write_len[32] = {0};
	sprintf(write_len,"0x%x",CONFIG_REGION_SIZE);

	printf("write_from_addr[] = %s\n",write_from_addr);
	printf("write_offset[] = %s\n",write_offset);
	printf("write_len[] = %s\n",write_len);
	
	char* argv_write[4] = {"write",(char*)&write_from_addr,(char*)&write_offset,(char*)&write_len};
	do_spi_flash_read_write(4,argv_write);

	//进行一次 saveenv 操作，防止uboot 报警告：*** Warning - bad CRC, using default environment
	setenv("bootcmd","hle_bootm");
	saveenv();
	
	printf("------------------------------------------------------------------\n");

	return 0;
}


/*******************************************************************************
*@ Description    :选择启动镜像（kernel + app）分区
*@ Input          :
*@ Output         :
*@ Return         :镜像的起始地址
*@ attention      :
*******************************************************************************/
ulong select_boot_system_partition(void)
{
	//读取 norflash 配置分区的数据到内存
	config_info_t *config_info = ((config_info_t*)malloc(CONFIG_REGION_SIZE));
	if(NULL == config_info)
	{
		printf("\033[1;31m<ERR!!>select_boot_system_partition malloc failed!!\n\033[0m");
		return (ulong)IMAGE0_REGION_START;//默认返回分区0的image
	}
	memset(config_info,0,CONFIG_REGION_SIZE);

	char config_size[32] = {0};
	sprintf(config_size,"0x%x",CONFIG_REGION_SIZE);
	//printf("## config_size[] = %s\n",config_size);

	char mem_addr[32] = {0};
	sprintf(mem_addr,"0x%x",(int)config_info);
	//printf("## mem_addr[] = %s\n",mem_addr);

	char src_flash_addr[32] = {0};
	sprintf(src_flash_addr,"0x%x",CONFIG_REGION_START);
	//printf("## src_flash_addr[] = %s\n",src_flash_addr);
	
	//sf read 0x80000000 config_addr 0x100000
	char* argv[4] = {"read",(char*)&mem_addr,(char*)&src_flash_addr,(char*)&config_size};
	do_spi_flash_read_write(4,argv);

	
	if(config_info->magic_flag != HLE_MAGIC)
	{ 
		printf("config_info->magic_flag = %#x\n",config_info->magic_flag);
		printf("\033[1;31m<ERR!!>config_info.magic_flag error!! start to init CONFIG REGION...\n\033[0m");
		init_config_info();//出厂初次启动，初始化 config 分区
		return (ulong)IMAGE0_REGION_START;//默认返回分区0的image
	}

	/*打印两个分区的详细信息*/
	printf_config_info(config_info);
	
	//情况1：两个都为坏的,默认返回分区0的image	
	if(config_info->image_info[0].damage_flag != FLAG_OK &&
	   config_info->image_info[1].damage_flag != FLAG_OK)
	{
		printf("\033[1;31m<ERR!!>Both image[0] and image[1] is bad !!\n\033[0m");
		return (ulong)IMAGE0_REGION_START;
	}

	//情况2：两个都是好的,谁的版本高用哪个
	if(config_info->image_info[0].damage_flag == FLAG_OK &&
	   config_info->image_info[1].damage_flag == FLAG_OK) 
	{
		printf("## Both image[0] and image[1] is OK!!\n");
		if(config_info->image_info[0].image_version > 
		   config_info->image_info[1].image_version)
		{
			return config_info->image_info[0].start_address;
		}
		else
		{
			return config_info->image_info[1].start_address;
		}
	}
	   
	//情况3：只有一个是好的，只能用这个好的
	if(config_info->image_info[0].damage_flag == FLAG_OK)
	{
		printf("##Just image[0] is OK!!\n");
		return config_info->image_info[0].start_address;
	}
	else //if(config_info.image_info[1].damage_flag == FLAG_OK)
	{
		printf("##Just image[1] is OK!!\n");
		return config_info->image_info[1].start_address;
	}

}


//bootcmd=sf probe 0; sf read 0x80000000 0x100000 0x700000; go 0x80000000
int do_double_system_bootm(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 1) 
	{
		cmd_usage(cmdtp);
		return 1;
	}

	//sf probe 0;
	char* argv1[2] = {"probe","0"};
	do_spi_flash_probe(2,argv1);

	//选择启动分区(获取分区首地址)
	char image_addr[32] = {0};
	ulong addr = select_boot_system_partition();
	sprintf(image_addr,"0x%lx",addr);
	printf("## image_addr[] = %s\n",image_addr);

	//sf read 0x80000000 addr 0x700000
	char*argv2[4] = {"read","0x80000000",(char*)&image_addr,"0x700000"};
	do_spi_flash_read_write(4,argv2);

	//go 0x80000000
	char*argv3[2] = {"go","0x80000000"};
	do_go(NULL,0,2,argv3);

	return 0;
	
}


U_BOOT_CMD(
	hle_bootm, 1, 0,	do_double_system_bootm,
	"select (kernel + app) image and boot it , No input parameters are required",
	""
);



