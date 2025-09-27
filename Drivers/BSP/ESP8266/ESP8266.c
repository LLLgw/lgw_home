#include "./BSP/ESP8266/ESP8266.h"
#include "./BSP/USART2/usart2.h"
#include "./SYSTEM/delay/delay.h"
#include "./MALLOC/malloc.h"
#include "string.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"

#define MQTT_client 		"AT+MQTTUSERCFG=0,1,\"M\",\"\",\"\",0,0,\"\"\r\n"      //用户、账号、密码

#define ESP8266_WIFI_INFO		"AT+CWJAP=\"Tenda_ZC_5G\",\"zhongchuang\"\r\n" 		 //连接wifi

#define ESP8266_mqtt_INFO		"AT+MQTTCONN=0,\"192.168.110.58\",1883,0\r\n"      //连接mqtt服务器

#define esp8266_uart  USART3

#define buff_size  256   //串口缓冲区大小
#define temp_size  100   //发送字符串大小
#define ptr_size   100

char sub_topic[10]="node1"; //订阅主题

//内存指针
unsigned char  *esp8266_buf=NULL;
unsigned char  ptr[ptr_size];

//申请内存
void apply_buff(void)
{
	esp8266_buf=mymalloc(SRAMIN,buff_size);
}

//清理接收缓冲区
void ESP8266_Clear(void)
{
	memset(esp8266_buf,0,buff_size);
}


//清理接收缓冲区
void ptr_Clear(void)
{
	memset(ptr,0,ptr_size);
}


//向esp8266发送命令
_Bool ESP8266_SendCmd(char *cmd, char *res)
{
	HAL_UART_Transmit(&g_uart3_handle,(uint8_t *)cmd,strlen((const char*)cmd),50);
	
	delay_ms(100);
	
	if(strstr((const char *)esp8266_buf, res) != NULL)		
	{
		ESP8266_Clear();
		return 0;
	}
	
	 //ESP8266_Clear();
	return 1;
}

_Bool ESP8266_SendData(char *cmd, char *res)
{
	HAL_UART_Transmit(&g_uart3_handle,(uint8_t *)cmd,strlen((const char*)cmd),50);
	
	 delay_ms(100);
	 //printf("%s\r\n",esp8266_buf);
	
	if(strstr((const char *)esp8266_buf, res) != NULL)		
	{
		ESP8266_Clear();
		return 0;
	}
	
	 ESP8266_Clear();
	return 1;
}


//初始化esp8266,且连接wifi
void ESP8266_Init(void)
{
	ESP8266_Clear();
	
	apply_buff();   //申请内存
	
	printf("MY1:  AT\r\n");
	while(ESP8266_SendCmd("AT\r\n", "OK"));
	
	printf("MY2:  AT+RST\r\n");
	ESP8266_SendCmd("AT+RST\r\n", "OK");
	delay_ms(100);
	ESP8266_Clear();

	printf("MY3:  AT+CWMODE=1\r\n");
	while(ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK"));
	
	printf("MY4:  AT+CWDHCP=1,1\r\n");
	while(ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK"));
	
	printf("MY5:%s\r\n",ESP8266_WIFI_INFO);
	while(ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"));
	
	ESP8266_Clear();
	printf("ESP8266 Init OK\r\n");
}

void ESP8266_connect_mqtt(void)
{
	ESP8266_Clear();
	printf("MY6:AT+CIPMUX=0\r\n");
	while(ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK"));//单连接模式
	
	printf("MY7:AT+CIPMODE=1\r\n");
	while(ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK"));//透传模式
	
	printf("MY8:%s\r\n",MQTT_client);
	while(ESP8266_SendCmd(MQTT_client,"OK"));//设置用户
	
	printf("MY9:%s\r\n",ESP8266_mqtt_INFO);
	while(ESP8266_SendCmd(ESP8266_mqtt_INFO,"OK"));//连接mqtt服务器
	delay_ms(200);
	
	ESP8266_Clear();
	printf("ESP8266_connect_mqtt OK\r\n");
}

void sub_mqtt_tpoic(void)
{
	
	ESP8266_Clear();
	ptr_Clear();
	
	sprintf((char *)ptr,(char *)"AT+MQTTSUB=0,\"%s\",0\r\n",sub_topic);
	printf("MY10:%s\r\n",ptr);
	while(ESP8266_SendCmd((char *)ptr,"OK"));//订阅节点
	
	printf("订阅topic成功!\r\n");
	ESP8266_Clear();
  ptr_Clear();
}

void pub_mqtt_topic(const unsigned char * send_topic,unsigned char * send_data,uint8_t qos)
{
	uint16_t send_len=0;
	ESP8266_Clear();
	ptr_Clear();
	
	send_len=strlen((char *)send_data);//计算数据大小
	
	sprintf((char *)ptr,"AT+MQTTPUBRAW=0,\"%s\",%d,%d,0\r\n",send_topic,send_len,qos);
	
	while(ESP8266_SendData((char *)ptr,"OK")); //发布topic
	
	printf("Pub:%s\r\n",send_data);
	ESP8266_SendData((char *)send_data,"OK"); //发送数据
	
	printf("Send succeed!\r\n");
	printf("\r\n");
	ESP8266_Clear();
	ptr_Clear();
}
