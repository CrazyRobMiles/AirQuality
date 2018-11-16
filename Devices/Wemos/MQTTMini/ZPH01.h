#include <Adafruit_NeoPixel.h>

#pragma once

#define MESSAGE_START 0xFF

#define READING_COMMAND 0xC0
#define READING_LENGTH 9

SoftwareSerial SensorSerial(12, 13, false, 128);

enum Zpho1_sensor_state {zph01_waiting_for_start, zph01_loading_block};

Zpho1_sensor_state zph01_sensor_state;

int readingByteCounter;
byte readChecksum;
byte sensorBuffer[READING_LENGTH-2];

void zph01_pump_byte(byte b)
{
	TRACE((int)b);
	TRACE(" ");

	switch (zph01_sensor_state) 
	{
	case zph01_waiting_for_start:
		if (b == MESSAGE_START)
		{
			readingByteCounter = 0;
			readChecksum = 0;
			zph01_sensor_state = zph01_loading_block;
		}
		break;

	case zph01_loading_block:
		if (readingByteCounter == 7)
		{
			// got the checksum - work out the data one
			byte calculatedChecksum = 0;
			for (int j = 0; j < 7; j++)
			{
				calculatedChecksum += sensorBuffer[j];
			}
			calculatedChecksum = (~calculatedChecksum) + 1;
			if (calculatedChecksum == b)
			{
				// Got a valid value
				pub_ppm_25 = sensorBuffer[2] + sensorBuffer[2] / 10.0;
				pub_air_values_ready = true;
			}
			else
			{
				TRACELN(calculatedChecksum);
				TRACELN(b);
			}
			zph01_sensor_state = zph01_waiting_for_start;
		}
		else
		{
			sensorBuffer[readingByteCounter] = b;
			readingByteCounter++;
		}
		break;
	
	}
}

void setup_zph01()
{
	SensorSerial.begin(9600);
	zph01_sensor_state = zph01_waiting_for_start;
}


void loop_zph01()
{
	while (SensorSerial.available()) {
		byte ch = SensorSerial.read();
		zph01_pump_byte(ch);
	}
}
