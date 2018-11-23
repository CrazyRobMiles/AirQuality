# Heltec Sensor with full User Interface

![Sensor 01 Assembly](Images/Sensor01Construction.png?raw=true "Sensor 01 Printed components")

The code runs on a Heltec LoRa 32 device which has an OLED display. The connections are described in the document "Heltec Connections" in this folder. 

Designs for a 3D printable case for the device are in the case folder. The device uses:

+ Heltec LoRa 32 cpu
+ Bosch BME280 temperature, presssure and humidity sensor
+ SDS011 dust particle sensor
+ DS3231 Real Time Clock 
+ NMEA compatible serially connected GPS system
+ Three push buttons to drive the user interface

The code makes use of a number of Arduino libaries. The source has a customised version of the LMIC library for LoRa. This is required to fit with the pin assignments of the LoRa device on the Heltec processor. There are also customised routines for the clock chip so that they work with the unconventional I2C configuration of the Heltec processor. 

The MQTT interface uses the PubSubClient library. The code can be configured to use sockets so that the unit can be used with Azure IOT hub. Note that this configuration is presently performed at build time, using the SECURE_SOCKETS symbol.

Note that you are advised to modify your PubSubClient library installation to increase the maximum MQTT packet size to 256. This is particularly important if you use the SECURE_SOCKETS option because some of the key values are quite long strings.