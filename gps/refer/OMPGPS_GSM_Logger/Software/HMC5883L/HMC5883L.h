#ifndef HMC5883L_H
#define HMC5883L_H

#include "mbed.h"

#define HMC5883L_IDENT_A        0x0A // In this case the identification register A is used to identify the devide. ASCII value H
#define HMC5883L_I2C            0x1E // 7-bit address. 0x3C write, 0x3D read.
#define HMC5883L_I2C_WRITE      0x3C // Same as (& 0xFE), ensure that the MSB bit is being set to zero (RW=0 -> Writing)
#define HMC5883L_I2C_READ       0x3D // Same as (| 0x01), ensure that the MSB bit is being set to one  (RW=1 -> Reading)

#define HMC5883L_CONFIG_A       0x00
#define HMC5883L_CONFIG_B       0x01
#define HMC5883L_MODE           0x02
#define HMC5883L_STATUS         0x09

#define HMC5883L_X_MSB          0x03
#define HMC5883L_X_LSB          0x04
#define HMC5883L_Z_MSB          0x05
#define HMC5883L_Z_LSB          0x06
#define HMC5883L_Y_MSB          0x07
#define HMC5883L_Y_LSB          0x08


class HMC5883L 
{

public:

    HMC5883L(PinName sda, PinName scl);
    HMC5883L(I2C &i2c);

    float getMx();
    float getMy();
    float getMz();
private:
    void Write(char reg_address, char data);
    char Read(char data);
    void MultiByteRead(char address, char* output, int size); 
    I2C i2c_;
    
};

#endif /* HMC5883L_H */