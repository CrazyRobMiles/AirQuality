#pragma once
#include "ZPH01.h"
#include "SDS011.h"

void(*active_sensor_loop) ();

void setup_airq_sensor()
{
	switch (settings.airqSensorType)
	{
	case SDS011_SENSOR:
		setup_sds011();
		active_sensor_loop = loop_sds011;
		break;

	case ZPH01_SENSOR:
		setup_zph01();
		// Indicate that there are no 10 micron readings from this sensor 
		pub_ppm_10 = -1;
		active_sensor_loop = loop_zph01;
		break;
	}
}

void loop_airq_sensor()
{
	active_sensor_loop();
}
