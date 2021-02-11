#include "common.h"
#include "light_sensor.h"
#include "temp_sensor.h"
#include "ultrasonic.h"
#include <RTL.h>

OS_TID front_light, fan;
__task void front_light_control();
__task void fan_control();

bool dark = false;
float temp;
bool is_on = false;
bool passed = false;
void house_light_control();

int main()
{
	DWT_Init();
	DS1820_GPIO_Configure();
	motor_config();
	
	set_up_light_sensor();
	set_up_light();
	light_off();
	
	SystemClock_Config();
	TIM_GPIO_Config();
	TIMBase_Config();
	set_up_house_light();
	house_light_off();
	
	while(1)
	{
		front_light_control();
		fan_control();
		house_light_control();
	}
}

void front_light_control()
{
	//add task here
	dark = is_dark();
	if(get_light() < 0x00F5)
	{
		light_on();
	}
	else
	{
		light_off();
	}
}

void fan_control()
{
	temp = get_temp();
	
	if(temp > 33.0)
	{
		motor_on();
	}
	else
	{
		motor_off();
	}
}
void house_light_control()
{
	if(getdistance() < 0.15)
	{
		if(passed)
		{
			if(!is_on)
			{
				house_light_on();
				is_on = true;
			}
			else
			{
				house_light_off();
				is_on = false;
			}
			passed = false;
		}
	}
	else
	{
		passed = true;
	}
}