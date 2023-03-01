#ifndef _BSP_JUDGE_H
#define _BSP_JUDGE_H

#include "main.h"

//#define    COM5_BUFFER_NUM           5
#define    JUDGE_BUFFER_LEN           200

extern uint8_t  JUDGE_get_Buffer[ JUDGE_BUFFER_LEN ];
extern uint8_t  JUDGE_get[ JUDGE_BUFFER_LEN ];
void judge_uart_init( void );
void judge_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);
void judge_control_init(void);

//uint8_t *pucCom5ReadBuffer( void );
//void UART2_SendChar( uint8_t cData );

//void UART5_UpdateMemoryAddr( DMA_InitTypeDef *xDMAInit, uint8_t *buffer );


#endif

