#ifndef _CHASSIS_TASK_H_
#define _CHASSIS_TASK_H_

#include "main.h"
#include "cmsis_os.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "pid.h"
#include "CAN_receive.h"
#include "remote_control.h"
#include "bsp_can.h"



//pid_type_def motor1pid_t;
//pid_type_def motor2pid_t;
//pid_type_def motor3pid_t;
//pid_type_def motor4pid_t;

//const motor_measure_t* motor1_measure;
//const motor_measure_t* motor2_measure;
//const motor_measure_t* motor3_measure;
//const motor_measure_t* motor4_measure;


typedef  struct 
{   const RC_ctrl_t        *gimbal_rc_ctrl;
	const RC_ctrl_t        *gimbal_RC;	
	const RC_ctrl_t        *chassis_RC;     //����ң�صĽṹ��
	
	
	const motor_measure_t  *motor_measure;  //���̵����Ϣ�Ľṹ��
	const motor_measure_t  *motor1_measure;
    const motor_measure_t  *motor2_measure;

	pid_type_def           motor1pid_t;
	pid_type_def  		   motor2pid_t;

	pid_type_def           chassis_pid[4];  //�����ĸ������PID�ṹ��
	
	fp32                      rc_0;
    fp32                      rc_1;	
	fp32                      rc_2;
    fp32                      rc_3;
	fp32 speed[4];               					 //�����ĸ�������ٶ�ʵ��ֵ
	fp32 speed_set[4];           					 //�����ĸ�������ٶ�����ֵ
	
}chassis_t; 

extern void chassis_TASK(void const * argument);

extern void chassis_get(void);

extern void chassis_get(void); 


#endif
