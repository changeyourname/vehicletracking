#include "HMC5883L.h"

HMC5883L::HMC5883L(PinName sda, PinName scl): i2c_(sda, scl)
{
    //100KHz, as specified by the datasheet.
    char rx;
    
    i2c_.frequency(100000);
    //Testar depois com 400KHz
    //==========================================================================================================
    // Read chip_id
    //==========================================================================================================
    rx = Read(HMC5883L_IDENT_A);
    if (rx != 0x48)//ID do chip
        printf("\ninvalid chip id %d\r\n", rx); 
     
    //==========================================================================================================
    // Let's set the Configuration Register A
    //==========================================================================================================
    // This register set's the number of samples averaged per measurement output, the rate at which data is written
    // to all three data output registers and the measurement flow of the device.
    // -------------------------------------------------------
    // |CRA7 CRA6   CRA5   CRA4   CRA3   CRA2   CRA1   CRA0  |
    // |(1)  MA1(1) MA0(1) DO2(1) DO1(0) DO0(0) MS1(0) MS0(0)| -> This is the default value
    // -------------------------------------------------------
    // CRA7 -> we have to clear this bit for correct operation                                      (0)
    // CRA6 to CRA5 -> Let's select the maximum number of samples averaged per measurement output   (11)
    // CRA4 to CRA2 -> Also let's select the maximum data output rate                               (110)
    // CRA1 to CRA0 -> The measurement flow is defined to normal                                    (00)
    // -------------------------------------------------------
    // |CRA7 CRA6   CRA5   CRA4   CRA3   CRA2   CRA1   CRA0  |
    // |(0)  MA1(1) MA0(1) DO2(1) DO1(1) DO0(0) MS1(0) MS0(0)| -> This is the new value, 0x78 in hex
    // -------------------------------------------------------
    //Write(HMC5883L_CONFIG_A,0x78);
    //Write(HMC5883L_CONFIG_A,0x70);
        
    //==========================================================================================================
    // The Configuration Register B is set to 0010 0000 by default, this is a +/- 1.3 Ga sensor field range and
    // the gain of LSB/gauss is 1090. This is the maximum value, so let's leave it like that.
    //==========================================================================================================
    //Datasheet page 13. I will explain later
    //Write(HMC5883L_CONFIG_B,0x20);
    //Write(HMC5883L_CONFIG_B,0xA0);
  
    //==========================================================================================================
    // Let's set the Mode Register
    //==========================================================================================================
    // This register set's the operation mode, from continuous-measurements mode, single-measurement mode and idle mode.
    // We will set to Continuouse-measurement mode, so the device continuously performs measurements and places the
    // result in the data register
    // ---------------------------------------------
    // |MR7  MR6  MR5  MR4  MR3  MR2  MR1    MR0   |  -> This is the new value, 0x78 in hex, we are going to change
    // |(1)  (0)  (0)  (0)  (0)  (0)  MD1(0) MD0(1)|     the MD1 and MD0 to 00 and clear the MR7 for correct operation.
    // ---------------------------------------------     The final value is 0000 0000 (0x00).
    Write(HMC5883L_MODE,0x00);
}

HMC5883L::HMC5883L(I2C &i2c) : i2c_(i2c) 
{
    char rx;
    
    rx = Read(HMC5883L_IDENT_A);
    if (rx != 0x48)//ID do chip
        printf("\ninvalid chip id %d\r\n", rx); 

    Write(HMC5883L_MODE,0x00);
}


void HMC5883L::Write(char reg_address, char data)
{
    char tx[2];
    tx[0]=reg_address;
    tx[1]=data;

    i2c_.write(HMC5883L_I2C_WRITE,tx,2);
}

char HMC5883L::Read(char data)
{
    char tx = data;
    char rx;

    i2c_.write(HMC5883L_I2C_WRITE, &tx, 1);
    i2c_.read(HMC5883L_I2C_READ, &rx, 1);
    return rx;
}

void HMC5883L::MultiByteRead(char address, char* output, int size) 
{
    i2c_.write(HMC5883L_I2C_WRITE, &address, 1);      //tell it where to read from
    i2c_.read(HMC5883L_I2C_READ, output, size);     //tell it where to store the data read
}

float HMC5883L::getMx()
{
    //return (x * m_Scale);
    char lsb_byte = 0;
    signed short msb_byte;

    lsb_byte = Read(HMC5883L_X_MSB);
    msb_byte = lsb_byte << 8;
    msb_byte |= Read(HMC5883L_X_LSB);
    return (float)msb_byte;
    /*
    char tx[1];
    char rx[2];


    tx[0]=HMC5883L_X_MSB;
    i2c_.write(HMC5883L_I2C_READ,tx,1);
    i2c_.read(HMC5883L_I2C_READ,rx,2);
    return ((int)rx[0]<<8|(int)rx[1]);
    */

}

float HMC5883L::getMy()
{
    //return (y * m_Scale);

    char lsb_byte = 0;
    signed short msb_byte;

    lsb_byte = Read(HMC5883L_Y_MSB);
    msb_byte = lsb_byte << 8;
    msb_byte |= Read(HMC5883L_Y_LSB);
    return (float)msb_byte;
}


float HMC5883L::getMz()
{
    //return (z * m_Scale);
    
    char lsb_byte = 0;
    signed short msb_byte;

    lsb_byte = Read(HMC5883L_Z_MSB);
    msb_byte = lsb_byte << 8;
    msb_byte |= Read(HMC5883L_Z_LSB);
    return (float)msb_byte;
 }