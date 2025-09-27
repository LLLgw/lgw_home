#include "./BSP/JDQ/jdq.h"

void jdq_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    
    JDQ_GPIO_CLK_ENABLE();                               

    gpio_init_struct.Pin = JDQ_GPIO_PIN;                  
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;           
    gpio_init_struct.Pull = GPIO_PULLUP;                   
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        
    HAL_GPIO_Init(JDQ_GPIO_PORT, &gpio_init_struct);     
                                                  
    JDQ(0);
}
