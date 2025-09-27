#include "./BSP/LCD/lcd.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/OV2640/ov2640.h"
#include "./SYSTEM/usart/usart.h"

DCMI_HandleTypeDef g_dcmi_handle;      
DMA_HandleTypeDef g_dma_dcmi_handle;   
               

void (*dcmi_inter_callback)(void);
/**
 * @brief       DCMI 初始化
 *   @note      IO对应关系如下:
 *              摄像头模块 ------------  STM32开发板
 *               OV_D0~D7  ------------  PC6/PC7/PC8/PC9/PC11/PB6/PE5/PE6
 *               OV_SCL    ------------  PD6
 *               OV_SDA    ------------  PD7
 *               OV_VSYNC  ------------  PB7
 *               OV_HREF   ------------  PA4
 *               OV_PCLK   ------------  PA6
 *               OV_PWDN   ------------  PG9
 *               OV_RESET  ------------  PG15
 *               OV_XCLK   ------------  PA8
 *              本函数仅初始化OV_D0~D7/OV_VSYNC/OV_HREF/OV_PCLK等信号(11个).
 * @param       无
 * @retval      无
 */
void dcmi_init(void)
{
    g_dcmi_handle.Instance = DCMI;
    g_dcmi_handle.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;     /* 硬件同步HSYNC,VSYNC */
    g_dcmi_handle.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;   /* PCLK  上升沿有效 */
    g_dcmi_handle.Init.VSPolarity = DCMI_VSPOLARITY_LOW;        /* VSYNC 低电平有效 */
    g_dcmi_handle.Init.HSPolarity = DCMI_HSPOLARITY_LOW;        /* HSYNC 低电平有效 */
    g_dcmi_handle.Init.CaptureRate = DCMI_CR_ALL_FRAME;         /* 全帧捕获 */
    g_dcmi_handle.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;  /* 8位数据格式 */
    HAL_DCMI_Init(&g_dcmi_handle);                              /* 初始化DCMI，此函数会开启帧中断 */

    /* 关闭行中断、VSYNC中断、同步错误中断和溢出中断 */
//    __HAL_DCMI_DISABLE_IT(&g_dcmi_handle, DCMI_IT_LINE | DCMI_IT_VSYNC | DCMI_IT_ERR | DCMI_IT_OVR);

    /** 关闭所有中断，函数HAL_DCMI_Init()会默认打开很多中断，开启这些中断
     * 以后我们就需要对这些中断做相应的处理，否则的话就会导致各种各样的问题，
     * 但是这些中断很多都不需要，所以这里将其全部关闭掉，也就是将IER寄存器清零。
     * 关闭完所有中断以后再根据自己的实际需求来使能相应的中断
     */
    DCMI->IER = 0x0;

    __HAL_DCMI_ENABLE_IT(&g_dcmi_handle, DCMI_IT_FRAME);        /* 使能帧中断 */
    __HAL_DCMI_ENABLE(&g_dcmi_handle);                          /* 使能DCMI */
}

/**
 * @brief       DCMI底层驱动，引脚配置，时钟使能，中断配置
 * @param       hdcmi:DCMI句柄
 * @note        此函数会被HAL_DCMI_Init()调用
 * @retval      无
 */
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
    GPIO_InitTypeDef gpio_init_struct;
 
    __HAL_RCC_DCMI_CLK_ENABLE();                /* 使能DCMI时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();               /* 使能GPIOA时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();               /* 使能GPIOB时钟 */
    __HAL_RCC_GPIOC_CLK_ENABLE();               /* 使能GPIOC时钟 */
    __HAL_RCC_GPIOE_CLK_ENABLE();               /* 使能GPIOE时钟 */
    
    gpio_init_struct.Pin = GPIO_PIN_4 | GPIO_PIN_6;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* 推挽复用 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* 高速 */
    gpio_init_struct.Alternate = GPIO_AF13_DCMI;        /* 复用为DCMI */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);            /* 初始化PA4，6引脚 */

    gpio_init_struct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);            /* 初始化PB6,7引脚 */

    gpio_init_struct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);            /* 初始化PC6,7,8,9,11引脚 */

    gpio_init_struct.Pin = GPIO_PIN_5 | GPIO_PIN_6; 
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);            /* 初始化PE5,6引脚 */

    HAL_NVIC_SetPriority(DCMI_IRQn, 7, 2);              /* 抢占优先级2，子优先级2 */
    HAL_NVIC_EnableIRQ(DCMI_IRQn);                      /* 使能DCMI中断 */
}


//dma_dcmi配置

void dcmi_dma_init(uint32_t mem0addr, uint32_t mem1addr, uint16_t memsize, uint32_t memblen, uint32_t meminc)
{ 
    __HAL_RCC_DMA2_CLK_ENABLE();                                        /* 使能DMA2时钟 */
    __HAL_LINKDMA(&g_dcmi_handle, DMA_Handle, g_dma_dcmi_handle);       /* 将DMA与DCMI联系起来 */
    __HAL_DMA_DISABLE_IT(&g_dma_dcmi_handle, DMA_IT_TC);                /* 先关闭DMA传输完成中断(否则在使用MCU屏的时候会出现花屏的情况) */

    g_dma_dcmi_handle.Instance = DMA2_Stream1;                          /* DMA2数据流1 */
    g_dma_dcmi_handle.Init.Channel = DMA_CHANNEL_1;                     /* DCMI的DMA请求 */
    g_dma_dcmi_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;            /* 外设到存储器 */
    g_dma_dcmi_handle.Init.PeriphInc = DMA_PINC_DISABLE;                /* 外设非增量模式 */
    g_dma_dcmi_handle.Init.MemInc = meminc;                             /* 存储器增量模式 */
    g_dma_dcmi_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;   /* 外设数据长度:32位 */
    g_dma_dcmi_handle.Init.MemDataAlignment = memblen;                  /* 存储器数据长度:8/16/32位 */
    g_dma_dcmi_handle.Init.Mode = DMA_CIRCULAR ;                         /* 使用循环模式 */
    g_dma_dcmi_handle.Init.Priority = DMA_PRIORITY_HIGH;                /* 高优先级 */
    g_dma_dcmi_handle.Init.FIFOMode = DMA_FIFOMODE_ENABLE;              /* 使能FIFO */
    g_dma_dcmi_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL; /* 使用1/2的FIFO */
    g_dma_dcmi_handle.Init.MemBurst = DMA_MBURST_SINGLE;                /* 存储器突发传输 */
    g_dma_dcmi_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;             /* 外设突发单次传输 */
    HAL_DMA_DeInit(&g_dma_dcmi_handle);                                 /* 先清除以前的设置 */
    HAL_DMA_Init(&g_dma_dcmi_handle);                                   /* 初始化DMA */
    
    /* 在开启DMA之前先使用__HAL_UNLOCK()解锁一次DMA,因为HAL_DMA_Statrt()HAL_DMAEx_MultiBufferStart()
     * 这两个函数一开始要先使用__HAL_LOCK()锁定DMA,而函数__HAL_LOCK()会判断当前的DMA状态是否为锁定状态，如果是
     * 锁定状态的话就直接返回HAL_BUSY，这样会导致函数HAL_DMA_Statrt()和HAL_DMAEx_MultiBufferStart()后续的DMA配置
     * 程序直接被跳过！DMA也就不能正常工作，为了避免这种现象，所以在启动DMA之前先调用__HAL_UNLOCK()先解锁一次DMA。
     */
    __HAL_UNLOCK(&g_dma_dcmi_handle);
   
		 HAL_DMA_Start(&g_dma_dcmi_handle, (uint32_t)&DCMI->DR, mem0addr, memsize);
		 
}

//启动dma和dcmi
void dcmi_start(void)
{
    __HAL_DMA_ENABLE(&g_dma_dcmi_handle);   /* 使能DMA */
    DCMI->CR |= DCMI_CR_CAPTURE;            /* DCMI捕获使能 */
}

//关闭dma和dcmi
void dcmi_stop(void)
{ 
    DCMI->CR &= ~(DCMI_CR_CAPTURE);         /* DCMI捕获关闭 */

    while (DCMI->CR & 0X01);                /* 等待传输结束 */

    __HAL_DMA_DISABLE(&g_dma_dcmi_handle);  /* 关闭DMA */
}

//dcmi中断函数
void DCMI_IRQHandler(void)
{
    HAL_DCMI_IRQHandler(&g_dcmi_handle);
}

//dcmi帧回调函数
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    __HAL_DCMI_CLEAR_FLAG(&g_dcmi_handle, DCMI_FLAG_FRAMERI);    /* 清除帧中断 */
	  dcmi_inter_callback();
	
  __HAL_DCMI_ENABLE_IT(&g_dcmi_handle, DCMI_IT_FRAME);
    
}



//void DMA2_Stream1_IRQHandler(void)
//{    
//    if (__HAL_DMA_GET_FLAG(&g_dma_dcmi_handle, DMA_FLAG_TCIF1_5) != RESET)  /* DMA传输完成 */
//    {
//        __HAL_DMA_CLEAR_FLAG(&g_dma_dcmi_handle, DMA_FLAG_TCIF1_5); /* 清除DMA传输完成中断标志位 */ 
//				dma_inter_callback();
//		} 
//}








