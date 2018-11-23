# Air Quality Sensors


There is code for two devies. It is based on a similar structure. 


The Heltec configuration uses a Heltec LoRa 32 processor, an SDS011 sensor, a BME280 environmental sensor a DS3231 real time clock and a NMEA compatible GPS sensor. It has a menu driven interface and can upload readings using MQTT or LoRa. 

The Wemos configuration uses a Wemos D1 Mini, a ZPH01 sensor and a 12 pixel neopixel ring. This has no user interface on the device. It can upload readings using MQTT only.

Both devices can be configured over MQTT/LoRa or via a device connected to their serial port. The commands.h source file contains descriptions of the messages that can be sent to configure the device. Each message is a JSON string that can either set or read a given setting. The Device Configuration folder contains programs that can be used to configure a node over serial or MQTT connections. These are presently work in progress.

Both of the codebases use an "objectless" approach to software structure. Each hardware interface or system element is implemented as a C code file that is included in the application. All of these software components contain setup and loop behaviours that are called in sequence by the main body of the application. The underlying principle of operation is that no loop calls should be "blocking" and stop the operation of the device interface. 

Sensor reading code sets the values of public variables that are then displayed in the user interface and transmitted to clients. These public variables have the prefix "pub_".

Each device maintains setting information which is held in EEPROM. When a device is started for the first time a set of default settings is written to eeprom. The command.h file contains this default setting information. 

Rob Miles
