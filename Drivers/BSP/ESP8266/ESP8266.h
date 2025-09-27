#ifndef __ESP8266_H
#define __ESP8266_H

#include "sys.h"
void ESP8266_Init(void);

void ESP8266_Clear(void);

void Instruction_Clear(void);

_Bool ESP8266_SendCmd(char *cmd, char *res);

void ESP8266_connect_mqtt(void);

void sub_mqtt_tpoic(void);

void pub_mqtt_topic(const unsigned char * send_topic,unsigned char * send_buff,uint8_t qos);

#endif


















