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

//int aa[4]; //����

fp32 motorpid[3] = {15, 0 ,0.2}; // 15, 2, 0.2

fp32 chassis_motor_0_pid[3]={5,0,0}; //���3 kp ki kd

extern  int a; 


void Chassis_pid_init()//�����ʼ��
{
		PID_init(&chassis.motor1pid_t, PID_POSITION, motorpid, 16000, 2000);
		PID_init(&chassis.motor2pid_t, PID_POSITION, motorpid, 16000, 2000);
		PID_init(&chassis .chassis_pid[1],PID_POSITION ,chassis_motor_0_pid ,16000,2000);
		PID_init(&chassis .chassis_pid[2],PID_POSITION ,chassis_motor_0_pid ,16000,2000);
        PID_init(&chassis .chassis_pid[3],PID_POSITION ,chassis_motor_0_pid ,16000,2000);
}

void chassis_get(void)   //��ȡ����
{
		chassis.chassis_RC = get_remote_control_point();              //��ȡң�ص�����

		chassis .rc_1	= chassis .chassis_RC ->rc .ch [1];
	    chassis .rc_2	= chassis .chassis_RC ->rc .ch [2];
	    chassis .rc_3	= chassis .chassis_RC ->rc .ch [3];

		chassis .motor1_measure = get_chassis_motor_measure_point(0);
	    chassis .motor2_measure = get_chassis_motor_measure_point(1);
		chassis .motor_measure  = get_trigger_motor_measure_point();  //��ȡ���3������
		chassis .motor_measure  = get_trigger_motor_measure_point();  //��ȡ���3������
		chassis .motor_measure  = get_yaw_gimbal_motor_measure_point();  //��ȡ���4������
	
		chassis.speed[1]= chassis .motor_measure ->speed_rpm ;		  //��YAW��ת�ٸ�ֵ��chassis.speed[1]����ΪYAW���ٶ�
		chassis.speed[2]= chassis .motor_measure ->speed_rpm ;        //�����3��ת�ٸ�ֵ��chassis.speed[2]����Ϊ���3���ٶ�
	    chassis.speed[3]= chassis .motor_measure ->speed_rpm ;        //�����3��ת�ٸ�ֵ��chassis.speed[3]����Ϊ���4���ٶ�
}


void chassis_set(void)  //����Ŀ��ֵ
{
	
		chassis.speed_set[1] = chassis .chassis_RC ->rc .ch[2]*4;      //��ң��ͨ��2��ң��ֵ��ֵ��chassis.speed[1]����ΪYAW��Ŀ���ٶ�
		chassis.speed_set[2] = chassis .chassis_RC ->rc .ch[1]*0.5;      //��ң��ͨ��1��ң��ֵ��ֵ��chassis.speed[2]����Ϊ���3��Ŀ���ٶ�	
		chassis.speed_set[3] = chassis .chassis_RC ->rc .ch[0]*1;      //��ң��ͨ��0��ң��ֵ��ֵ��chassis.speed[3]����Ϊ���4��Ŀ���ٶ�

}


void chassis_pid_cale(void)  //PID�������ֵ
{
	    PID_calc(&chassis.motor1pid_t, chassis .motor1_measure->speed_rpm, 1000);	 //7330	 7060  5.19���� 6895 7920 5.20 7810��ǰ��ս�� 5.27 
	    PID_calc(&chassis.motor2pid_t, chassis .motor2_measure->speed_rpm, -500);   //������̬����7000  6685           -6895
		PID_calc(&chassis.chassis_pid[1] ,chassis .speed[1] ,chassis .speed_set[1]);  //YAW��PID�ṹ�壬YAW��ʵ���ٶȣ�Ŀ���ٶ�
		PID_calc(&chassis.chassis_pid[2] ,chassis .speed[2] ,chassis .speed_set[2]);  //���3��PID�ṹ�壬���3��ʵ���ٶȣ�Ŀ���ٶ�
	    PID_calc(&chassis.chassis_pid[3] ,chassis .speed[3] ,chassis .speed_set[3]);  //���3��PID�ṹ�壬���3��ʵ���ٶȣ�Ŀ���ٶ�
	
}



void chassis_TASK(void const * argument) //����
{
	
	Chassis_pid_init();
	
	while(1)
	{
		chassis_get();         //��ȡң�ء����������
		chassis_set();         //�������Ŀ���ٶȸ�ֵ
		chassis_pid_cale();    //PID���������ֵ
//		Dartsframe_choose();
//		if(a == 1)
//		{
//			 PID_calc(&chassis.motor1pid_t, chassis .motor1_measure->speed_rpm, 6850);	 //73W30	 7060  5.19���� 6895 7920 5.20 7810��ǰ��ս�� 5.27 
//	         PID_calc(&chassis.motor2pid_t, chassis .motor2_measure->speed_rpm, -6850);   //������̬����7000  6685           -6895
//			 CAN_cmd_chassis(chassis.motor1pid_t.out, chassis.motor2pid_t.out, chassis. chassis_pid[2].out, chassis.chassis_pid[3] .out);//ͨ��CAN�߷��͵���ֵ 3508 3508 2006 0

//		}
//		
//		
//		if(a == 2)
//		{
//			 PID_calc(&chassis.motor1pid_t, chassis .motor1_measure->speed_rpm, 0);	 //7330	 7060  5.19���� 6895 7920 5.20 7810��ǰ��ս�� 5.27 
//	         PID_calc(&chassis.motor2pid_t, chassis .motor2_measure->speed_rpm, 0);   //������̬����7000  6685           -6895
//			 CAN_cmd_chassis(chassis.motor1pid_t.out, chassis.motor2pid_t.out, chassis. chassis_pid[2].out, chassis.chassis_pid[3] .out);//ͨ��CAN�߷��͵���ֵ 3508 3508 2006 0

//		}
		
		CAN_cmd_chassis(chassis.motor1pid_t.out, chassis.motor2pid_t.out, chassis. chassis_pid[2].out, chassis.chassis_pid[3] .out);//ͨ��CAN�߷��͵���ֵ 3508 3508 2006 0
		osDelay(2);
//		CAN_cmd_gimbal(chassis.chassis_pid[1] .out,0,0,0);
		
//		aa[0]=chassis .motor1_measure->speed_rpm; //���� 
//		aa[1]=chassis .motor2_measure->speed_rpm;
//		aa[2]=7130;
//		aa[3]=-7130;
//		SendWaveform((uint8_t*)aa,sizeof(aa));
//		
//		osDelay(2);
		
	}		
}
