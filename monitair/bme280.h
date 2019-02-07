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
#include "sensors.h"

#define BME280_NOT_CONNECTED -1

struct bme280Reading {
	int activeBMEAddress;
	float temperature;
	float pressure;
	float humidity;
};

int startBme280(struct sensor * bme280Sensor);
int updateBME280Reading(struct sensor * bme280Sensor);
int addBME280Reading(struct sensor * bme280Sensor, char * jsonBuffer, int jsonBufferSize);
void bme280StatusMessage(struct sensor * bme280sensor, char * buffer, int bufferLength);

