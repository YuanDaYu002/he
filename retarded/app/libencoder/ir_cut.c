
#include <sys/time.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include "mpi_isp.h"
#include "hi_comm_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_sys.h"
#include "gpio_reg.h"
#include "typeport.h"
#include "comm_sys.h"



typedef struct
{
	HI_BOOL   bThreadFlag;
	pthread_t pThread;
}HI_IR_AUTO_THREAD_S;
static HI_IR_AUTO_THREAD_S g_astIrThread[ISP_MAX_DEV_NUM] = {{0}};

static ISP_IR_AUTO_ATTR_S g_astIrAttr[ISP_MAX_DEV_NUM] = {{0}};
static ISP_DEV IspDev = 0;

#define HI_3516CV300_CHIP           (0)
#define HI_3516EV100_CHIP           (1)
#define GAIN_MAX_COEF               (280)
#define GAIN_MIN_COEF               (190)




/*******************************************************************************
*@ Description    :IR-CUT 切换到 正常模式
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int  IR_CUT_ENBLE(void)
{
	int ret; 

	//设置复用
	HI_MPI_SYS_SetReg(MUXCTRL_REG62,0x0);//GPIO0_1

	
	//设置方向,GPIO0_1 	/GPIO0_2 配置为输出
	HLE_U32 val;
	HI_MPI_SYS_GetReg(GPIO_DIR(0), &val);
	val = val|0x2;
	val = val|0x4;
	HI_MPI_SYS_SetReg(GPIO_DIR(0),val);

	//设置值
	HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,1),0x00);//GPIO0_1 设置为 0（低电平）	
	HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,2),0x04);//GPIO0_2 设置为 1（高电平）
	
	usleep(100*1000);
	//reset
	HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,1),0);		
	HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,2),0);

	return 0;


}

//IR MODE
/*******************************************************************************
*@ Description    :IR-CUT 切换到 夜视模式
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int  IR_CUT_DISABLE(void)
{
    	int ret;

		//设置复用
		HI_MPI_SYS_SetReg(MUXCTRL_REG62,0x0);//GPIO0_1
		
		//设置方向,GPIO0_1	/GPIO0_2 配置为输出
		HLE_U32 val;
		HI_MPI_SYS_GetReg(GPIO_DIR(0), &val);
		val = val|0x2;
		val = val|0x4;
		HI_MPI_SYS_SetReg(GPIO_DIR(0),val);
		
		//设置值
		HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,1),0x02);//GPIO0_1 设置为 1（高电平）
		HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,2),0x00);//GPIO0_2 设置为 0（低电平）
		
		usleep(100*1000);
		//reset
		HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,1),0);		
		HI_MPI_SYS_SetReg(GPIO_PIN_DATA(0,2),0);

		return 0;
}



/*******************************************************************************
*@ Description    :切换到正常模式（图像 + IR-CUT）
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
HI_S32 ISP_IrSwitchToNormal(ISP_DEV IspDev)
{
    HI_S32 s32Ret;
    ISP_SATURATION_ATTR_S stIspSaturationAttr;
    ISP_WB_ATTR_S stIspWbAttr;
    ISP_COLORMATRIX_ATTR_S stIspCCMAttr;

    /* switch ir-cut to normal */
    IR_CUT_ENBLE();


    /* switch isp config to normal */
    s32Ret = HI_MPI_ISP_GetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetSaturationAttr failed\n");
        return s32Ret;
    }
    stIspSaturationAttr.enOpType = OP_TYPE_AUTO;
    s32Ret = HI_MPI_ISP_SetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetSaturationAttr failed\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &stIspWbAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetWBAttr failed\n");
        return s32Ret;
    }
    stIspWbAttr.enOpType = OP_TYPE_AUTO;
    s32Ret = HI_MPI_ISP_SetWBAttr(IspDev, &stIspWbAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetWBAttr failed\n");
        return s32Ret;
    }

    s32Ret = HI_MPI_ISP_GetCCMAttr(IspDev, &stIspCCMAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetCCMAttr failed\n");
        return s32Ret;
    }
    stIspCCMAttr.enOpType = OP_TYPE_AUTO;
    s32Ret = HI_MPI_ISP_SetCCMAttr(IspDev, &stIspCCMAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetCCMAttr failed\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

/*******************************************************************************
*@ Description    :切换到夜视模式（图像 + IR-CUT）
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
HI_S32 ISP_IrSwitchToIr(ISP_DEV IspDev)
{
    HI_S32 s32Ret;
    ISP_SATURATION_ATTR_S stIspSaturationAttr;
    ISP_WB_ATTR_S stIspWbAttr;
    ISP_COLORMATRIX_ATTR_S stIspCCMAttr;

    /* switch isp config to ir */
	/*---#修改ISP 饱和度属性 ------------------------------------------------------------*/
    s32Ret = HI_MPI_ISP_GetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetSaturationAttr failed\n");
        return s32Ret;
    }
	
    stIspSaturationAttr.enOpType = OP_TYPE_MANUAL; //设置饱和度类型(手动)。
    stIspSaturationAttr.stManual.u8Saturation = 0; //手动饱和度值。取值范围：[0x0, 0xFF]
    s32Ret = HI_MPI_ISP_SetSaturationAttr(IspDev, &stIspSaturationAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetSaturationAttr failed\n");
        return s32Ret;
    }

	/*---#修改ISP 白平衡属性。------------------------------------------------------------*/
    s32Ret = HI_MPI_ISP_GetWBAttr(IspDev, &stIspWbAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetWBAttr failed\n");
        return s32Ret;
    }
	
	stIspWbAttr.enOpType = OP_TYPE_MANUAL; //切换为手动白平衡
	stIspWbAttr.stManual.u16Rgain  = 0x100;//手动白平衡(红色)通道增益，8bit 小数精度。取值范围：[0x0,0xFFF]。
	stIspWbAttr.stManual.u16Grgain = 0x100;//手动白平衡(绿色)通道增益，8bit 小数精度。取值范围：[0x0,0xFFF]。
	stIspWbAttr.stManual.u16Gbgain = 0x100;//手动白平衡(绿色)通道增益，8bit 小数精度。取值范围：[0x0,0xFFF]
	stIspWbAttr.stManual.u16Bgain  = 0x100;//手动白平衡(蓝色)通道增益，8bit 小数精度。取值范围：[0x0,0xFFF]。
       
    s32Ret = HI_MPI_ISP_SetWBAttr(IspDev, &stIspWbAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetWBAttr failed\n");
        return s32Ret;
    }
	
	/*---#修改颜色矩阵属性------------------------------------------------------------*/
    s32Ret = HI_MPI_ISP_GetCCMAttr(IspDev, &stIspCCMAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_GetCCMAttr failed\n");
        return s32Ret;
    }
    stIspCCMAttr.enOpType = OP_TYPE_MANUAL;//颜色校正矩阵类型(手动模式)。
	/*手动颜色校正矩阵，8bit 小数精度。bit 15 是符号位，0 表示正数，1
	表示负数，例如 0x8010 表示-16。取值范围：[0x0,0xFFFF]。
	*/
	stIspCCMAttr.stManual.au16CCM[0] = 0x100;
    stIspCCMAttr.stManual.au16CCM[1] = 0x0;
    stIspCCMAttr.stManual.au16CCM[2] = 0x0;
    stIspCCMAttr.stManual.au16CCM[3] = 0x0;
    stIspCCMAttr.stManual.au16CCM[4] = 0x100;
    stIspCCMAttr.stManual.au16CCM[5] = 0x0;
    stIspCCMAttr.stManual.au16CCM[6] = 0x0;
    stIspCCMAttr.stManual.au16CCM[7] = 0x0;
    stIspCCMAttr.stManual.au16CCM[8] = 0x100;
    s32Ret = HI_MPI_ISP_SetCCMAttr(IspDev, &stIspCCMAttr);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_ISP_SetCCMAttr failed\n");
        return s32Ret;
    }
	
    usleep(1000000);//1s睡眠

    /* switch ir-cut to ir */
    IR_CUT_DISABLE();


    return HI_SUCCESS;
}

/*******************************************************************************
*@ Description    :(正常/夜视) 模式自动切换线程函数
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
HI_S32 ISP_IrAutoRun(ISP_DEV IspDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    ISP_IR_AUTO_ATTR_S stIrAttr;

    stIrAttr.bEnable            = g_astIrAttr[IspDev].bEnable;
    stIrAttr.u32Normal2IrIsoThr = g_astIrAttr[IspDev].u32Normal2IrIsoThr;
    stIrAttr.u32Ir2NormalIsoThr = g_astIrAttr[IspDev].u32Ir2NormalIsoThr;
    stIrAttr.u32RGMax           = g_astIrAttr[IspDev].u32RGMax;
    stIrAttr.u32RGMin           = g_astIrAttr[IspDev].u32RGMin;
    stIrAttr.u32BGMax           = g_astIrAttr[IspDev].u32BGMax;
    stIrAttr.u32BGMin           = g_astIrAttr[IspDev].u32BGMin;
    stIrAttr.enIrStatus         = g_astIrAttr[IspDev].enIrStatus;

    while (HI_TRUE == g_astIrThread[IspDev].bThreadFlag)
    {
        /* run_interval: 40 ms */
        usleep(40000);

		//运行红外自动切换功能
        s32Ret = HI_MPI_ISP_IrAutoRunOnce(IspDev, &stIrAttr);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ISP_IrAutoRunOnce failed\n");
            return s32Ret;
        }
        
        if (ISP_IR_SWITCH_TO_IR == stIrAttr.enIrSwitch) /* Normal to IR */
        {
            printf("\n[Normal -> IR]\n");

            s32Ret = ISP_IrSwitchToIr(IspDev);
            if (HI_SUCCESS != s32Ret)
            {
                printf("ISP_IrSwitchToIr failed\n");
                return s32Ret;
            }

            stIrAttr.enIrStatus = ISP_IR_STATUS_IR;
        }
        else if (ISP_IR_SWITCH_TO_NORMAL == stIrAttr.enIrSwitch) /* IR to Normal */
        {
            printf("\n[IR -> Normal]\n");

            s32Ret = ISP_IrSwitchToNormal(IspDev);
            if (HI_SUCCESS != s32Ret)
            {
                printf("ISP_IrSwitchToNormal failed\n");
                return s32Ret;
            }

            stIrAttr.enIrStatus = ISP_IR_STATUS_NORMAL;
        }
        else
        {}
    }

    return HI_SUCCESS;
}


/*******************************************************************************
*@ Description    :自动模式 线程入口函数
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
static void* ISP_IrAutoThread(void* param)
{
	DEBUG_LOG("ISP_IrAutoThread start!\n");
    pthread_detach(pthread_self());
    ISP_DEV IspDev;

    IspDev = (ISP_DEV)param;

    prctl(PR_SET_NAME, (unsigned long)"isp_IrAuto", 0, 0, 0);//修改线程名字
    ISP_IrAutoRun(IspDev); //循环，直到退出

	DEBUG_LOG("ISP_IrAutoThread exit!\n");

	pthread_exit(NULL);
	return NULL;
}

/*******************************************************************************
*@ Description    :退出ir-cut切换线程
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
HI_S32 ISP_IrAutoExit(ISP_DEV IspDev)
{
    if (g_astIrThread[IspDev].pThread)
    {
        g_astIrThread[IspDev].bThreadFlag = HI_FALSE;
        //pthread_join(g_astIrThread[IspDev].pThread, NULL);
        g_astIrThread[IspDev].pThread = 0;
    }

    return HI_SUCCESS;
}



/*******************************************************************************
*@ Description    :IR-CUT主业务入口函数
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int ir_cut_main(void)
{
	printf("-------------into ir_cut_main--------------------\n");
    HI_S32 s32Ret = HI_SUCCESS;
    
    IspDev = 0;
    
    /* need to modified when sensor/lens/IR-cut/Infrared light changed */
    g_astIrAttr[IspDev].bEnable = HI_TRUE;
    g_astIrAttr[IspDev].u32Normal2IrIsoThr = 16000; //从普通状态切换到红外状态的 ISO 阈值(大于该值就会切换)
    g_astIrAttr[IspDev].u32Ir2NormalIsoThr = 400;//从红外状态切换到普通状态的 ISO 阈值(小于该值就需要切换)
    g_astIrAttr[IspDev].u32RGMax = 280;//红外状态下的 R/G 最大值(实际图像的 R/G 大于此参数时,切回普通状态)
    g_astIrAttr[IspDev].u32RGMin = 190;//红外状态下的 R/G 最小值(实际图像的 R/G 小于此参数时,切回普通状态)
    g_astIrAttr[IspDev].u32BGMax = 280;//红外状态下的 B/G 最大值(实际图像的 B/G 大于此参数时,切回普通状态)
    g_astIrAttr[IspDev].u32BGMin = 190;//红外状态下的 B/G 最小值(实际图像的 B/G 小于此参数时,切回普通状态)
    g_astIrAttr[IspDev].enIrStatus = ISP_IR_STATUS_NORMAL;//设备当前的红外状态

    /* SAMPLE_IR_AUTO_MODE 启动自动模式线程*/
    g_astIrThread[IspDev].bThreadFlag = HI_TRUE;
    s32Ret = pthread_create(&g_astIrThread[IspDev].pThread, HI_NULL, ISP_IrAutoThread, (HI_VOID *)IspDev);
    if (0 != s32Ret)
    {
        printf("%s: create isp ir_auto thread failed!, error: %d, %s\r\n", __FUNCTION__, s32Ret, strerror(s32Ret));
        return HI_FAILURE;
    }
	

    //ISP_IrAutoExit(IspDev);

    return HI_SUCCESS;

}





