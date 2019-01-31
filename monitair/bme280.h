#pragma once
/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

#define BME280_NOT_CONNECTED -1

struct bme280Reading {
	int activeBMEAddress;
	float temperature;
	float pressure;
	float humidity;
};

int bmeAddresses[] = { 0x76, 0x77 };

int startBme280(struct sensor * bme280Sensor)
{
	struct bme280Reading * bme280activeReading;

	if (bme280Sensor->activeReading == NULL)
	{
		bme280activeReading = new bme280Reading();
		bme280Sensor->activeReading = bme280activeReading;
	}
	else
	{
		bme280activeReading =
			(struct bme280Reading *) bme280Sensor->activeReading;
	}

	for (int i = 0; i < sizeof(bmeAddresses) / sizeof(int); i++)
	{
		if (bme.begin(bmeAddresses[i]))
		{
			bme280activeReading->activeBMEAddress = bmeAddresses[i];
			bme280Sensor->status = SENSOR_OK;
			return SENSOR_OK;
		}
	}

	bme280Sensor->status = BME280_NOT_CONNECTED;

	return BME280_NOT_CONNECTED;
}

int updateBME280Reading(struct sensor * bme280Sensor)
{
	if (bme280Sensor->status == SENSOR_OK)
	{
		struct bme280Reading * bme280activeReading =
			(struct bme280Reading *) bme280Sensor->activeReading;

		bme280activeReading->temperature = bme.readTemperature();
		bme280activeReading->humidity = bme.readHumidity();
		bme280activeReading->pressure = bme.readPressure() / 100.0F;
		bme280Sensor->millisAtLastReading = millis();
	}

	return bme280Sensor->status;
}

int addBME280Reading(struct sensor * bme280Sensor, char * jsonBuffer, int jsonBufferSize)
{
	if (bme280Sensor->status == SENSOR_OK)
	{
		struct bme280Reading * bme280activeReading =
			(struct bme280Reading *) bme280Sensor->activeReading;

		snprintf(jsonBuffer, jsonBufferSize, "%s,\"temp\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f",
			jsonBuffer,
			bme280activeReading->temperature, 
			bme280activeReading->humidity, 
			bme280activeReading->pressure);
	}

	return bme280Sensor->status;
}

void bme280StatusMessage(struct sensor * bme280sensor, char * buffer, int bufferLength)
{
	struct bme280Reading * bme280activeReading =
		(struct bme280Reading *) bme280sensor->activeReading;

	switch (bme280sensor->status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "BME 280 sensor connected at %x", bme280activeReading->activeBMEAddress);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "BME 280 sensor off");
		break;

	case BME280_NOT_CONNECTED:
		snprintf(buffer, bufferLength, "BME 280 sensor not connected");
		break;
	default:
		snprintf(buffer, bufferLength, "BME 280 status invalid");
		break;
	}
}

