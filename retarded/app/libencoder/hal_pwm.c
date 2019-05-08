
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "pwm.h"
#include "hal_pwm.h"
#include "typeport.h"





//PWM
#define PWM_ENABLE          0x01
#define PWM_DISABLE         0x00

#define PWM_NUM 			0	//范围：[0,3] , 2已经被海思使用
#define PWM_DEV_FILE	"/dev/pwm"
int fdpwm = -1;


/*******************************************************************************
*@ Description    :
*@ Input          :<pwm_num> pwm 端口号：0-3
					<duty>占空比 [0, 1000]
					<period> PWM 的周期 范围未知,不确定[0,2000]
					<enable> PWM_DISABLE:关闭 PWM_ENABLE:打开
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
extern int PWM_DRV_Write(unsigned char pwm_num, unsigned int duty, unsigned int period, unsigned char enable);


static 	unsigned int pwm_duty = 500;  	//<duty>占空比 [0, 1000]
static  unsigned int pwm_period = 1000; //<period> PWM 的周期 范围未知,不确定[0,2000]

int pwm_reset_config(void)
{
	pwm_duty = 500;
	pwm_period = 1000;
	return 0;
}

int pwm_enable(void)
{
	
	pwm_reset_config();	
	PWM_DRV_Write(PWM_NUM, pwm_duty, pwm_period, PWM_ENABLE);

	return HLE_RET_OK;
}

int pwm_disable(void)
{
	PWM_DRV_Write(PWM_NUM, pwm_duty, pwm_period, PWM_DISABLE);
	return HLE_RET_OK;
}

/*******************************************************************************
*@ Description    :pwm波，占空比设置函数
*@ Input          :<num>占空比（对应百分比） [1,100] 
*@ Output         :
*@ Return         :成功 ： 0 ；失败：-1
*@ attention      :
*******************************************************************************/
int pwm_set_duty(unsigned int num)
{
	if(num <1 || num > 100)
	{
		ERROR_LOG("illegal parameter!\n");
		return -1;
	}

	pwm_duty = num*10;
	PWM_DRV_Write(PWM_NUM, pwm_duty, pwm_period, PWM_ENABLE);
	
	return 0;
}















