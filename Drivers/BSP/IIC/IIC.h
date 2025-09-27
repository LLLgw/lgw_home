 #ifndef __IIC_H
#define __IIC_H

#include "sys.h"


/******************************************************************************************/
/* 引脚 定义 */

#define IIC_SCL_GPIO_PORT               GPIOC
#define IIC_SCL_GPIO_PIN                GPIO_PIN_12
#define IIC_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define IIC_SDA_GPIO_PORT               GPIOD
#define IIC_SDA_GPIO_PIN                GPIO_PIN_2
#define IIC_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

/******************************************************************************************/

/* IO操作 */
#define IIC_SCL(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SCL */

#define IIC_SDA(x)        do{ x ? \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                              HAL_GPIO_WritePin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                          }while(0)       /* SDA */

#define IIC_READ_SDA     HAL_GPIO_ReadPin(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN) /* 读取SDA */

void iic_init(void);            /* 初始化IIC的IO口 */
void IIC_Start(void);           /* 发送IIC开始信号 */
void IIC_Stop(void);            /* 发送IIC停止信号 */
void IIC_Ack(void);             /* IIC发送ACK信号 */
void IIC_Nack(void);            /* IIC不发送ACK信号 */
uint8_t IIC_Wait_Ack(void);     /* IIC等待ACK信号 */
void IIC_Send_Byte(uint8_t txd);/* IIC发送一个字节 */
uint8_t IIC_Read_Byte(unsigned char ack);/* IIC读取一个字节 */

#endif

