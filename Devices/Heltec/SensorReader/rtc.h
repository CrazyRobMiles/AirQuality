
MD_DS3231 RTC(&heltecWire);

void setup_rtc()
{
	//RTC.yyyy = 2018;
	//RTC.mm = 8;
	//RTC.dd = 28;
	//RTC.h = 9;
	//RTC.m = 14;
	//RTC.s = 0;
	//RTC.writeTime();
}

void set_time(uint8_t hour, uint8_t minute, uint8_t second, 
	uint8_t day, uint8_t month, uint16_t year)
{
	RTC.h = hour;
	RTC.m = minute;
	RTC.s = second;

	RTC.dd = day;
	RTC.mm = month;
	RTC.yyyy = year;

	// Now work out the day of the week

	RTC.dow = RTC.calcDoW(year, month, day);

	RTC.writeTime();
}

char * dayNames[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

char * monthNames[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void read_time()
{
	RTC.readTime();
	pub_hour = RTC.h;
	pub_minute = RTC.m;
	pub_second = RTC.s;
	pub_day_of_week = RTC.dow;

	pub_day = RTC.dd;
	pub_month = RTC.mm;
	pub_year = RTC.yyyy;
}

void loop_rtc()
{
	read_time();
}