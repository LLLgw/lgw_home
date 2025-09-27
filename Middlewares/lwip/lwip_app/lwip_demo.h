#ifndef _LWIP_DEMO_H
#define _LWIP_DEMO_H
#include "./SYSTEM/sys/sys.h"

typedef struct								//视频流状态结构体				
{
	uint32_t back_start;
	uint32_t back_end;
	uint32_t back_len;
  uint32_t buf_len;
	uint32_t send_size;
	uint8_t  picture_ok;
	uint8_t  back_flag;
}typedef_buf_state;  


void lwip_demo(void);

#endif /* _CLIENT_H */
