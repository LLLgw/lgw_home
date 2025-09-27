#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./USMART/usmart.h"
#include "./BSP/SRAM/sram.h"
#include "./MALLOC/malloc.h"
#include "freertos_demo.h"
#include "./BSP/ov2640/ov2640.h"
#include "./BSP/ESP8266/esp8266.h"
#include "./BSP/LNA226/bsp_ina226.h"
#include "./BSP/BH1750/BH1750.h"
#include "./BSP/DHT11/dht11.h"
#include "./BSP/USART2/usart2.h"

//esp8266初始化完成标志
uint8_t finish_link=0;

//选择图片大小
#define picture_size 2

//图片大小
const uint16_t jpeg_img_size_tbl[][2] =
{
    160, 120,       
    176, 144,       
    320, 240,       
    400,240,        
    352,288,        
    640, 480,       		
    800, 600,       
    1024, 768,      
    1280, 800,      
    1280, 960,      
    1440, 900,      
    1280, 1024,     
    1600, 1200,     
};


int main(void)
{
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(336, 8, 2, 7); /* 设置时钟,168Mhz */
    delay_init(168);                    /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    sram_init();                        /* SRAM初始化 */
    
    my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */
    my_mem_init(SRAMEX);                /* 初始化外部SRAM内存池 */
    //my_mem_init(SRAMCCM);               /* 初始化内部CCM内存池 */
	  
		while (ov2640_init())       
		{
			printf("ov2640初始化失败\r\n");
		}
		ov2640_flash_intctrl();     //闪光灯控制 
		ov2640_jpeg_mode();         //选择jpeg格式
		ov2640_outsize_set(jpeg_img_size_tbl[picture_size][0], jpeg_img_size_tbl[picture_size][1]);
		printf("摄像头初始化完成\r\n");
			
		freertos_demo();                    /* 创建lwIP的任务函数 */
}
