#ifndef COMPASS_H
#define COMPASS_H

#include "I2cDataBus.h"
#include "sdLogger.h"
#include "HMC5883L/HMC5883L.h"

class Compass: SDLogger, public HMC5883L
{
private:
	float x, y, z, heading;
protected:
public:
	Compass(void);

    float GetMx();
    float GetMy();
    float GetMz();
    float GetHeading();

	void Init(void);
	void Process(void);
};

#endif