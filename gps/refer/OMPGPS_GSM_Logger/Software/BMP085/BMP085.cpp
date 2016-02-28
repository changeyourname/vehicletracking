/*
 * @file BMP085.cpp
 * @author Tyler Weaver
 * @author Kory Hill
 *
 * @section LICENSE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * BMP085 I2C Temperature/Pressure/Altitude Sensor
 *
 * Datasheet:
 *
 * http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Pressure/BST-BMP085-DS000-06.pdf
 */

#include "BMP085.h"
#include <new>

#if 0
BMP085::BMP085(PinName sda, PinName scl) : i2c_(*reinterpret_cast<I2C*>(i2cRaw))
{
    // Placement new to avoid additional heap memory allocation.
    new(i2cRaw) I2C(sda, scl);

    init();
}
#endif

BMP085::~BMP085()
{
    // If the I2C object is initialized in the buffer in this object, call destructor of it.
    if(&i2c_ == reinterpret_cast<I2C*>(&i2cRaw))
        reinterpret_cast<I2C*>(&i2cRaw)->~I2C();
}

void BMP085::init()
{
    get_cal_param();
    set_oss(8); // standard over sampling (2 samples)
    get_ut(); // initalize values
    get_up();
}

void BMP085::get_cal_param()
{
    // get calibration values
    AC1 = read_int16(0xAA);
    AC2 = read_int16(0xAC);
    AC3 = read_int16(0xAE);
    AC4 = read_uint16(0xB0);
    AC5 = read_uint16(0xB2);
    AC6 = read_uint16(0xB4);
    B1  = read_int16(0xB6);
    B2  = read_int16(0xB8);
    MB  = read_int16(0xBA);
    MC  = read_int16(0xBC);
    MD  = read_int16(0xBE);
}

void BMP085::display_cal_param(Serial *pc)
{
    pc->printf("AC1 %x\r\n", AC1);
    pc->printf("AC2 %x\r\n", AC2);
    pc->printf("AC3 %x\r\n", AC3);
    pc->printf("AC4 %x\r\n", AC4);
    pc->printf("AC5 %x\r\n", AC5);
    pc->printf("AC6 %x\r\n", AC6);
    pc->printf("B1 %x\r\n", B1);
    pc->printf("B2 %x\r\n", B2);
    pc->printf("MB %x\r\n", MB);
    pc->printf("MC %x\r\n", MC);
    pc->printf("MD %x\r\n", MD);
}

void BMP085::get_ut()
{
    write_char(0xF4,0x2E);
    wait(0.045);
    char buffer[3];
    read_multiple(0xF6, buffer, 2);
    
    UT = (buffer[0]<<8) | buffer[1];
}

void BMP085::get_up()
{
    // set sample setting
    write_char(0xF4, oversampling_setting_);
    
    // wait
    wait(conversion_time_);
    
    // read the three bits
    char buffer[3];
    read_multiple(0xF6, buffer, 3);
    
    UP = ((buffer[0]<<16) | (buffer[1]<<8) | (buffer[2])) >> (8 - oss_bit_);
}

void BMP085::set_oss(int oss)
{
    switch(oss) {
        case 1: // low power
            oss_bit_ = 0;
            oversampling_setting_ = 0x34;
            conversion_time_ = 0.045;
            break;
        case 2: // standard
            oss_bit_ = 1;
            oversampling_setting_ = 0x74;
            conversion_time_ = 0.075;
            break;
        case 4: // high resolution
            oss_bit_ = 2;
            oversampling_setting_ = 0xB4;
            conversion_time_ = 0.135;
            break;
        case 8: // ultra high resolution
            oss_bit_ = 3;
            oversampling_setting_ = 0xF4;
            conversion_time_ = 0.255;
            break;
        default: // standard
            oss_bit_ = 1;
            oversampling_setting_ = 0x74;
            conversion_time_ = 0.075;
    }
}

void BMP085::write_char(char address, char data)
{
    char cmd[2];
    cmd[0] = address;
    cmd[1] = data;
    i2c_.write( I2C_ADDRESS , cmd, 2);
}

int16_t BMP085::read_int16(char address)
{
    char tx[2];
    tx[0] = address;
    i2c_.write(I2C_ADDRESS, tx, 1);
    i2c_.read(I2C_ADDRESS, tx, 2);
    int16_t value = ((tx[0] << 8) | tx[1]);
    return value;
}

void BMP085::read_multiple(char address, char* buffer, int bits)
{
    char cmd[2];
    cmd[0] = address;
    i2c_.write(I2C_ADDRESS, cmd, 1);
    i2c_.read(I2C_ADDRESS, buffer, bits);
}

uint16_t BMP085::read_uint16(char address)
{
    char tx[2];
    tx[0] = address;
    i2c_.write(I2C_ADDRESS, tx, 1);
    i2c_.read(I2C_ADDRESS, tx, 2);
    uint16_t value = ((tx[0] << 8) | tx[1]);
    return value;
}

int32_t BMP085::get_temperature()
{
    get_ut(); // uncompressed temperature
    get_up(); // uncompressed pressure
    
    long x1,x2;

    //Start temperature calculation
    x1 = ((UT - AC6) * AC5) >> 15;
    x2 = (MC << 11) / (x1 + MD);
    B5 = x1 + x2;
    temperature = ((B5 + 8) >> 4);
    return temperature;
}

int32_t BMP085::get_pressure()
{
    get_up(); // get uncompressed pressure (uncompressed temperature must be called recently)

    B6 = B5 - 4000;
    X1 = (B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((((long)(AC1) * 4) + X3) << oss_bit_) + 2) >> 2);
    X1 = (AC3 * B6) >> 13;
    X2 = (B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (AC4 * (unsigned long)(X3 + 32768)) >> 15;
    B7 = ((unsigned long)(UP - B3) * (50000 >> oss_bit_));
    if (B7 < 0x80000000)
        pressure = (B7 << 1) / B4;
    else
        pressure = (B7 / B4) << 1;

    X1 = (pressure >> 8);
    X1 *= X1;
    X1 = (X1 * 3038) >>16;
    X2 = (-7357 * pressure) >>16;
    pressure += (X1 + X2 + 3791)>>4;
    
    return pressure;
}

double BMP085::get_altitude_m()
{
    const double P0 = 1013.25; //pressure at sea level in hPa
    double pres_hpa = pressure / 100.0;
    altitude = 44330.0 * (1.0 - pow((pres_hpa/P0), 0.190295));
    return altitude;
}

double BMP085::get_altitude_ft()
{
    return get_altitude_m()*3.28084;
}