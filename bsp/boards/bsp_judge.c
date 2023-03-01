#include "bsp_judge.h"
#include "judge.h"
#include "string.h"

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;

#define JUDGE_HUART              huart6
#define JUDGE_DMA             hdma_usart6_rx
#define JUDGE_UART              USART6
#define JUDGE_BOUND             115200
#define JUDGE_GPIO_AF           GPIO_AF_USART2
#define JUDGE_UART_RCC_ENABLE() RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

#define JUDGE_TX_PORT           GPIOD
#define JUDGE_TX_PIN            GPIO_Pin_5
#define JUDGE_TX_PINSOURCE      GPIO_PinSource5
#define JUDGE_TX_RCC_ENABLE()   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//

#define JUDGE_RX_PORT           GPIOD
#define JUDGE_RX_PIN            GPIO_Pin_6
#define JUDGE_RX_PINSOURCE      GPIO_PinSource6
#define JUDGE_RX_RCC_ENABLE()   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

#define JUDGE_IRQn              USART6_IRQn
#define JUDGE_NVIC              1
#define JUDGE_IRQHandler        USART6_IRQHandler

#define JUDGE_DMA_Stream        DMA2_Stream1
#define JUDGE_DMA_Channel       DMA_Channel_4
#define JUDGE_DMA_RCC_ENABLE()  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
/* 接收缓存 */
#define    JUDGE_BUFFER_LEN           200   //稍微给大点
uint8_t    JUDGE_get_Buffer[ JUDGE_BUFFER_LEN ] = {0};
uint8_t    JUDGE_get       [ JUDGE_BUFFER_LEN ] = {0};
int16_t    JUDGE_Clean_IDLE_Flag = 0;

//receive data, 18 bytes one frame, but set 36 bytes 
//接收原始数据，为18个字节，给了36个字节长度，防止DMA传输越界
static uint8_t judge_rx_buf[2][JUDGE_BUFFER_LEN];


void judge_control_init(void)
{
    judge_init(judge_rx_buf[0], judge_rx_buf[1], JUDGE_BUFFER_LEN);
}

void judge_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num)
{
    //enable the DMA transfer for the receiver request
    //使能DMA串口接收
    SET_BIT(JUDGE_HUART.Instance->CR3, USART_CR3_DMAR);

    //enalbe idle interrupt
    //使能空闲中断
    __HAL_UART_ENABLE_IT(&JUDGE_HUART, UART_IT_IDLE);

    //disable DMA
    //失效DMA
    __HAL_DMA_DISABLE(&JUDGE_DMA);
    while(JUDGE_DMA.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&JUDGE_DMA);
    }

    JUDGE_DMA.Instance->PAR = (uint32_t) & (JUDGE_UART->DR);
    //memory buffer 1
    //内存缓冲区1
    JUDGE_DMA.Instance->M0AR = (uint32_t)(rx1_buf);
    //memory buffer 2
    //内存缓冲区2
    JUDGE_DMA.Instance->M1AR = (uint32_t)(rx2_buf);
    //data length
    //数据长度
    JUDGE_DMA.Instance->NDTR = dma_buf_num;
    //enable double memory buffer
    //使能双缓冲区
    SET_BIT(JUDGE_DMA.Instance->CR, DMA_SxCR_DBM);

    //enable DMA
    //使能DMA
    __HAL_DMA_ENABLE(&JUDGE_DMA);

}
//串口中断
void JUDGE_IRQHandler(void)
{
    if(JUDGE_HUART.Instance->SR & UART_FLAG_RXNE)//接收到数据
    {
        __HAL_UART_CLEAR_PEFLAG(&JUDGE_HUART);
    }
    else if(JUDGE_UART->SR & UART_FLAG_IDLE)
    {
        static uint16_t this_time_rx_len = 0;

        __HAL_UART_CLEAR_PEFLAG(&JUDGE_HUART);

        if ((JUDGE_DMA.Instance->CR & DMA_SxCR_CT) == RESET)
        {
            /* Current memory buffer used is Memory 0 */
    
            //disable DMA
            //失效DMA
            __HAL_DMA_DISABLE(&JUDGE_DMA);

            //get receive data length, length = set_data_length - remain_length
            //获取接收数据长度,长度 = 设定长度 - 剩余长度
            this_time_rx_len = JUDGE_BUFFER_LEN - JUDGE_DMA.Instance->NDTR;

            //reset set_data_lenght
            //重新设定数据长度
            JUDGE_DMA.Instance->NDTR = JUDGE_BUFFER_LEN;

            //set memory buffer 1
            //设定缓冲区1
            JUDGE_DMA.Instance->CR |= DMA_SxCR_CT;
            
            //enable DMA
            //使能DMA
            __HAL_DMA_ENABLE(&JUDGE_DMA);

//            if(this_time_rx_len == RC_FRAME_LENGTH)
//            {
                Judge_Read_Data(judge_rx_buf[0]);
//            }
        }
        else
        {
            /* Current memory buffer used is Memory 1 */
            //disable DMA
            //失效DMA
            __HAL_DMA_DISABLE(&JUDGE_DMA);

            //get receive data length, length = set_data_length - remain_length
            //获取接收数据长度,长度 = 设定长度 - 剩余长度
            this_time_rx_len = JUDGE_BUFFER_LEN - JUDGE_DMA.Instance->NDTR;

            //reset set_data_lenght
            //重新设定数据长度
            JUDGE_DMA.Instance->NDTR = JUDGE_BUFFER_LEN;

            //set memory buffer 0
            //设定缓冲区0
            JUDGE_DMA_Stream->CR &= ~(DMA_SxCR_CT);
            
            //enable DMA
            //使能DMA
            __HAL_DMA_ENABLE(&JUDGE_DMA);

//            if(this_time_rx_len == RC_FRAME_LENGTH)
//            {
                //处理遥控器数据
                Judge_Read_Data(judge_rx_buf[1]);
//            }
        }
    }
}

