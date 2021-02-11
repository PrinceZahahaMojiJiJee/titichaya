/*Base register adddress header file*/
#include "stm32l1xx.h"
/*Library related header files*/
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_tim.h"
#include "dwt_delay.h"

void OW_WriteBit(uint8_t d);
uint8_t OW_ReadBit(void);
void DS1820_GPIO_Configure(void);
uint8_t DS1820_ResetPulse(void);

/*haven't been implemented yet!*/
void OW_Master(void);
void OW_Slave(void);
void OW_WriteByte(uint8_t data);
uint16_t OW_ReadByte(void);
void motor_config();
void motor_on();
void motor_off();

uint8_t temp1, temp2;
uint16_t t_temp;
float real_temp;
float get_temp();

float get_temp()
{
	//Send reset pulse
	DS1820_ResetPulse();
	
	//Send 'Skip Rom (0xCC)' command
	OW_WriteByte(0xCC);
	
	//Send 'Temp Convert (0x44)' command
	OW_WriteByte(0x44);
	
	//Delay at least 200ms (typical conversion time)
	LL_mDelay(200);
	
	//Send reset pulse
	DS1820_ResetPulse();
	
	//Send 'Skip Rom (0xCC)' command
	OW_WriteByte(0xCC);
	
	//Send 'Read Scractpad (0xBE)' command
	OW_WriteByte(0xBE);
	
	//Read byte 1 (Temperature data in LSB)
	temp1 = OW_ReadByte();
	
	//Read byte 2 (Temperature data in MSB)
	temp2 = OW_ReadByte();
	t_temp = (temp2<<8)|(temp1);
	
	//Convert to readable floating point temperature
	real_temp = t_temp*1.0/16.0;
	
	DS1820_ResetPulse();
	
	return real_temp;
}

void OW_WriteBit(uint8_t d)
{
	if(d == 1) //Write 1
	{
		OW_Master(); //uC occupies wire bus
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);
		DWT_Delay(1);
		OW_Slave(); //uC releases wire bus
		DWT_Delay(60);
	}
	else //Write 0
	{
		OW_Master(); //uC occupies wire bus
		DWT_Delay(60);
		OW_Slave(); //uC releases wire bus
	}
}

uint8_t OW_ReadBit(void)
{
	OW_Master();
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);
	DWT_Delay(2);
	OW_Slave();
	
	return LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_6);	
}


void DS1820_GPIO_Configure(void)
{
	LL_GPIO_InitTypeDef ds1820_io;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	ds1820_io.Mode = LL_GPIO_MODE_OUTPUT;
	ds1820_io.Pin = LL_GPIO_PIN_6;
	ds1820_io.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	ds1820_io.Pull = LL_GPIO_PULL_NO;
	ds1820_io.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOB, &ds1820_io);
}

uint8_t DS1820_ResetPulse(void)
{	
	OW_Master();
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);
	DWT_Delay(480);
	OW_Slave();
	DWT_Delay(80);
	
	if(LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_6) == 0)
	{
		DWT_Delay(400);
		return 0;
	}
	else
	{
		DWT_Delay(400);
		return 1;
	}
}

void OW_Master(void)
{
	  LL_GPIO_SetPinMode(GPIOB,LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinPull(GPIOB,LL_GPIO_PIN_6,LL_GPIO_PULL_NO);
}

void OW_Slave(void)
{
	 LL_GPIO_SetPinMode(GPIOB,LL_GPIO_PIN_6, LL_GPIO_MODE_INPUT);
   LL_GPIO_SetPinPull(GPIOB,LL_GPIO_PIN_6,LL_GPIO_PULL_UP);
}

void OW_WriteByte(uint8_t data)
{
	uint8_t index;
	for(index = 0; index < 8; index++)
	{
		OW_WriteBit(data & 0x01);
		data = data>>1;
	}
}

uint16_t OW_ReadByte(void)
{
	uint8_t index, bit;
	uint16_t data = 0;
	for(index = 0; index < 8; index++)
	{
		bit = OW_ReadBit();
		if(bit == 1)
			data |= (1<<index);
		DWT_Delay(60);
	}
	return data;
}

void motor_config()
{
	LL_GPIO_InitTypeDef motor;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	motor.Mode = LL_GPIO_MODE_OUTPUT;
	motor.Pin = LL_GPIO_PIN_15;
	motor.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	motor.Pull = LL_GPIO_PULL_NO;
	motor.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOB, &motor);
}

void motor_on()
{
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_15);
}

void motor_off()
{
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_15);
}