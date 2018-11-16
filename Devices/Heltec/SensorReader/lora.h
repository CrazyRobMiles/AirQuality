#pragma once

/*******************************************************************************
* Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
*
* Permission is hereby granted, free of charge, to anyone
* obtaining a copy of this document and accompanying files,
* to do whatever they want with them without any restriction,
* including, but not limited to, copying, modification and redistribution.
* NO WARRANTY OF ANY KIND IS PROVIDED.
*
* This uses ABP (Activation-by-personalisation), where a DevAddr and
* Session keys are preconfigured (unlike OTAA, where a DevEUI and
* application key is configured, while the DevAddr and session keys are
* assigned/generated in the over-the-air-activation procedure).
*
*
* Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
* g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
* violated by this sketch when left running for longer)!

* To use this sketch, first register your application and device with
* the things network, to set or generate an AppEUI, DevEUI and AppKey.
* Multiple devices can use the same AppEUI, but each device has its own
* DevEUI and AppKey.
*
* Do not forget to define the radio type correctly in config.h.
*
*******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <Arduino.h>

bool lora_otaa_joined = false;
bool sleeping = false;

#define DEBUG //change to DEBUG to see Serial messages

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_HEX_PRINT(x,y) Serial.print(x,y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_HEX_PRINT(x,y)
#endif

void os_getArtEui(u1_t* buf) {
	memcpy(buf, settings.lora_otaa_APPEUI, 8);
}

// provide DEVEUI (8 bytes, LSBF)
void os_getDevEui(u1_t* buf) {
	memcpy(buf, settings.lora_otaa_DEVEUI, 8);
}

// provide APPKEY key (16 bytes)
void os_getDevKey(u1_t* buf) {
	memcpy(buf, settings.lora_otaa_APPKEY, 16);
}

static osjob_t sendjob;
static osjob_t initjob;

// Pin mapping is hardware specific.
// Pin mapping is hardware specific.
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
//

// From https://github.com/jpraychev/Heltec_ESP32/blob/master/test_heltec.ino

const lmic_pinmap lmic_pins = {
	.nss = 18,
	.rxtx = LMIC_UNUSED_PIN,
	.rst = 14,
	.dio = { 26, 33, 32 },
};

bool OK_to_send = true;

// initial job
static void initfunc(osjob_t* j) {
	if (settings.lora_abp_active)
	{

	}
	else
	{
		// reset MAC state
		LMIC_reset();
		// start joining
		LMIC_startJoining();
		// init done - onEvent() callback will be invoked...
	}
}

void onEvent(ev_t ev)
{
	int i, j;
	switch (ev) {
	case EV_SCAN_TIMEOUT:
		DEBUG_PRINTLN("EV_SCAN_TIMEOUT");
		break;
	case EV_BEACON_FOUND:
		DEBUG_PRINTLN("EV_BEACON_FOUND");
		break;
	case EV_BEACON_MISSED:
		DEBUG_PRINTLN("EV_BEACON_MISSED");
		break;
	case EV_BEACON_TRACKED:
		DEBUG_PRINTLN("EV_BEACON_TRACKED");
		break;
	case EV_JOINING:
		DEBUG_PRINTLN("EV_JOINING");
		break;
	case EV_JOINED:
		DEBUG_PRINTLN("EV_JOINED");
		// Disable link check validation (automatically enabled
		// during join, but not supported by TTN at this time).
		LMIC_setLinkCheckMode(0);
		// after Joining a job with the values will be sent.
		lora_otaa_joined = true;
		break;
	case EV_RFU1:
		DEBUG_PRINTLN("EV_RFU1");
		break;
	case EV_JOIN_FAILED:
		DEBUG_PRINTLN("EV_JOIN_FAILED");
		break;
	case EV_REJOIN_FAILED:
		DEBUG_PRINTLN("EV_REJOIN_FAILED");
		// Re-init
		os_setCallback(&initjob, initfunc);
		break;
	case EV_TXCOMPLETE:
		if (LMIC.dataLen) {
        // data received in rx slot after tx
        // if any data received, a LED will blink
        // this number of times, with a maximum of 10
        Serial.print(F("Data Received: "));
			for(int i=0; i<LMIC.dataLen; i++)
			{
				Serial.println(LMIC.frame[LMIC.dataBeg+i],HEX);
			}
		}
		DEBUG_PRINTLN("EV_TXCOMPLETE (includes waiting for RX windows)");
		OK_to_send = true;
		break;
	case EV_LOST_TSYNC:
		DEBUG_PRINTLN("EV_LOST_TSYNC");
		break;
	case EV_RESET:
		DEBUG_PRINTLN("EV_RESET");
		break;
	case EV_RXCOMPLETE:
		// data received in ping slot
		DEBUG_PRINTLN("EV_RXCOMPLETE");
		break;
	case EV_LINK_DEAD:
		DEBUG_PRINTLN("EV_LINK_DEAD");
		break;
	case EV_LINK_ALIVE:
		DEBUG_PRINTLN("EV_LINK_ALIVE");
		break;
	default:
		DEBUG_PRINTLN("Unknown event");
		break;
	}
}

void do_send(osjob_t* j) {
	byte buffer[5];
	float temperature, pascal, humidity;
	uint16_t t_value, p_value, s_value;
	uint8_t h_value;
	temperature = pub_temp;
	humidity = pub_humidity;
	pascal = pub_pressure;
	pascal = pascal / 100;
	DEBUG_PRINT("Pressure: ");
	DEBUG_PRINT(pascal);
	DEBUG_PRINT("Pa\t");
	DEBUG_PRINT("Temperature: ");
	DEBUG_PRINT(temperature);
	DEBUG_PRINT("C\t");
	DEBUG_PRINT("\tHumidity: ");
	DEBUG_PRINT(humidity);
	DEBUG_PRINTLN("%");
	temperature = constrain(temperature, -24, 40);  //temp in range -24 to 40 (64 steps)
	pascal = constrain(pascal, 970, 1034);    //pressure in range 970 to 1034 (64 steps)*/
	t_value = uint16_t((temperature*(100 / 6.25) + 2400 / 6.25)); //0.0625 degree steps with offset
																  // no negative values
	DEBUG_PRINT("Coded TEMP: ");
	DEBUG_HEX_PRINT(t_value, HEX);
	p_value = uint16_t((pascal - 970) / 1); //1 mbar steps, offset 970.
	DEBUG_PRINT("\tCoded Pascal: ");
	DEBUG_HEX_PRINT(p_value, HEX);
	DEBUG_PRINT("\tCoded Humidity: ");
	DEBUG_HEX_PRINT(h_value, HEX);
	DEBUG_PRINTLN("");
	s_value = (p_value << 10) + t_value;  // putting the bits in the right place
	h_value = uint8_t(humidity);
	DEBUG_PRINT("\tCoded sent: ");
	DEBUG_HEX_PRINT(h_value, HEX);
	DEBUG_HEX_PRINT(s_value, HEX);
	DEBUG_PRINTLN("");
	buffer[0] = s_value & 0xFF; //lower byte
	buffer[1] = s_value >> 8;   //higher byte
	buffer[2] = h_value;
	buffer[3] = (int)(pub_ppm_25+0.5f); // just send the sensor values as int readings
	buffer[4] = (int)(pub_ppm_10+0.5f);
	// Check if there is not a current TX/RX job running
	if (LMIC.opmode & OP_TXRXPEND) {
		DEBUG_PRINTLN("OP_TXRXPEND, not sending");
	}
	else {
		// Prepare upstream data transmission at the next possible time.
		LMIC_setTxData2(1, (uint8_t*)buffer, 5, 0);
		DEBUG_PRINTLN("Sending: ");
	}
}

void abp_setup_lora()
{

	// LMIC init
	os_init();
	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();

	// Must be called after the settings values have been loaded by command.h
	LMIC_setSession(0x1, settings.lora_abp_DEVADDR, settings.lora_abp_NWKSKEY, settings.lora_abp_APPSKEY);

#if defined(CFG_eu868)
	// Set up the channels used by the Things Network, which corresponds
	// to the defaults of most gateways. Without this, only three base
	// channels from the LoRaWAN specification are used, which certainly
	// works, so it is good for debugging, but can overload those
	// frequencies, so be sure to configure the full frequency range of
	// your network here (unless your network autoconfigures them).
	// Setting up channels should happen after LMIC_setSession, as that
	// configures the minimal channel set.
	// NA-US channels 0-71 are configured automatically
	LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
	LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
	LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);      // g2-band
																					// TTN defines an additional channel at 869.525Mhz using SF9 for class B
																					// devices' ping slots. LMIC does not have an easy way to define set this
																					// frequency and support for class B is spotty and untested, so this
																					// frequency is not configured here.
#elif defined(CFG_us915)
	// NA-US channels 0-71 are configured automatically
	// but only one group of 8 should (a subband) should be active
	// TTN recommends the second sub band, 1 in a zero based count.
	// https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
	LMIC_selectSubBand(1);
#endif

	// Disable link check validation
	LMIC_setLinkCheckMode(0);

	// TTN uses SF9 for its RX2 window.
	LMIC.dn2Dr = DR_SF9;

	// Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
	LMIC_setDrTxpow(DR_SF7, 14);
}

void otaa_setup_lora() {
	LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
	os_init();
	// Reset the MAC state. Session and pending data transfers will be discarded.
	os_setCallback(&initjob, initfunc);
	LMIC_reset();
}


bool send_to_lora()
{
	if (!settings.lora_abp_active)
	{
		// Using otaa
		if (!lora_otaa_joined)
		{
			TRACELN("Waiting to join using otaa");
			return false;
		}
	}

	if (!OK_to_send)
	{
		TRACELN("waiting to send");
		return false;
	}

	do_send(&sendjob);    // Sent sensor values
	OK_to_send = false;
	return true;
}

void setup_lora()
{
	if (settings.lora_abp_active)
	{
		abp_setup_lora();
	}
	else
	{
		otaa_setup_lora();
	}
}

void loop_lora()
{
	os_runloop_once();
}
