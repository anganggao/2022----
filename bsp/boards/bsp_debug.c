#include "bsp_debug.h"
#include "stdio.h"
#include "string.h"

/*
	注意：移植时注意每个部分的修改
*/
#define DEBUG_UART              USART1
#define DEBUG_BOUND             9600
#define DEBUG_GPIO_AF           GPIO_AF_USART6
#define DEBUG_UART_RCC_ENABLE() RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

#define DEBUG_TX_PORT           GPIOG 
#define DEBUG_TX_PIN            GPIO_Pin_14 
#define DEBUG_TX_PINSOURCE      GPIO_PinSource14
#define DEBUG_TX_RCC_ENABLE()   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

#define DEBUG_RX_PORT           GPIOG
#define DEBUG_RX_PIN            GPIO_Pin_9
#define DEBUG_RX_PINSOURCE      GPIO_PinSource9
#define DEBUG_RX_RCC_ENABLE()   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

#define DEBUG_IRQn              USART6_IRQn
#define DEBUG_NVIC              3
#define DEBUG_IRQHandler        USART1_IRQHandler
int REs;
extern UART_HandleTypeDef huart1;
void DEBUG_IRQHandler(void)
{
	//接收中断
//	if(USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) != RESET) 
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)	
	{
//		REs = USART_ReceiveData(DEBUG_UART);
//		HAL_UART_Receive(&huart6,);
		REs=(uint16_t)(USART1->DR & (uint16_t)0x01FF);
	} 
	//清除中断标志
//	USART_ClearFlag(DEBUG_UART, USART_IT_RXNE);
} 	
//printf重定义
int fputc(int ch, FILE *f)
{ 	
	while((DEBUG_UART->SR&0X40)==0);//循环发送,直到发送完毕   
	DEBUG_UART->DR = (uint8_t) ch;      
	return ch;
}
//山外上位机
/*
用法 例：int16_t a[2];
		 SendWaveform((uint8_t*)a,sizeof(a));
*/
void SendWaveform(uint8_t *send, uint8_t size)
{
	DEBUG_UART->DR = 0x03; 
	while((DEBUG_UART->SR&0X40)==0);
	DEBUG_UART->DR = 0xfc; 
	while((DEBUG_UART->SR&0X40)==0);
	while(size--)
	{
		DEBUG_UART->DR = *send; 
		while((DEBUG_UART->SR&0X40)==0);	
		send ++;
	}
	DEBUG_UART->DR = 0xfc; 
	while((DEBUG_UART->SR&0X40)==0);
	DEBUG_UART->DR = 0x03; 
	while((DEBUG_UART->SR&0X40)==0);
}

