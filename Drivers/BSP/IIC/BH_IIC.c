#include "./BSP/IIC/BH_iic.h"
#include "./SYSTEM/delay/delay.h"

void BH_IIC_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
		__HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();	
	
    gpio_init_struct.Pin = GPIO_PIN_7;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;        /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; /* 快速 */
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);/* SCL */

    gpio_init_struct.Pin = GPIO_PIN_6;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_OD;        /* 开漏输出 */
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);/* SDA */

    BH_IIC_Stop();     
}

/**
 * @brief       IIC延时函数,用于控制IIC读写速度
 * @param       无
 * @retval      无
 */
static void iic_delay(void)
{
    delay_us(2);    /* 2us的延时, 读写速度在250Khz以内 */
}

/**
 * @brief       产生IIC起始信号
 * @param       无
 * @retval      无
 */
void BH_IIC_Start(void)
{
    BH_IIC_SDA(1);
    BH_IIC_SCL(1);
    iic_delay();
    BH_IIC_SDA(0);     /* START信号: 当SCL为高时, SDA从高变成低, 表示起始信号 */
    iic_delay();
    BH_IIC_SCL(0);     /* 钳住I2C总线，准备发送或接收数据 */
    //iic_delay();
}

/**
 * @brief       产生IIC停止信号
 * @param       无
 * @retval      无
 */
void BH_IIC_Stop(void)
{
    BH_IIC_SDA(0);     /* STOP信号: 当SCL为高时, SDA从低变成高, 表示停止信号 */
    //iic_delay();
    BH_IIC_SCL(1);
    iic_delay();
    BH_IIC_SDA(1);     /* 发送I2C总线结束信号 */
    iic_delay();
}

/**
 * @brief       等待应答信号到来
 * @param       无
 * @retval      1，接收应答失败
 *              0，接收应答成功
 */
uint8_t BH_IIC_Wait_Ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    BH_IIC_SDA(1);     /* 主机释放SDA线(此时外部器件可以拉低SDA线) */
    iic_delay();
    BH_IIC_SCL(1);     /* SCL=1, 此时从机可以返回ACK */
    iic_delay();

    while (BH_IIC_READ_SDA)    /* 等待应答 */
    {
        waittime++;

        if (waittime > 250)
        {
            BH_IIC_Stop();
            rack = 1;
            break;
        }
    }

    BH_IIC_SCL(0);     /* SCL=0, 结束ACK检查 */
    iic_delay();
    return rack;
}

/**
 * @brief       产生ACK应答
 * @param       无
 * @retval      无
 */
void BH_IIC_Ack(void)
{
    BH_IIC_SDA(0);     /* SCL 0 -> 1 时 SDA = 0,表示应答 */
    iic_delay();
    BH_IIC_SCL(1);     /* 产生一个时钟 */
    iic_delay();
    BH_IIC_SCL(0);
    iic_delay();
    BH_IIC_SDA(1);     /* 主机释放SDA线 */
    iic_delay();
}

/**
 * @brief       不产生ACK应答
 * @param       无
 * @retval      无
 */
void BH_IIC_Nack(void)
{
    BH_IIC_SDA(1);     /* SCL 0 -> 1  时 SDA = 1,表示不应答 */
    iic_delay();
    BH_IIC_SCL(1);     /* 产生一个时钟 */
    iic_delay();
    BH_IIC_SCL(0);
    iic_delay();
}

/**
 * @brief       IIC发送一个字节
 * @param       data: 要发送的数据
 * @retval      无
 */
void BH_IIC_Send_Byte(uint8_t data)
{
    uint8_t t;
    
    for (t = 0; t < 8; t++)
    {
        BH_IIC_SDA((data & 0x80) >> 7);    /* 高位先发送 */
        iic_delay();
        BH_IIC_SCL(1);
        iic_delay();
        BH_IIC_SCL(0);
        data <<= 1;     /* 左移1位,用于下一次发送 */
    }
    BH_IIC_SDA(1);         /* 发送完成, 主机释放SDA线 */
}

/**
 * @brief       IIC读取一个字节
 * @param       ack:  ack=1时，发送ack; ack=0时，发送nack
 * @retval      接收到的数据
 */
uint8_t BH_IIC_Read_Byte(uint8_t ack)
{
    uint8_t i, receive = 0;

    for (i = 0; i < 8; i++ )    /* 接收1个字节数据 */
    {
        receive <<= 1;  /* 高位先输出,所以先收到的数据位要左移 */
        BH_IIC_SCL(1);
        iic_delay();

        if (BH_IIC_READ_SDA)
        {
            receive++;
        }
        
        BH_IIC_SCL(0);
        iic_delay();
    }

    if (!ack)
    {
        BH_IIC_Nack();     /* 发送nACK */
    }
    else
    {
        BH_IIC_Ack();      /* 发送ACK */
    }

    return receive;
}
