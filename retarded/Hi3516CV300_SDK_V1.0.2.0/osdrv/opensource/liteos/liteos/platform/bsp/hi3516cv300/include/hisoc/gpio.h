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

#ifndef    __HISOC_GPIO_H__
#define    __HISOC_GPIO_H__

#include "asm/platform.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define GPIO_GROUP_NUM 9
#define GPIO_BIT_NUM 8
#define GPIO_IRQ_NUM 1

#define GPIO_IRQ_TAB gpio_irq_handler_tab pl061_irq_handler_tab[GPIO_IRQ_NUM] ={ \
    {NUM_HAL_INTERRUPT_GPIO,9,NULL} \
}

#define GPIO_GROUP_TAB gpio_group_ctrl pl061_group_ctrl[GPIO_GROUP_NUM] = { \
    [0] = { \
        .group_num = 0, \
        .gpio_group_reg_base = GPIO0_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [1] = { \
        .group_num = 1, \
        .gpio_group_reg_base = GPIO1_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [2] = { \
        .group_num = 2, \
        .gpio_group_reg_base = GPIO2_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [3] = { \
        .group_num = 3, \
        .gpio_group_reg_base = GPIO3_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [4] = { \
        .group_num = 4, \
        .gpio_group_reg_base = GPIO4_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [5] = { \
        .group_num = 5, \
        .gpio_group_reg_base = GPIO5_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [6] = { \
        .group_num = 6, \
        .gpio_group_reg_base = GPIO6_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [7] = { \
        .group_num = 7, \
        .gpio_group_reg_base = GPIO7_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    }, \
    [8] = { \
        .group_num = 8, \
        .gpio_group_reg_base = GPIO8_REG_BASE, \
        .irq = NUM_HAL_INTERRUPT_GPIO, \
    } \
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

