/*
 * @file BMP085.h
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
 * Max sample rate: 128 samples/second (temperature at 1/second)
 *
 * Datasheet:
 *
 * http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Sensors/Pressure/BST-BMP085-DS000-06.pdf
 */

#ifndef BMP085_H
#define BMP085_H

#include "mbed.h"

class BMP085
{
public:
    /**
     * The I2C address that can be passed directly to i2c object (it's already shifted 1 bit left).
     */
    static const int16_t I2C_ADDRESS = 0xEE; //address of bmp085

    /**
     * Constructor.
     *
     * Calls init function
     *
     * @param sda - mbed pin to use for the SDA I2C line.
     * @param scl - mbed pin to use for the SCL I2C line.
     */
    #if 0
    BMP085(PinName sda, PinName scl);
    #endif
    /**
    * Constructor that accepts external i2c interface object.
    *
    * Calls init function
    *
    * @param i2c The I2C interface object to use.
    */
    BMP085(I2C &i2c) : i2c_(i2c) {
        init();
    }
    
    ~BMP085();
    
    /**
    * Sets the oss rate variables
    * Acceptable values  = 1,2,4,8
    *
    *@param oss the number of over sampling
    */
    void set_oss(int oss);

    int32_t get_temperature();
    int32_t get_pressure();
    double get_altitude_m();
    double get_altitude_ft();
    
    /**
    * Initialize sensor and get calibration values
    */
    void init();
    
    void display_cal_param(Serial *pc);

protected:
    

private:

    I2C &i2c_;

    /**
     * The raw buffer for allocating I2C object in its own without heap memory.
     */
    char i2cRaw[sizeof(I2C)];

    // calculation variables
    int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
    uint16_t  AC4, AC5, AC6;
    
    int32_t UT,UP; // uncompressed temperature and pressure value
    
    int32_t X1, X2, B5, temperature; // get_temperature variables
    
    int32_t B6, B3, X3, pressure; // get_pressure variables
    uint32_t B4, B7;
    
    double altitude;
    
    // setting variables
    char oss_bit_;
    char oversampling_setting_;
    float conversion_time_;

    void get_cal_param(); // get calibration parameters
    void get_ut(); // get uncompressed temperature
    void get_up(); // get uncompressed pressure
    void write_char(char,char);
    int16_t read_int16(char);
    uint16_t read_uint16(char);
    void read_multiple(char, char*, int);
};

#endif