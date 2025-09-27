#include "./BSP/USART2/usart2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"

UART_HandleTypeDef g_uart3_handle;      //UART句柄
DMA_HandleTypeDef  g_dma_uart3_handler; //dma句柄
extern unsigned char * esp8266_buf;     //esp8266缓冲区
extern uint8_t finish_link;
extern TaskHandle_t json_style_task_handle;

void usart3_init(uint32_t baudrate)
{
    GPIO_InitTypeDef gpio_init_struct;

    USART3_UX_CLK_ENABLE();                                     /* USART3 时钟使能 */
    USART3_TX_GPIO_CLK_ENABLE();                                /* 发送引脚时钟使能 */
    USART3_RX_GPIO_CLK_ENABLE();                                /* 接收引脚时钟使能 */
	  __HAL_RCC_DMA1_CLK_ENABLE();

    gpio_init_struct.Pin = USART3_TX_GPIO_PIN;                  /* TX引脚 */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
    gpio_init_struct.Alternate = USART3_TX_GPIO_AF;             /* 复用为USART1 */
    HAL_GPIO_Init(USART3_TX_GPIO_PORT, &gpio_init_struct);      /* 初始化发送引脚 */

    gpio_init_struct.Pin = USART3_RX_GPIO_PIN;                  /* RX引脚 */
    gpio_init_struct.Alternate = USART3_RX_GPIO_AF;             /* 复用为USART3 */
    HAL_GPIO_Init(USART3_RX_GPIO_PORT, &gpio_init_struct);      /* 初始化接收引脚 */
    
    g_uart3_handle.Instance = USART3_UX;                        /* USART3 */
    g_uart3_handle.Init.BaudRate = baudrate;                    /* 波特率 */
    g_uart3_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* 字长为8位数据格式 */
    g_uart3_handle.Init.StopBits = UART_STOPBITS_1;             /* 一个停止位 */
    g_uart3_handle.Init.Parity = UART_PARITY_NONE;              /* 无奇偶校验位 */
    g_uart3_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* 无硬件流控 */
    g_uart3_handle.Init.Mode = UART_MODE_TX_RX;                 /* 收发模式 */
	  g_uart3_handle.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&g_uart3_handle); 
	
    //空闲中断配置	
		__HAL_UART_ENABLE_IT(&g_uart3_handle,UART_IT_IDLE);
		HAL_NVIC_EnableIRQ(USART3_IRQn);                      
		HAL_NVIC_SetPriority(USART3_IRQn, 6, 1);
	
	
		//DMA配置
		g_dma_uart3_handler.Instance=DMA1_Stream1;
		g_dma_uart3_handler.Init.Channel=DMA_CHANNEL_4;
		g_dma_uart3_handler.Init.Direction=DMA_PERIPH_TO_MEMORY;
		g_dma_uart3_handler.Init.FIFOMode=DMA_FIFOMODE_DISABLE;
		g_dma_uart3_handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_FULL;
		g_dma_uart3_handler.Init.MemBurst=DMA_MBURST_SINGLE;
		g_dma_uart3_handler.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;
		g_dma_uart3_handler.Init.MemInc=DMA_MINC_ENABLE;
		g_dma_uart3_handler.Init.Mode=DMA_NORMAL;
		g_dma_uart3_handler.Init.PeriphBurst=DMA_PBURST_SINGLE;
		g_dma_uart3_handler.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
		g_dma_uart3_handler.Init.PeriphInc=DMA_PINC_DISABLE;
		g_dma_uart3_handler.Init.Priority=DMA_PRIORITY_HIGH;
		

		__HAL_LINKDMA(&g_uart3_handle,hdmarx,g_dma_uart3_handler);
		HAL_DMA_DeInit(&g_dma_uart3_handler);
		HAL_DMA_Init(&g_dma_uart3_handler);

		HAL_UART_Init(&g_uart3_handle);
		HAL_UARTEx_ReceiveToIdle_DMA(&g_uart3_handle,esp8266_buf,256);
}


//esp8266串口中断
void USART3_IRQHandler(void)
{ 
   if(__HAL_UART_GET_FLAG(&g_uart3_handle,UART_FLAG_IDLE)!=RESET)
	 {
		 __HAL_UART_CLEAR_IDLEFLAG(&g_uart3_handle);
		 g_uart3_handle.Instance->SR;
		 g_uart3_handle.Instance->DR;
		 
		 HAL_UART_DMAStop(&g_uart3_handle);
		 
		 if(finish_link==1)
		{
			//if(strstr((const char *)esp8266_buf,"+MQTTSUBRECV")!=NULL)
			  //xTaskResumeFromISR(json_style_task_handle);
		}
		 
		 HAL_UARTEx_ReceiveToIdle_DMA(&g_uart3_handle,esp8266_buf,256);
		 HAL_UART_IRQHandler(&g_uart3_handle);
	 }
	 
}

