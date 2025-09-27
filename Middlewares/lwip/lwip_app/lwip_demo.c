#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdio.h>
#include <lwip/sockets.h>
#include "./MALLOC/malloc.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip_demo.h"
#include "queue.h"
#include "semphr.h"
#include "./BSP/DCMI/dcmi.h"
#include "./SYSTEM/delay/delay.h"

//socket参数
#define IP_ADDR             "192.168.110.166"   //远程ip地址  
#define LWIP_DEMO_PORT       8088   	          // 连接的本地端口号  
#define SEND_MAX_SIZE        1600								//如果使用分片发送,发送的数据大小
struct sockaddr_in client_info;                 //Socket地址信息结构体
int	g_sock_fd;                            			//定义一个Socket接口


//camera参数
#define big_buf_size      1500              		//缓冲区大小
__ALIGNED(4) uint32_t big_buf[big_buf_size] __attribute__((section("RAM1"))); //申请缓冲区
typedef_buf_state buf_state={0};            		//数据处理状态

//camera传输任务参数
#define CAMERA_TASK_PRIO     3     
#define CAMERA_STK_SIZE      2048  
TaskHandle_t CAMERA_Task_Handler;  

extern QueueHandle_t Semaphore1_handle;

//camera任务
void camera_transfer(void *pvParameters);

//dcmi帧中断回调函数
void dcmi_callback(void)
{
	if(buf_state.picture_ok==0)
	{	
		__HAL_DMA_DISABLE(&g_dma_dcmi_handle);
		while(DMA2_Stream1->CR&0x01);
		buf_state.buf_len=big_buf_size-__HAL_DMA_GET_COUNTER(&g_dma_dcmi_handle);
		buf_state.picture_ok=1;
		printf("buf_len:%d\r\n",buf_state.buf_len*4);
	}
}


void lwip_demo(void)
{   
	//socket连接服务器					
	err_t err;
	uint8_t mode=1;
				
  while(1)
		{
			client_info.sin_family = AF_INET;                    
			client_info.sin_port = htons(LWIP_DEMO_PORT);        
			client_info.sin_addr.s_addr = inet_addr(IP_ADDR);     
			g_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
			
	
		  memset(&client_info.sin_zero,0,sizeof(client_info.sin_zero));

			err = connect(g_sock_fd, (struct sockaddr *)&client_info, sizeof(struct sockaddr));
			
			lwip_ioctl(g_sock_fd,FIONBIO,&mode);
			
			if(err==-1)
			{
				printf("连接失败err:%d\r\n",errno);
				closesocket(g_sock_fd);
				vTaskDelay(10);
			}
			else
			{
				printf("连接成功\r\n");
				break;
			}
		}
		
	 //创建发送任务函数
	 xTaskCreate((TaskFunction_t)camera_transfer,
								(const char *  )"camera",
								(uint16_t      )CAMERA_STK_SIZE,
								(void *         )NULL,
								(UBaseType_t    )CAMERA_TASK_PRIO,
								(TaskHandle_t * )CAMERA_Task_Handler);
		
		//初始化dcmi
	  dcmi_init();
		dcmi_inter_callback=dcmi_callback;
		dcmi_dma_init((uint32_t)big_buf,0, big_buf_size, DMA_MDATAALIGN_WORD, DMA_MINC_ENABLE);
		dcmi_start();
}



//获取jpeg文件
void camera_transfer(void *pvParameters)
{
	uint32_t i=0;
	uint8_t * p=NULL;
	while(1)
	{ 
		if(buf_state.picture_ok==1)
		{
			buf_state.back_flag=0;
			buf_state.back_start=0;
			p=(uint8_t *)big_buf;
			
			//查找jpeg的头和尾
			for(i=0;i<buf_state.buf_len*4;i++)
			{
				if(p[i]==0xFF && p[i+1]==0xD8)
				{
					buf_state.back_start=i;
					buf_state.back_flag=1;
				}
				if(buf_state.back_flag!=0 && p[i]==0xFF &&p[i+1] == 0xD9)
				{
					buf_state.back_end=i;
					break;
				}
			}
			
			//计算jpeg文件的长度
			buf_state.back_len = buf_state.back_end - buf_state.back_start+2;
			p+=buf_state.back_start;
			
			//发送图像
			if(buf_state.back_len>4&&buf_state.back_len<118784)
			{
					taskENTER_CRITICAL(); 
					write(g_sock_fd,(char *)p,buf_state.back_len);
					taskEXIT_CRITICAL();
					vTaskDelay(5);
				  //xSemaphoreGive(Semaphore1_handle);
				//send_camera(p); //分片发送
			}
			
			buf_state.buf_len=buf_state.back_end=buf_state.back_len=buf_state.picture_ok=0;
			memset(big_buf,0,big_buf_size);
	
			__HAL_DMA_SET_COUNTER(&g_dma_dcmi_handle, big_buf_size);  
			__HAL_DMA_ENABLE(&g_dma_dcmi_handle);
	}
	 }
}

//						taskENTER_CRITICAL();
//						write(g_sock_fd,(char *)p,buf_state.back_len);
//						taskEXIT_CRITICAL();
//						//if(err<0) printf("发送失败\r\n");
//						vTaskDelay(6);


//分片发送jpeg文件
//void send_camera(uint8_t *p)
//{

//	uint8_t send_size_num;
//	
//	send_size_num=buf_state.back_len/SEND_MAX_SIZE+1;
//	buf_state.back_len=buf_state.back_len%SEND_MAX_SIZE;
//	buf_state.send_size=SEND_MAX_SIZE;
//	
//	while(send_size_num--)
//	{
//		if(send_size_num==0)
//		{
//			buf_state.send_size=buf_state.back_len;
//		}
//		
//		taskENTER_CRITICAL(); 
//		write(g_sock_fd,(char *)p,buf_state.send_size);
//		taskEXIT_CRITICAL();
//		
//		p+=buf_state.send_size;
//		
//		vTaskDelay(1);
//	}
//}
