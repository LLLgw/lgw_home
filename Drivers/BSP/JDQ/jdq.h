#ifndef __JDQ_H
#define __JDQ_H

#include "./SYSTEM/sys/sys.h"


#define JDQ_GPIO_PORT                  GPIOC
#define JDQ_GPIO_PIN                   GPIO_PIN_2
#define JDQ_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)             /* PF口时钟使能 */


#define JDQ(x)   do{ x ? \
                      HAL_GPIO_WritePin(JDQ_GPIO_PORT, JDQ_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(JDQ_GPIO_PORT, JDQ_GPIO_PIN, GPIO_PIN_RESET); \
                  }while(0)     

void jdq_init(void);                                                                            /* 初始化 */

#endif
