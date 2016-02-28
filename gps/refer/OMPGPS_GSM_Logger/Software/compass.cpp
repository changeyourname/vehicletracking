#include "compass.h"

#define PI       3.14159265

Compass::Compass(void): SDLogger("compass", "log"), HMC5883L(i2cDataBus)
{}

void Compass::Init(void)
{}

float Compass::GetMx()
{
   x = getMx();
   return(x);
}

float Compass::GetMy()
{
   y = getMy();
   return(y);
}

float Compass::GetMz()
{
   y = getMy();
   return(z);
}

float Compass::GetHeading()
{
   x = getMx();
   y = getMy();
   z = getMz();

   heading = atan2(y, x);
   if(heading < 0) 
       heading += 2*PI;
   if(heading > 2*PI) 
       heading -= 2*PI;
   
   heading = heading * 180 / PI;
   return(heading);
}

void Compass::Process(void)
{
	GetHeading();
	
	printf    ("x: %f \t\ty: %f \t\t z: %f \t\t heading: %f \t\r\n", x, y, z, heading);
	log_printf("x: %f \t\ty: %f \t\t z: %f \t\t heading: %f \t\r\n", x, y, z, heading);
}
