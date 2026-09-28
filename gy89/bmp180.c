/*
 * bmp180.c
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#include "bmp180.h"

#include <math.h>

short AC1;
short AC2;
short AC3;
unsigned short AC4;
unsigned short AC5;
unsigned short AC6;
short B1;
short B2;
short MB;
short MC;
short MD;

long X1;
long X2;
long B5;

uint8_t oss = 0;
long B6;
long X3;
long B3;
unsigned long B4;
unsigned long B7;
long p;

uint8_t read_reg_bmp180(I2C_HandleTypeDef *i2c, uint8_t reg)
{
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, &reg, 1, 0xff);
    return reg;
}

void write_reg_bmp180(I2C_HandleTypeDef *i2c, uint8_t reg, uint8_t parametr)
{
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = parametr;
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, (uint8_t *)buffer, 2, 1000);
}

uint8_t Init_bmp180(I2C_HandleTypeDef *i2c)
{
    uint8_t str[100];
    if (read_reg_bmp180(i2c, 0xd0) != 0x55)
        return 0;

    uint8_t reg = 0xaa;
    uint8_t buf[22];
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, buf, 22, 0xff);

    AC1 = (buf[0] << 8) | buf[1];
    AC2 = (buf[2] << 8) | buf[3];
    AC3 = (buf[4] << 8) | buf[5];
    AC4 = (buf[6] << 8) | buf[7];
    AC5 = (buf[8] << 8) | buf[9];
    AC6 = (buf[10] << 8) | buf[11];
    B1 = (buf[12] << 8) | buf[13];
    B2 = (buf[14] << 8) | buf[15];
    MB = (buf[16] << 8) | buf[17];
    MC = (buf[18] << 8) | buf[19];
    MD = (buf[20] << 8) | buf[21];
    //---------------------------------------------------------------------------------------
    write_reg_bmp180(i2c, 0xf4, 0x2e);
    HAL_Delay(5);
    reg = 0xf6;
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, buf, 2, 0xff);
    long UT = (buf[0] << 8) | buf[1];
    //----------------------------------------------------------------------------------------
    uint8_t oss = 0;
    write_reg_bmp180(i2c, 0xf4, 0x34 + (oss << 6));
    HAL_Delay(5);
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, buf, 2, 0xff);
    long UP = (buf[0] << 8) | buf[1];
    //-----------------------------------------------------------------------------------------------------
    X1 = (UT - AC6) * AC5 / 32768;
    X2 = MC * 2048 / (X1 + MD);
    B5 = X1 + X2;
    long T = (X1 + X2 + 8) / 16;
    //-----------------------------------------------------------------------------------------------------

    B6 = B5 - 4000;
    X1 = B2 * B6 * B6 / 4096 / 2048;
    X2 = AC2 * B6 / 2048;
    X3 = X1 + X2;
    B3 = ((((AC1 * 4) + X3) << oss) + 2) / 4;
    X1 = AC3 * B6 / 8192;
    X2 = B1 * B6 * B6 / 4096 / 65536;
    X3 = (X1 + X2 + 2) / 4;
    B4 = AC4 * (unsigned long)(X3 + 32768) / 32768;
    B7 = ((unsigned long)UP - B3) * (50000 >> oss);
    if (B7 < 0x80000000)
    {
        p = (B7 * 2) / B4;
    }
    else
    {
        p = (B7 / B4) * 2;
    }
    X1 = p / 256 * p / 256;
    X1 = X1 * 3038 / 65536;
    X2 = (-7357 * p) / 65536;
    p = p + (X1 + X2 + 3791) / 4;

    double altitude = 44330.0 * (1 - pow((double)p / (double)101325, 1 / 5.255));

    return 1;
}

double get_pressure(I2C_HandleTypeDef *i2c)
{
    uint8_t buf[2];
    uint8_t reg = 0xaa;

    write_reg_bmp180(i2c, 0xf4, 0x2e);
    HAL_Delay(5);
    reg = 0xf6;
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, buf, 2, 0xff);
    long UT = (buf[0] << 8) | buf[1];
    //----------------------------------------------------------------------------------------
    uint8_t oss = 0;
    write_reg_bmp180(i2c, 0xf4, 0x34 + (oss << 6));
    HAL_Delay(5);
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, buf, 2, 0xff);
    long UP = (buf[0] << 8) | buf[1];
    //-----------------------------------------------------------------------------------------------------
    X1 = (UT - AC6) * AC5 / 32768;
    X2 = MC * 2048 / (X1 + MD);
    B5 = X1 + X2;
    // long T=(X1+X2+8)/16;
    //-----------------------------------------------------------------------------------------------------

    B6 = B5 - 4000;
    X1 = B2 * B6 * B6 / 4096 / 2048;
    X2 = AC2 * B6 / 2048;
    X3 = X1 + X2;
    B3 = ((((AC1 * 4) + X3) << oss) + 2) / 4;
    X1 = AC3 * B6 / 8192;
    X2 = B1 * B6 * B6 / 4096 / 65536;
    X3 = (X1 + X2 + 2) / 4;
    B4 = AC4 * (unsigned long)(X3 + 32768) / 32768;
    B7 = ((unsigned long)UP - B3) * (50000 >> oss);
    if (B7 < 0x80000000)
    {
        p = (B7 * 2) / B4;
    }
    else
    {
        p = (B7 / B4) * 2;
    }
    X1 = p / 256 * p / 256;
    X1 = X1 * 3038 / 65536;
    X2 = (-7357 * p) / 65536;
    p = p + (X1 + X2 + 3791) / 4;

    // double altitude =44330.0*(1-pow((double)p/(double)101325,1/5.255));
    return p;
}
