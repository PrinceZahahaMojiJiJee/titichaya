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

int done = 0;

void TIMBase_Config(void)
{
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_TIM_EnableCounter(TIM2);
}

void TIM_GPIO_Config(void)
{
		LL_GPIO_InitTypeDef hcsr04_gpio;
		
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		
		hcsr04_gpio.Mode = LL_GPIO_MODE_OUTPUT;
		hcsr04_gpio.Pull = LL_GPIO_PULL_NO;
		hcsr04_gpio.Pin = LL_GPIO_PIN_2;
		hcsr04_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		hcsr04_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		LL_GPIO_Init(GPIOA, &hcsr04_gpio);
	
		hcsr04_gpio.Mode = LL_GPIO_MODE_INPUT;
		hcsr04_gpio.Pin = LL_GPIO_PIN_1;
		hcsr04_gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		hcsr04_gpio.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		LL_GPIO_Init(GPIOA, &hcsr04_gpio);
	
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
}

uint16_t rise_timestamp = 0;
uint16_t fall_timestamp = 0;
uint16_t up_cycle = 0;

uint8_t state = 0;
float period = 0;
float distance = 0;

uint32_t TIM2CLK;
uint32_t PSC;

float getdistance()
{

		//distance = -1;
	done = 0;
	while(!done)
	{
			switch(state)
			{
				case 0:
					//Trigger measurement
					LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_2);
					LL_mDelay(1);
					LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);
					state = 1;
				break;
				
				case 1:
					if(LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1))
					{
						rise_timestamp = LL_TIM_GetCounter(TIM2);
						state = 2;
					}
				break;
					
				case 2:
					if(LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1) == RESET)
					{
						fall_timestamp = LL_TIM_GetCounter(TIM2);
						//Calculate uptime
						if(fall_timestamp > rise_timestamp)
						{
							up_cycle = fall_timestamp - rise_timestamp;
						}
						else if(fall_timestamp < rise_timestamp)
						{
							up_cycle = (LL_TIM_GetAutoReload(TIM2) - rise_timestamp) + fall_timestamp + 1; 
						}
						else
						{
							//cannot measure at this freq
							up_cycle = 0;
						}
						
						if(up_cycle != 0)
						{
							PSC = LL_TIM_GetPrescaler(TIM2) + 1;
							TIM2CLK = SystemCoreClock / PSC;
							
							period = (up_cycle*(PSC) * 1.0) / (TIM2CLK * 1.0); //calculate uptime period
							distance = (period * 340) / 2; //meter unit
						}
						state = 0;
						done = 1;
					}
				break;
					
			}
			__NOP();
		}
			
		return distance;
}

void set_up_house_light()
{
	RCC->AHBENR |= (1<<1);
	GPIOB->MODER |= (1<<4);
	GPIOB->ODR &= ~(1<<2);
}

void house_light_on()
{
	GPIOB->ODR |= (1<<2);
}

void house_light_off()
{
	GPIOB->ODR &= ~(1<<2);
}