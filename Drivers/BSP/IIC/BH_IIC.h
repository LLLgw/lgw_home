 #ifndef __BH_IIC_H
#define __BH_IIC_H

#include "sys.h"


#define BH_IIC_SCL(x)        do{ x ? \
                              HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET); \
                          }while(0)       /* SCL */

#define BH_IIC_SDA(x)        do{ x ? \
                              HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET); \
                          }while(0)       /* SDA */

#define BH_IIC_READ_SDA    HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) /* 读取SDA */

void BH_IIC_init(void);            /* 初始化IIC的IO口 */
void BH_IIC_Start(void);           /* 发送IIC开始信号 */
void BH_IIC_Stop(void);            /* 发送IIC停止信号 */
void BH_IIC_Ack(void);             /* IIC发送ACK信号 */
void BH_IIC_Nack(void);            /* IIC不发送ACK信号 */
uint8_t BH_IIC_Wait_Ack(void);     /* IIC等待ACK信号 */
void BH_IIC_Send_Byte(uint8_t txd);/* IIC发送一个字节 */
uint8_t BH_IIC_Read_Byte(unsigned char ack);/* IIC读取一个字节 */

#endif

