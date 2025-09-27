#include "./BSP/BH1750/BH1750.h"
#include "./BSP/IIC/BH_iic.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "string.h"

#define Slaveaddress 0x46
uint8_t BUff[2]={0};
uint8_t Bh1750_single_write(uint8_t reg_address);

void BH1750_Init(void)
{
	BH_IIC_init();
}


uint8_t Bh1750_single_write(uint8_t reg_address)
{
	uint8_t ack=0;
	
	BH_IIC_Start();
	//BH_IIC_Wait_Ack();
	
	BH_IIC_Send_Byte(Slaveaddress);
	BH_IIC_Wait_Ack();
	
	BH_IIC_Send_Byte(reg_address);
	ack=BH_IIC_Wait_Ack();
	
	BH_IIC_Stop();
	
	return ack;
}


void BH1750_read(void)
{
	uint8_t temp=1;
		
	BH_IIC_Start();
	BH_IIC_Send_Byte(Slaveaddress+1);
	BH_IIC_Wait_Ack();
	
	for(uint8_t i=0;i<2;i++)
	{
		if(i==1) temp=0;
		BUff[i]=BH_IIC_Read_Byte(temp);
	}
	
	BH_IIC_Stop();
	delay_ms(5);
}

void BH1750_receied_data(float * temp)
{
	uint16_t dis_data=0;
	Bh1750_single_write(0x01);
	Bh1750_single_write(0x10);
	
	delay_ms(180);
	BH1750_read();
	
	dis_data=BUff[0];
	dis_data=(dis_data<<8)+BUff[1];
	*temp=(dis_data*1.0)/1.2;
	
	memset(BUff,0,2);
}
