/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
#ifdef LOSCFG_SHELL
#include "los_hwi.h"
#include "shcmd.h"
#include "shell.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#if defined(HI3559) || defined(HI3556)
char *hwi_name_map[] =
{
    /*0 */    "Inner Irq",
    /*1 */    "Inner Irq",
    /*2 */    "Inner Irq",
    /*3 */    "Inner Irq",
    /*4 */    "Inner Irq",
    /*5 */    "Inner Irq",
    /*6 */    "Inner Irq",
    /*7 */    "Inner Irq",
    /*8 */    "Inner Irq",
    /*9 */    "Inner Irq",
    /*10*/    "Inner Irq",
    /*11*/    "Inner Irq",
    /*12*/    "Inner Irq",
    /*13*/    "Inner Irq",
    /*14*/    "Inner Irq",
    /*15*/    "Inner Irq",
    /*16*/    "Inner Irq",
    /*17*/    "Inner Irq",
    /*18*/    "Inner Irq",
    /*19*/    "Inner Irq",
    /*20*/    "Inner Irq",
    /*21*/    "Inner Irq",
    /*22*/    "Inner Irq",
    /*23*/    "Inner Irq",
    /*24*/    "Inner Irq",
    /*25*/    "Inner Irq",
    /*26*/    "Inner Irq",
    /*27*/    "Inner Irq",
    /*28*/    "Inner Irq",
    /*29*/    "Inner Irq",
    /*30*/    "Inner Irq",
    /*31*/    "Inner Irq",
    /*32*/    "WatchDog",
    /*33*/    "RTC/TEM_CAP",
    /*34*/    "null",
    /*35*/    "null",
    /*36*/    "UART0",
    /*37*/    "UART1",
    /*38*/    "UART2",
    /*39*/    "UART3",
    /*40*/    "UART4",
    /*41*/    "SSP0/I2C0",
    /*42*/    "SSP1/I2C2",
    /*43*/    "SSP2",
    /*44*/    "SSP3/I2C3",
    /*45*/    "MMC2",
    /*46*/    "I2C2",
    /*47*/    "IR",
    /*48*/    "LSADC",
    /*49*/    "DMAC",
    /*50*/    "FMC",
    /*51*/    "USB2_EHCI",
    /*52*/    "USB2_OHCI",
    /*53*/    "USB2_DEV",
    /*54*/    "USB3",
    /*55*/    "MMC0",
    /*56*/    "MMC1",
    /*57*/    "null",
    /*58*/    "Cipher",
    /*59*/    "VDP",
    /*60*/    "MIPI0",
    /*61*/    "MIPI1",
    /*62*/    "VICAP0",
    /*63*/    "VICAP1",
    /*64*/    "VPSS",
    /*65*/    "null",
    /*66*/    "TDE",
    /*67*/    "VGS",
    /*68*/    "AIAO",
    /*69*/    "VEDU",
    /*70*/    "JPGE",
    /*71*/    "IVE",
    /*72*/    "null",
    /*73*/    "GZIP",
    /*74*/    "SWI",
    /*75*/    "GPIO0~7",
    /*76*/    "GPIO8~14/GPIO16",
    /*77*/    "A7_PMU",
    /*78*/    "A17_PMU",
    /*79*/    "CCI",
    /*80*/    "GDC",
    /*81*/    "A17_COMMRX",
    /*82*/    "A17_COMMTX",
    /*83*/    "A7_COMMRX",
    /*84*/    "A7_COMMTX",
    /*85*/    "null",
    /*86*/    "PCIE_CFG",
    /*87*/    "PCIE_CFG",
    /*88*/    "PCIE_PM",
    /*89*/    "PCIE_INTA",
    /*90*/    "PCIE_INTB",
    /*91*/    "PCIE_INTC",
    /*92*/    "PCIE_INTD",
    /*93*/    "PCIE_EDMA",
    /*94*/    "PCIE_MSI",
    /*95*/    "PCIE_LINK_DOWN",
    /*96*/    "TIMER0",
    /*97*/    "TIMER1",
    /*98*/    "TIMER2",
    /*99*/    "TIMER3",
    /*100*/    "TIMER4",
    /*101*/    "TIMER5",
    /*102*/    "null",
    /*103*/    "null",
    /*104*/    "null",
    /*105*/    "null",
    /*106*/    "null",
    /*107*/    "null",
    /*108*/    "null",
    /*109*/    "null",
    /*110*/    "null",
    /*111*/    "null",
    /*112*/    "null",
    /*113*/    "null",
    /*114*/    "null",
    /*115*/    "null",
    /*116*/    "null",
    /*117*/    "null",
    /*118*/    "null",
    /*119*/    "null",
    /*120*/    "null",
    /*121*/    "null",
    /*122*/    "null",
    /*123*/    "null",
    /*124*/    "null",
    /*125*/    "null",
    /*126*/    "null",
    /*127*/    "null",
};
#endif

extern UINT32 g_vuwHwiFormCnt[OS_HWI_MAX_NUM];
LITE_OS_SEC_TEXT_MINOR UINT32 shCmd_hwi(int argc, char **argv)
{
    int i  = 0;

    if (argc > 0)
    {
        PRINTK("\nUsage: hwi\n");
        return OS_ERROR;
    }
#if defined(HI3559) || defined(HI3556)
    PRINTK(" InterruptNo  Name             Count\n");
    for(i = 0; i < OS_HWI_MAX_NUM; i++)
    {
        if(g_vuwHwiFormCnt[i] != 0)
        {
            PRINTK(" %8d:    %-15s  %-10d\n",
                    i, hwi_name_map[i], g_vuwHwiFormCnt[i]);
        }
    }
#else
    PRINTK(" InterruptNo     Count\n");
    for(i = 0; i < OS_HWI_MAX_NUM; i++)
    {
        if(g_vuwHwiFormCnt[i] != 0)
        {
            PRINTK(" %8d:%10d\n",i,g_vuwHwiFormCnt[i]);
        }
    }
#endif

    return 0;
}

SHELLCMD_ENTRY(hwi_shellcmd, CMD_TYPE_EX, "hwi", 1, (CMD_CBK_FUNC)shCmd_hwi); /*lint !e19*/

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* LOSCFG_SHELL */

