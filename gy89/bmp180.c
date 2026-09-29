/*
 * bmp180.c
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#include "bmp180.h"

static short AC1;
static short AC2;
static short AC3;
static unsigned short AC4;
static unsigned short AC5;
static unsigned short AC6;
static short B1;
static short B2;
static short MB;
static short MC;
static short MD;

static long X1;
static long X2;
static long B5;

static long B6;
static long X3;
static long B3;
static unsigned long B4;
static unsigned long B7;
static long p;

static uint8_t read_reg_bmp180(I2C_HandleTypeDef *i2c, uint8_t reg)
{
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, bmp180_addr, &reg, 1, 0xff);
    return reg;
}

static void write_reg_bmp180(I2C_HandleTypeDef *i2c, uint8_t reg, uint8_t parameter)
{
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = parameter;
    HAL_I2C_Master_Transmit(i2c, bmp180_addr, (uint8_t *)buffer, 2, 1000);
}

uint8_t Init_bmp180(I2C_HandleTypeDef *i2c)
{
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

    return 1;
}
