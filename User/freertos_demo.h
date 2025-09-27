#ifndef __FREERTOS_DEMO_H
#define __FREERTOS_DEMO_H

#include "sys.h"

typedef struct 
{
	uint8_t temp;
	uint8_t humi;
	float A;
	float V;
	float L;
}realdata_typedef;

void freertos_demo(void);   /* 创建lwIP的任务函数 */

#endif
