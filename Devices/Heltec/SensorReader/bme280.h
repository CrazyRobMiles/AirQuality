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
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

enum Bme280_sensor_state { bme280_starting, bme280_active};

Bme280_sensor_state bme280_sensor_state;

unsigned long delayTime;

void setup_bme280() 
{
	bme280_sensor_state = bme280_starting;

	pub_bme_values_ready = false;

#ifdef VERBOSE
	Serial.println("Done setting up BME 280");
#endif
}


void printBMEValues() {
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

void loop_bme280() 
{
	switch(bme280_sensor_state)
	{
		case bme280_starting:

		if(bme.begin(0x76, &heltecWire))
		{
			bme280_sensor_state = bme280_active;
		}

		break;

		case bme280_active:

		if (pub_bme_values_ready)
			return;

		if (pub_millis_to_next_update > 0)
			return;

		pub_temp = bme.readTemperature();
		pub_pressure = bme.readPressure() / 100.0F;
		pub_humidity = bme.readHumidity();
		pub_bme_values_ready = true;

		break;
	}
}

