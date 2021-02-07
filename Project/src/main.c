#include "common.h"
#include "light_sensor.h"
#include "temp_sensor.h"
#include <RTL.h>

OS_TID front_light, fan;
__task void front_light_control();
__task void fan_control();

bool dark = false;
float temp;

int main()
{
	DWT_Init();
	DS1820_GPIO_Configure();
	motor_config();
	set_up_light_sensor();
	set_up_light();
	light_off();
	SystemClock_Config();
	while(1)
	{
		front_light_control();
		fan_control();
	}
}

void front_light_control()
{
	//add task here
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

void fan_control()
{
	temp = get_temp();
	
	if(temp > 35.0)
	{
		motor_on();
	}
	else
	{
		motor_off();
	}
}