

#ifndef _GPIO_REG_H
#define _GPIO_REG_H


#define GPIO_BASE(group)                (0x12140000 + (group) * 0x1000)
#define GPIO_PIN_DATA(group, pin)       (GPIO_BASE(group) + (1 << (2 + pin)))
#define GPIO_DIR(group)                 (GPIO_BASE(group) + 0x400)

//Audio
#define STATUS_AUDIO_MUTE   GPIO_PIN_DATA(3, 2)
#define MUXCTRL_REG35	0x1204008C

// IR-CUT
//复用寄存器：
//#define MUXCTRL_REG1	0x12040004
//#define MUXCTRL_REG2	0x12040008
//#define MUXCTRL_REG37	0x12040094
#define MUXCTRL_REG62 	0x120400F8

//管脚驱动能力寄存器
#define PADCTRL_REG70	0x12040918		//GPIO0_1管脚驱动能力寄存器。
#define PADCTRL_REG71	0x1204091C		//GPIO0_2管脚驱动能力寄存器。



#endif 
 

