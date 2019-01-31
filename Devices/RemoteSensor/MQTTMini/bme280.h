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

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

enum Bme280_sensor_state { bme280_disconnected, bme280_active };

Bme280_sensor_state bme280_sensor_state;

void printBMEValues() 
{

	switch (bme280_sensor_state)
	{

	case bme280_disconnected:
		Serial.println("BME 280 sensor not connected");

	case bme280_active:
		Serial.print("Temperature = ");
		Serial.print(bme.readTemperature());
		Serial.println(" *C");

		Serial.print("Pressure = ");

		Serial.print(bme.readPressure() / 100.0F);
		Serial.println(" hPa");

		Serial.print("Approx. Altitude = ");
		Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
		Serial.println(" m");

		Serial.print("Humidity = ");
		Serial.print(bme.readHumidity());
		Serial.println(" %");

		Serial.println();
	}
}

void checkBme280()
{
	bool connectOK = false;

	if (bme.begin(0x76))
	{
		bme280_sensor_state = bme280_active;
		Serial.println("BME 280 connected at 76");
		printBMEValues();
		connectOK = true;
	}
	else
	{
		if (bme.begin(0x77))
		{
			bme280_sensor_state = bme280_active;
			Serial.println("BME 280 connected at 77");
			printBMEValues();
			connectOK = true;
		}
	}

	if (!connectOK)
	{
		bme280_sensor_state = bme280_disconnected;
		Serial.println("BME 280 setup failed");
	}
}

void setup_bme280()
{
	checkBme280();

	pub_bme_values_ready = false;
}

void loop_bme280()
{
	switch (bme280_sensor_state)
	{

	case bme280_disconnected:
		break;

	case bme280_active:

		pub_temp = bme.readTemperature();
		pub_pressure = bme.readPressure() / 100.0F;
		pub_humidity = bme.readHumidity();

		pub_bme_values_ready = true;

		break;
	}
}



