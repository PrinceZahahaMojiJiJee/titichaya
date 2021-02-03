#include "stm32l1xx.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_utils.h"
#include <RTL.h>

#define LIGHT_SENSITIVE 0x00FF

typedef enum{false, true}bool;

uint16_t ADC_DATA = 0;
bool dark = false;

void SystemClock_Config(void);

void set_up_light_sensor();
void set_up_light();
void light_on();
void light_off();
bool is_dark();

OS_TID id1;
__task void front_light_control();


int main()
{
	//SystemClock_Config();
	ADC_DATA = 1;
	set_up_light_sensor();
	set_up_light();
	light_off();
	os_sys_init(front_light_control);
	while(1)
	{
		//front_light_control();
	}

}

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

void front_light_control()
{
	//add task here
	id1 = os_tsk_self();
	
	while(1)
	{
		dark = is_dark();
		if(dark)
		{
			light_on();
		}
		else
		{
			light_off();
		}
	}
}

void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}