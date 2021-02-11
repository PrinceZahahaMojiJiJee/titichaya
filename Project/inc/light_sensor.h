#include "stm32l1xx.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"

#define LIGHT_SENSITIVE 0x00FF//high value sensitive

typedef enum{false, true}bool;

uint16_t ADC_DATA = 0;

void set_up_light_sensor();
void set_up_light();
void light_on();
void light_off();
bool is_dark();

void set_up_light_sensor()
{
	RCC->AHBENR |= (1<<0);
	GPIOA->MODER |= (3<<8);
	RCC->CR |= (1<<0);
	while(((RCC->CR & 0x02) >> 1) == 1);
	RCC->APB2ENR |= (1<<9);
	
	ADC1->CR1 |= (1<<24)|(1<<11);
	ADC1->CR1 &= ~(7<<13);
	ADC1->CR2 &= ~(1<<11);
	ADC1->SMPR3 |= (2<<12);
	ADC1->SQR5 |= (4<<0);
	ADC1->CR2 |= (1<<0);
}

bool is_dark()
{
	ADC1->CR2 |= (1<<30);
	while((ADC1->SR & (1<<1)) == 0);
	ADC_DATA = ADC1->DR;
	if(ADC_DATA < LIGHT_SENSITIVE)
	{
		return true;
	}
	return false;
}

void set_up_light()
{
	RCC->AHBENR |= (1<<0);
	GPIOA->MODER |= (1<<0);
	GPIOA->ODR &= ~(1<<0);
}

void light_on()
{
	GPIOA->ODR |= (1<<0);
}

void light_off()
{
	GPIOA->ODR &= ~(1<<0);
}