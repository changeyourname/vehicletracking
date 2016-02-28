#ifndef BAROMETER_H
#define BAROMETER_H

#include "I2cDataBus.h"
#include "sdLogger.h"
#include "BMP085/BMP085.h"

class Barometer: SDLogger, public BMP085
{
private:
protected:
public:
	Barometer(void);

	void Init(void);
	void Process(void);
};

#endif
