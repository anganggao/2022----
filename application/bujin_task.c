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

extern chassis_t chassis;

int a = 0; //��ʶλ ������Ħ��������

//void delay()
//{
//	int b = 0;
//	int c = 0;
//	
//	for(b = 0;b < 10000;b++)
//	{
//		while()
//	}
//	
//}


void pull_1()//��һ�������Ƹ�
{
	     int num_pul_5=0;
	     int num_rate_5=0;
			
	a = 1;
	
	osDelay(400);
	
//	delay();
	     
			
			 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//�����Ƹ�

			for (num_pul_5=0;num_pul_5 <16500;num_pul_5++) //15000
					{
					  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
					 while(num_rate_5<8000){num_rate_5++;}       //7000
					 num_rate_5=0;
					 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
					 while(num_rate_5<8000){num_rate_5++;}
					 num_rate_5=0;
					 
					}
					
}	

void pull_2()//��һ�������Ƹ�
{
	     osDelay(200);
	
	     int num_pul_5=0;
	     int num_rate_5=0;
	    
	    	a = 2;
			
			 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//�����Ƹ�

			for (num_pul_5=0;num_pul_5 <16500;num_pul_5++)
					{
					  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
					 while(num_rate_5<8000){num_rate_5++;}
					 num_rate_5=0;
					 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
					 while(num_rate_5<8000){num_rate_5++;}
					 num_rate_5=0;
					 
					}
				
					
}	

void pull_3()//���ҵ�� 
{
	     int num_pul_5=0;
	     int num_rate_5=0;
	    
	     
			
			 HAL_GPIO_WritePin(GPIOF,GPIO_PIN_1,GPIO_PIN_RESET);//�����Ƹ�

			for (num_pul_5=0;num_pul_5 <4430;num_pul_5++)  //4430(ԭֵ)
					{
					  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_SET);
					 while(num_rate_5<6000){num_rate_5++;}
					 num_rate_5=0;
					 HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_RESET);
					 while(num_rate_5<6000){num_rate_5++;}
					 num_rate_5=0;
					 
					}
					
}	

void pull_4()//���ҵ�� 
{
	     int num_pul_5=0;
	     int num_rate_5=0;
	    
	     
			
			 HAL_GPIO_WritePin(GPIOF,GPIO_PIN_1,GPIO_PIN_SET);//�����Ƹ�

			for (num_pul_5=0;num_pul_5 <4430;num_pul_5++)
					{
					  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_SET);
					 while(num_rate_5<6000){num_rate_5++;}
					 num_rate_5=0;
					 HAL_GPIO_WritePin(GPIOF,GPIO_PIN_0,GPIO_PIN_RESET);
					 while(num_rate_5<6000){num_rate_5++;}
					 num_rate_5=0;
					 
					}
			
					
}	

void pull_5()//
{
	     int num_pul_5=0;
	     int num_rate_5=0;
	    
	     
			
			 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//�����Ƹ�

			for (num_pul_5=0;num_pul_5 <10;num_pul_5++)
					{
					  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
					 while(num_rate_5<7000){num_rate_5++;}
					 num_rate_5=0;
					 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
					 while(num_rate_5<7000){num_rate_5++;}
					 num_rate_5=0;
					 
					}
					
}



void Dartsframe_choose()//ң����ѡ��
{

/*������*/ 
//	if((chassis .chassis_RC ->rc .s [1] == 1 && chassis .chassis_RC ->rc .s [0] == 1))   //S1=1 S0=1 ˿�������ƶ�
//        {
				
//				a = 1 ;
//				pull_1();	
//				pull_2();
//			
//			    pull_3();
//			
//				pull_1();	
//				pull_2();
//				
//			    pull_4();
				
//				a = 0;
//        }
//	if((chassis .chassis_RC ->rc .s [1] == 1 && chassis .chassis_RC ->rc .s [0] == 3))   //
//        {
//				pull_5();
//			
//        }
	
	/*�Կ���*/
	if( chassis .chassis_RC ->rc .s [0] == 1)   
        {
				pull_1();	
				pull_2();
			
			    pull_3();
					
        }
		
	if( chassis .chassis_RC ->rc .s [1] == 1)   
		{
				
				pull_1();	
				pull_2();
				
			    pull_4();

        }
		
	if(chassis .chassis_RC ->rc .s [0] == 3)   
        {
				pull_5();
			
        }
		
	if(chassis .chassis_RC ->rc .s [1] == 3)   
        {
				pull_5();
		
        }
	
}


void bujin_TASK(void const * argument)
{

  while (1)
  {
	  chassis_get(); 
	  Dartsframe_choose();
	  osDelay(2);
	  
  }

}

