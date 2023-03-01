#ifndef _BUJIN_TASK_H_
#define _BUJIN_TASK_H_

#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "pid.h"
#include "remote_control.h"
#include "bsp_can.h"




extern void Dartsframe_choose(void);//Ò£¿ØÆ÷Ñ¡Ôñ

extern void bujin_TASK(void const * argument);



#endif
