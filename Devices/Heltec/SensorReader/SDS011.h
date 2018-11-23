
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
	TRACELN(message);
}

void process_readings(float ppm25, float ppm10)
{
	TRACE("PPM 2.5:");
	TRACELN(ppm25);
	TRACE("PPM 10:");
	TRACELN(ppm10);

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
		TRACELN("Sensor in active mode");
	}
	else
	{
		TRACELN("Sensor in query mode");
	}
}

void dump_reading()
{
	for (int i = 0; i < message_pos; i++)
	{
		TRACE_HEX(message_text[i]);
		TRACE(" ");
	}
	TRACELN();
}

uint8_t get_checksum(uint8_t * byte_buffer, int byte_buffer_length)
{
	uint8_t check_sum = 0;

	TRACE("Check:");
	for (int i = 2; i < byte_buffer_length; i++)
	{
		TRACE_HEX(byte_buffer[i]);
		TRACE(" ");
		check_sum = check_sum + byte_buffer[i];
	}
	TRACELN();
	return check_sum;
}

bool check_checksum()
{
	int checksum_pos = message_pos - 1;

	uint8_t check_sum = get_checksum(message_text, checksum_pos);

	TRACE("Checksum:");
	TRACE_HEXLN(check_sum);
	TRACE("Received Checksum:");
	TRACE_HEXLN(message_text[checksum_pos]);

	return check_sum == message_text[checksum_pos];
}

void process_reading()
{
	TRACELN("Process reading");
	dump_reading();

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
	TRACELN("Process set device work");
	if (working)
		TRACELN("Device is working");
	else
		TRACELN("Device is asleep");
}

void process_query_device_work(bool working)
{
	TRACELN("Process query device work");
	if (working)
		TRACELN("Device is working");
	else
		TRACELN("Device is asleep");
}


void process_reporting_mode(bool active)
{
	TRACELN("Process reporting mode");
	if (active)
		TRACELN("Device is in active mode");
	else
		TRACELN("Device is query mode");
}

void process_device_messages()
{
	TRACELN("Processing device message");
	dump_reading();

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
	TRACELN("Beginning loading command");
	// Store the message start in the block
	buffer_byte(MESSAGE_START);
	state = waiting_for_command;
}

void begin_loading_reading()
{
	TRACELN("Beginning loading reading");
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
	TRACE("Pumping byte:");
	TRACE_HEXLN(b);

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

	TRACE("Calculated checksum:");
	TRACE_HEX(checksum);

	reporting_mode_command[checksum_pos] = checksum;
	for (int i = 0; i < block_length; i++)
	{

		TRACE("Writing byte:");
		TRACE_HEXLN(reporting_mode_command[i]);

		MySerial.write(reporting_mode_command[i]);
	}
}

void get_reporting_mode()
{
	TRACELN("Getting reporting mode");

	// select the command to query the reporting mode
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 2;
	reporting_mode_command[3] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void get_working_mode()
{
	TRACELN("Getting working mode");
	// select the command to query the reporting mode
	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 6;
	reporting_mode_command[3] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}


void set_reporting_mode(bool set_active_mode)
{

	if (set_active_mode)
		TRACELN("Setting active reporting mode");
	else
		TRACELN("Setting query reporting mode");

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
	TRACELN("Query the sensor");

	reporting_mode_command[1] = 0xB4;
	reporting_mode_command[2] = 4;
	reporting_mode_command[3] = 0;
	send_block(reporting_mode_command, sizeof(reporting_mode_command) / sizeof(uint8_t));
}

void set_sensor_working(bool working)
{
	if (working)
		TRACELN("Setting sensor working");
	else
		TRACELN("Setting sensor asleep");

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
	TRACELN("Setting up Sensor");

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

