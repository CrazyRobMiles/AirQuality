
// SDS011 Driver code
// Uses a message pump so that we don't tie up the processor for the time it takes
// to receive a reading

#include <HardwareSerial.h>

HardwareSerial MySerial(1);

#define MESSAGE_START 0xAA

#define READING_COMMAND 0xC0
#define READING_LENGTH 9

#define REPORTING_MODE_COMMAND 0xC5
#define REPORTING_MODE_LENGTH 9

#define MAX_MESSAGE_LENGTH 20

uint8_t message_text[MAX_MESSAGE_LENGTH];

int message_pos = 0;

void message_error(String message)
{
	Serial.println(message);
}

void process_readings(float ppm25, float ppm10)
{
#ifdef VERBOSE
	Serial.print("PPM 2.5:");
	Serial.println(ppm25);
	Serial.print("PPM 10:");
	Serial.println(ppm10);
#endif

	// Only update the reading value if we are processing

	if (pub_air_values_ready)
		return;

	if (pub_millis_to_next_update > 0)
		return;

	pub_ppm_10 = ppm10;
	pub_ppm_25 = ppm25;

	pub_air_values_ready = true;
}

void process_report_status(bool active_mode)
{
	if (active_mode)
	{
		Serial.println("Sensor in active mode");
	}
	else
	{
		Serial.println("Sensor in query mode");
	}
}

void dump_reading()
{
	for (int i = 0; i < message_pos; i++)
	{
		Serial.print(message_text[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
}

uint8_t get_checksum(uint8_t * byte_buffer, int byte_buffer_length)
{
	uint8_t check_sum = 0;

#ifdef VERBOSE
	Serial.print("Check:");
#endif
	for (int i = 2; i < byte_buffer_length; i++)
	{
#ifdef VERBOSE
		Serial.print(byte_buffer[i], HEX);
		Serial.print(" ");
#endif
		check_sum = check_sum + byte_buffer[i];
	}
#ifdef VERBOSE
	Serial.println();
#endif
	return check_sum;
}

bool check_checksum()
{
	int checksum_pos = message_pos - 1;

	uint8_t check_sum = get_checksum(message_text, checksum_pos);

#ifdef VERBOSE
	Serial.print("Checksum:");
	Serial.println(check_sum, HEX);
	Serial.print("Received Checksum:");
	Serial.println(message_text[checksum_pos], HEX);
#endif

	return check_sum == message_text[checksum_pos];
}

void process_reading()
{
#ifdef VERBOSE
	Serial.println("Process reading");
	dump_reading();
#endif

	if (!check_checksum())
	{
		message_error("Invalid checksum");
		return;
	}

	float ppm_10 = ((int)message_text[4] + ((int)message_text[5] * 256)) / 10.0;
	float ppm_25 = ((int)message_text[2] + ((int)message_text[3] * 256)) / 10.0;
	process_readings(ppm_25, ppm_10);
}

void process_set_device_ID(uint8_t ID1, uint8_t ID2)
{
	TRACE("Set device ID to:");
	TRACE_HEX(ID1);
	TRACE_HEX(ID2);
	TRACELN();
}

void process_set_device_work(bool working)
{
#ifdef VERBOSE
	Serial.println("Process set device work");
#endif
	if (working)
		Serial.println("Device is working");
	else
		Serial.println("Device is asleep");
}

void process_query_device_work(bool working)
{
#ifdef VERBOSE
	Serial.println("Process query device work");
#endif
	if (working)
		Serial.println("Device is working");
	else
		Serial.println("Device is asleep");
}


void process_reporting_mode(bool active)
{
#ifdef VERBOSE
	Serial.println("Process reporting mode");
#endif
	if (active)
		Serial.println("Device is in active mode");
	else
		Serial.println("Device is query mode");
}

void process_device_messages()
{
#ifdef VERBOSE
	Serial.println("Processing device message");
	dump_reading();
#endif

	if (!check_checksum())
	{
		message_error("Invalid checksum");
		return;
	}
	switch (message_text[2]) {
	case 2: // set reporting modde
		process_set_device_ID(message_text[6], message_text[7]);
		break;
	case 5: // set device ID
		process_set_device_ID(message_text[6], message_text[7]);
		break;
	case 6: // set sleep and work
		if (message_text[3] == 0)
			process_query_device_work(message_text[4] == 1);
		else
			process_set_device_work(message_text[4] == 1);
		break;

	}
}


enum Message_State { waiting_for_start, waiting_for_command, loading_reading, loading_reporting_mode };

Message_State state;

void reset_buffer()
{
	message_pos = 0;
}

void buffer_byte(uint8_t b)
{
	message_text[message_pos] = b;
	message_pos++;
}

void reset_message()
{
	state = waiting_for_start;
	reset_buffer();
}

void begin_loading_command()
{
#ifdef VERBOSE
	Serial.println("Beginning loading command");
#endif
	// Store the message start in the block
	buffer_byte(MESSAGE_START);
	state = waiting_for_command;
}

void begin_loading_reading()
{
#ifdef VERBOSE
	Serial.println("Beginning loading reading");
#endif
	buffer_byte(READING_COMMAND);
	state = loading_reading;
}

void begin_loading_reporting_mode()
{
	buffer_byte(REPORTING_MODE_COMMAND);
	state = loading_reporting_mode;
}

void pump_byte(uint8_t b)
{
#ifdef VERBOSE
	Serial.print("Pumping byte:");
	Serial.println(b, HEX);
#endif

	switch (state)
	{
	case waiting_for_start:
		if (b == MESSAGE_START)
		{
			begin_loading_command();
		}
		break;

	case waiting_for_command:
		switch (b)
		{
		case READING_COMMAND:
			begin_loading_reading();
			break;
		case REPORTING_MODE_COMMAND:
			begin_loading_reporting_mode();
			break;

		}
		break;

	case loading_reading:
		if (message_pos < READING_LENGTH)
		{
			buffer_byte(b);
		}
		else
		{
			process_reading();
			reset_message();
		}
		break;

	case loading_reporting_mode:
		if (message_pos < REPORTING_MODE_LENGTH)
		{
			buffer_byte(b);
		}
		else
		{
			process_device_messages();
			reset_message();
		}
	}
}

enum Reporting_Mode { report_active, report_query };

uint8_t reporting_mode_command[] = {
  0xAA, //  00  start of command
  0xB4, //  01  command ID - 0xb4 means read - 0xc5 means write
  0x02, //  02  data byte 1 always 2
  0x00, //  03  data byte 2 - 0 means query 1 means set
  0x00, //  04  data byte 3 - 0 means report active 1 means report query
  0x00, //  05  data byte 4- 0 (reserved) 
  0x00, //  06  data byte 5 - 0 (reserved) or Device ID byte 1
  0x00, //  07  data byte 6 - 0 (reserved) or Device ID byte 2
  0x00, //  08  data byte 7 - 0
  0x00, //  09  data byte 8 - 0
  0x00, //  10  data byte 9 - 0
  0x00, //  11  data byte 10 - 0
  0x00, //  12  data byte 11 - 0
  0x00, //  13  data byte 12 - 0
  0x00, //  14  data byte 13 - 0
  0xFF, //  15  0xFF means all sensors or device ID byte 1
  0xFF, //  16  0xFF means all sensors or device ID byte 2
  0x00, //  17  checksum
  0xAB  //  18  tail
};

void send_block(uint8_t * block_base, int block_length)
{
	int checksum_pos = block_length - 2;
	uint8_t checksum = get_checksum(reporting_mode_command, checksum_pos);
#ifdef VERBOSE
	Serial.print("Calculated checksum:");
	Serial.println(checksum, HEX);
#endif
	reporting_mode_command[checksum_pos] = checksum;
	for (int i = 0; i < block_length; i++)
	{
#ifdef VERBOSE
		Serial.print("Writing byte:");
		Serial.println(reporting_mode_command[i], HEX);
#endif
		MySerial.write(reporting_mode_command[i]);
}
}

void get_reporting_mode()
{
#ifdef VERBOSE
	Serial.println("Getting reporting mode");
#endif
	// select the command to query the reporting mode
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 2;
	reporting_mode_command[3] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void get_working_mode()
{
#ifdef VERBOSE
	Serial.println("Getting working mode");
#endif
	// select the command to query the reporting mode
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 6;
	reporting_mode_command[3] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}


void set_reporting_mode(bool set_active_mode)
{

#ifdef VERBOSE
	if (set_active_mode)
		Serial.println("Setting active reporting mode");
	else
		Serial.println("Setting query reporting mode");
#endif

	// select the command to set the reporting mode
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 2;
	reporting_mode_command[3] = 1;
	if (set_active_mode)
		reporting_mode_command[4] = 0;
	else
		reporting_mode_command[4] = 1;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void query_sensor()
{
#ifdef VERBOSE
	Serial.println("Query the sensor");
#endif
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 4;
	reporting_mode_command[3] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void set_sensor_working(bool working)
{
#ifdef VERBOSE
	if (working)
		Serial.println("Setting sensor working");
	else
		Serial.println("Setting sensor asleep");
#endif
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 6;
	reporting_mode_command[3] = 1;
	if (working)
		reporting_mode_command[4] = 1;
	else
		reporting_mode_command[4] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void setup_sensor()
{
#ifdef VERBOSE
	Serial.println("Setting up Sensor");
#endif
	MySerial.begin(9600, SERIAL_8N1, 17, -1);  // badud, mode, rx, tx
	reset_message();
	pub_air_values_ready = false;
	set_reporting_mode(true);
}

void loop_sensor()
{
	while (MySerial.available() > 0) {
		uint8_t byteFromSerial = MySerial.read();
		pump_byte(byteFromSerial);
	}
}

