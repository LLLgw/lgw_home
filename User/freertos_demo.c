#include "freertos_demo.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LNA226/bsp_ina226.h"
#include "./BSP/BH1750/BH1750.h"
#include "./BSP/DHT11/dht11.h"
#include "./BSP/ESP8266/esp8266.h"
#include "./MALLOC/malloc.h"
#include "CJSON.h"
#include "lwip_comm.h"
#include "lwip_demo.h"
#include "lwipopts.h"
#include "stdio.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


//开始任务
#define START_TASK_PRIO         5          
#define START_STK_SIZE          128        
TaskHandle_t StartTask_Handler;                 

//lwip任务
#define Socket_TCP_TASK_PRIO      10       
#define Socket_TCP_STK_SIZE       512    
TaskHandle_t Socket_TCP_Task_Handler;          

//采集数据任务
#define DATA_COLLETION_PRIO       3
#define DATA_COLLETION_STACK_SIZE 1024
TaskHandle_t  data_collection_task_handle;


//获取信息json格式解析任务
#define JSON_SYTLE_PRIO        8
#define JSON_SYTLE_STACK_SIZE 256
TaskHandle_t json_style_task_handle;

//消息执行任务
#define TAKE_MESS_PRIO        9
#define TAKE_MESS_STACK_SIZE 256
TaskHandle_t  taske_msseage_task_handle;

//二值信号量与队列
QueueHandle_t Semaphore1_handle;
QueueHandle_t led_queue_handle;

#define json_style 	"{\"temp\":%d,\"humi\":%d,\"a\":%f,\"v\":%f,\"l\":%f}"
const unsigned char pub_topics[]={"zone1"}; //发布的topic

#define temp_size   128

realdata_typedef data_collection;
extern unsigned char * esp8266_buf;
unsigned char  temp_humi[64];

//任务函数
void start_task(void *pvParameters);  
void socket_tcp_task(void *pvParameters); 
void data_collection_task(void *pvParameters);
void data_send_task(void *pvParameters);
void json_style_task(void *pvParameters);
void take_msseage_task(void *pvParameters);


//创建开始任务
void freertos_demo(void)
{

	  Semaphore1_handle=xSemaphoreCreateBinary();
	  led_queue_handle=xQueueCreate(1,sizeof(uint8_t));
	
    xTaskCreate((TaskFunction_t )start_task,
                (const char *   )"start_task",
                (uint16_t       )START_STK_SIZE,
                (void *         )NULL,
                (UBaseType_t    )START_TASK_PRIO,
                (TaskHandle_t * )&StartTask_Handler);

    vTaskStartScheduler(); 
}


//开始任务
void start_task(void *pvParameters)
{
    pvParameters = pvParameters;
    
    //初始化lwip
    while (lwip_comm_init() != 0)
    {
			printf("lwip初始化失败\r\n");
    }
		
		printf("lwip初始完成\r\n");
		
    while ((g_lwipdev.dhcpstatus != 2)&&(g_lwipdev.dhcpstatus != 0XFF))  // 等待DHCP获取成功/超时溢出 
    {
      vTaskDelay(5);
    }
		
		
    taskENTER_CRITICAL(); 
		//socket——tcp视频流任务
    xTaskCreate((TaskFunction_t )socket_tcp_task,
                (const char*    )"lwip_demo_task",
                (uint16_t       )Socket_TCP_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Socket_TCP_TASK_PRIO,
                (TaskHandle_t*  )&Socket_TCP_Task_Handler);

    //数据采集任务
    xTaskCreate((TaskFunction_t )data_collection_task,
                (const char*    )"data_collection_task",
                (uint16_t       )DATA_COLLETION_STACK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )DATA_COLLETION_PRIO,
                (TaskHandle_t*  )&data_collection_task_handle);
		
		
    //json格式解析任务								
    xTaskCreate((TaskFunction_t )json_style_task,
                (const char*    )"json_style_task",
                (uint16_t       )JSON_SYTLE_STACK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )JSON_SYTLE_PRIO,
                (TaskHandle_t*  )&json_style_task_handle);

		//消息处理任务
    xTaskCreate((TaskFunction_t )take_msseage_task,
                (const char*    )"take_msseage_task",
                (uint16_t       )TAKE_MESS_STACK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )TAKE_MESS_PRIO,
                (TaskHandle_t*  )&taske_msseage_task_handle);								
    			
		vTaskDelete(StartTask_Handler); 
    taskEXIT_CRITICAL();            
}


//lwip任务
void socket_tcp_task(void *pvParameters)
{
    pvParameters = pvParameters;
    lwip_demo();            
    
	  //初始完后删掉自己
	  vTaskDelete(NULL);
	
    while (1)
    {
        vTaskDelay(5);
    }
}


//数据采集
void data_collection_task(void *pvParameters)
{
	while(1)
	{
		{
			dht11_read_data(&data_collection.temp,&data_collection.humi);//获取温湿度
			check_AV(&data_collection.V,&data_collection.A);//获取电压电流
			BH1750_receied_data(&data_collection.L);        //获取光照强度
			sprintf((char *)temp_humi,json_style,
											data_collection.temp,
											data_collection.humi,
											data_collection.A,
											data_collection.V,
											data_collection.L);
			
			vTaskDelay(3000);
			printf("数据采集\r\n");
		}
		//taskENTER_CRITICAL();
		//pub_mqtt_topic(pub_topics,(unsigned char *)temp_humi,2);
		//taskEXIT_CRITICAL();
	}
}



//JSON数据解析
void json_style_task(void *pvParameters)
{
	unsigned char * p=esp8266_buf;
	const char *req_payload=NULL;
	cJSON *json,*json_led_switch=NULL,*json_message=NULL;
	while(1)
	{
		vTaskSuspend(json_style_task_handle);
		printf("进Json数据解析任务\r\n");
		{
			req_payload=strchr((const char *)p,'{');
			json=cJSON_Parse(req_payload);
			printf("%s\r\n",req_payload);
			if(!json) printf("Json 解析错误!\r\n");
			else
			{
				json_led_switch=cJSON_GetObjectItem(json,"switch");
				json_message=cJSON_GetObjectItem(json,"message");
				
				printf("解析switch:%d\r\n",json_led_switch->valueint);
				printf("解析message:%s\r\n",json_message->valuestring);
				
				if(json_led_switch->valueint==1||json_led_switch->valueint==0)
				{
					xQueueSend(led_queue_handle,&json_led_switch->valueint,portMAX_DELAY);
				}
			}
			cJSON_Delete(json);
			ESP8266_Clear();
		}
	}
}


//消息执行
void take_msseage_task(void *pvParameters)
{
	uint8_t led_switch=2;
	while(1)
	{
		if(xQueueReceive(led_queue_handle,&led_switch,portMAX_DELAY)==pdTRUE)
		{
			if(led_switch==0)
			{
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);
				printf("关灯\r\n");
			}
			else if(led_switch==1)
			{
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
				printf("开灯\r\n");
			}
		}
	}
}
