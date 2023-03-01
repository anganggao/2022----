#include "main.h"
#include "can.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include "pid.h"
#include "CAN_receive.h"
#include "remote_control.h"
#include "bsp_can.h"
#include "cmsis_os.h"
#include "chassis_task.h"
#include "bujin_task.h"
#include "bsp_debug.h"

chassis_t chassis;

//int aa[4]; //打波型

fp32 motorpid[3] = {15, 0 ,0.2}; // 15, 2, 0.2

fp32 chassis_motor_0_pid[3]={5,0,0}; //电机3 kp ki kd

extern  int a; 


void Chassis_pid_init()//电机初始化
{
		PID_init(&chassis.motor1pid_t, PID_POSITION, motorpid, 16000, 2000);
		PID_init(&chassis.motor2pid_t, PID_POSITION, motorpid, 16000, 2000);
		PID_init(&chassis .chassis_pid[1],PID_POSITION ,chassis_motor_0_pid ,16000,2000);
		PID_init(&chassis .chassis_pid[2],PID_POSITION ,chassis_motor_0_pid ,16000,2000);
        PID_init(&chassis .chassis_pid[3],PID_POSITION ,chassis_motor_0_pid ,16000,2000);
}

void chassis_get(void)   //获取数据
{
		chassis.chassis_RC = get_remote_control_point();              //获取遥控的数据

		chassis .rc_1	= chassis .chassis_RC ->rc .ch [1];
	    chassis .rc_2	= chassis .chassis_RC ->rc .ch [2];
	    chassis .rc_3	= chassis .chassis_RC ->rc .ch [3];

		chassis .motor1_measure = get_chassis_motor_measure_point(0);
	    chassis .motor2_measure = get_chassis_motor_measure_point(1);
		chassis .motor_measure  = get_trigger_motor_measure_point();  //获取电机3的数据
		chassis .motor_measure  = get_trigger_motor_measure_point();  //获取电机3的数据
		chassis .motor_measure  = get_yaw_gimbal_motor_measure_point();  //获取电机4的数据
	
		chassis.speed[1]= chassis .motor_measure ->speed_rpm ;		  //将YAW的转速赋值给chassis.speed[1]，作为YAW的速度
		chassis.speed[2]= chassis .motor_measure ->speed_rpm ;        //将电机3的转速赋值给chassis.speed[2]，作为电机3的速度
	    chassis.speed[3]= chassis .motor_measure ->speed_rpm ;        //将电机3的转速赋值给chassis.speed[3]，作为电机4的速度
}


void chassis_set(void)  //设置目标值
{
	
		chassis.speed_set[1] = chassis .chassis_RC ->rc .ch[2]*4;      //将遥控通道2的遥感值赋值给chassis.speed[1]，作为YAW的目标速度
		chassis.speed_set[2] = chassis .chassis_RC ->rc .ch[1]*0.5;      //将遥控通道1的遥感值赋值给chassis.speed[2]，作为电机3的目标速度	
		chassis.speed_set[3] = chassis .chassis_RC ->rc .ch[0]*1;      //将遥控通道0的遥感值赋值给chassis.speed[3]，作为电机4的目标速度

}


void chassis_pid_cale(void)  //PID计算电流值
{
	    PID_calc(&chassis.motor1pid_t, chassis .motor1_measure->speed_rpm, 1000);	 //7330	 7060  5.19测试 6895 7920 5.20 7810（前哨战） 5.27 
	    PID_calc(&chassis.motor2pid_t, chassis .motor2_measure->speed_rpm, -500);   //完整形态考核7000  6685           -6895
		PID_calc(&chassis.chassis_pid[1] ,chassis .speed[1] ,chassis .speed_set[1]);  //YAW的PID结构体，YAW的实际速度，目标速度
		PID_calc(&chassis.chassis_pid[2] ,chassis .speed[2] ,chassis .speed_set[2]);  //电机3的PID结构体，电机3的实际速度，目标速度
	    PID_calc(&chassis.chassis_pid[3] ,chassis .speed[3] ,chassis .speed_set[3]);  //电机3的PID结构体，电机3的实际速度，目标速度
	
}



void chassis_TASK(void const * argument) //任务
{
	
	Chassis_pid_init();
	
	while(1)
	{
		chassis_get();         //获取遥控、电机的数据
		chassis_set();         //给电机的目标速度赋值
		chassis_pid_cale();    //PID计算出电流值
//		Dartsframe_choose();
//		if(a == 1)
//		{
//			 PID_calc(&chassis.motor1pid_t, chassis .motor1_measure->speed_rpm, 6850);	 //73W30	 7060  5.19测试 6895 7920 5.20 7810（前哨战） 5.27 
//	         PID_calc(&chassis.motor2pid_t, chassis .motor2_measure->speed_rpm, -6850);   //完整形态考核7000  6685           -6895
//			 CAN_cmd_chassis(chassis.motor1pid_t.out, chassis.motor2pid_t.out, chassis. chassis_pid[2].out, chassis.chassis_pid[3] .out);//通过CAN线发送电流值 3508 3508 2006 0

//		}
//		
//		
//		if(a == 2)
//		{
//			 PID_calc(&chassis.motor1pid_t, chassis .motor1_measure->speed_rpm, 0);	 //7330	 7060  5.19测试 6895 7920 5.20 7810（前哨战） 5.27 
//	         PID_calc(&chassis.motor2pid_t, chassis .motor2_measure->speed_rpm, 0);   //完整形态考核7000  6685           -6895
//			 CAN_cmd_chassis(chassis.motor1pid_t.out, chassis.motor2pid_t.out, chassis. chassis_pid[2].out, chassis.chassis_pid[3] .out);//通过CAN线发送电流值 3508 3508 2006 0

//		}
		
		CAN_cmd_chassis(chassis.motor1pid_t.out, chassis.motor2pid_t.out, chassis. chassis_pid[2].out, chassis.chassis_pid[3] .out);//通过CAN线发送电流值 3508 3508 2006 0
		osDelay(2);
//		CAN_cmd_gimbal(chassis.chassis_pid[1] .out,0,0,0);
		
//		aa[0]=chassis .motor1_measure->speed_rpm; //打波形 
//		aa[1]=chassis .motor2_measure->speed_rpm;
//		aa[2]=7130;
//		aa[3]=-7130;
//		SendWaveform((uint8_t*)aa,sizeof(aa));
//		
//		osDelay(2);
		
	}		
}
