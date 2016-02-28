#include "mbed.h"
#include "barometer.h"
#include "sdLogger.h"

Barometer::Barometer(void): SDLogger("baro", "log"), BMP085(i2cDataBus)
{}

void Barometer::Init(void)
{}

void Barometer::Process(void)
{
	printf("%lu - %lu - %f\n\r", get_temperature(), get_pressure(), get_altitude_m());
	log_printf("%lu - %lu - %f\n\r", get_temperature(), get_pressure(), get_altitude_m());	
}
