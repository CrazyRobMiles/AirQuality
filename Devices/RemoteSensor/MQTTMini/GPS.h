char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

volatile bool ppsTriggered = false;

void ppsHandler(void);

void ppsHandler(void)
{
	ppsTriggered = true;
}

// Swallow unknown sentances for now - when we get bi-directional comms established we can 
// stop these from being transmitted
void printUnknownSentence(const MicroNMEA& nmea)
{
//	TRACELN();
//	TRACE("Unknown sentence: ");
//	TRACELN(nmea.getSentence());
}


void setup_gps()
{
	// receive on pin 33 - no transmit
	GPS_Serial.begin(9600);

	nmea.setUnknownSentenceHandler(printUnknownSentence);

	pub_got_gps_fix = false;
	pub_ticks_at_last_gps_update = 0;
}

#define console Serial

void loop_gps()
{
	while (GPS_Serial.available())
	{
		char ch = GPS_Serial.read();
		nmea.process(ch);

		if (nmea.isValid())
		{
			pub_latitude_mdeg = nmea.getLatitude();
			pub_longitude_mdeg = nmea.getLongitude();
			pub_got_gps_fix = true;
			pub_ticks_at_last_gps_update = millis();
		}
	}
}
