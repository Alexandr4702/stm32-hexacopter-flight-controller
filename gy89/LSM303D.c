/*
 * LSM303D.c
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#include "LSM303D.h"

static uint8_t read_reg_LSM303D(I2C_HandleTypeDef *i2c, uint8_t reg)
{
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, &reg, 1, 0xff);
    return reg;
}

static void write_reg_LSM303D(I2C_HandleTypeDef *i2c, uint8_t reg, uint8_t parameter)
{
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = parameter;
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, (uint8_t *)buffer, 2, 1000);
}

uint8_t Init_LSM303D(I2C_HandleTypeDef *i2c)
{

    if (read_reg_LSM303D(i2c, 0x0f) != 0x49)
        return 0;

    write_reg_LSM303D(i2c, CTRL0, 0x00);
    write_reg_LSM303D(i2c, CTRL1, 0x67);
    write_reg_LSM303D(i2c, CTRL2, CTRL2_ABW_773 | CTRL2_6g);

    write_reg_LSM303D(i2c, CTRL3, 0x00);
    write_reg_LSM303D(i2c, CTRL4, 0x00);

    write_reg_LSM303D(i2c, CTRL5, CTRL5_TMP_EN | CTRL5_MODR_100);
    write_reg_LSM303D(i2c, CTRL6, CTRL6_MFS_4);
    write_reg_LSM303D(i2c, CTRL7, 0x00);

    return 1;
}

void read_data_LSM303D_A(I2C_HandleTypeDef *i2c, int16_t *buff)
{
    uint8_t reg = OUT_X_L_A | 0x80;
    uint8_t buf[6];
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, buf, 6, 0xff);
    buff[0] = buf[0] | (buf[1] << 8);
    buff[1] = buf[2] | (buf[3] << 8);
    buff[2] = buf[4] | (buf[5] << 8);
}

void read_data_LSM303D_M(I2C_HandleTypeDef *i2c, int16_t *buff)
{
    uint8_t reg = OUT_X_L_M | 0x80;
    uint8_t buf[6];
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, buf, 6, 0xff);
    buff[0] = buf[0] | (buf[1] << 8);
    buff[1] = buf[2] | (buf[3] << 8);
    buff[2] = buf[4] | (buf[5] << 8);
}

void read_data_LSM303D(I2C_HandleTypeDef *i2c, int16_t *accel, int16_t *mag)
{
    uint8_t reg = OUT_X_L_A | 0x80;
    uint8_t buf[6];
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, buf, 6, 0xff);
    accel[0] = buf[0] | (buf[1] << 8);
    accel[1] = buf[2] | (buf[3] << 8);
    accel[2] = buf[4] | (buf[5] << 8);

    reg = OUT_X_L_M | 0x80;
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, buf, 6, 0xff);
    mag[0] = buf[0] | (buf[1] << 8);
    mag[1] = buf[2] | (buf[3] << 8);
    mag[2] = buf[4] | (buf[5] << 8);
}

void read_data_LSM303D_d(I2C_HandleTypeDef *i2c, double *accel, double *mag)
{
    uint8_t reg = OUT_X_L_A | 0x80;
    uint8_t buf[6];
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, buf, 6, 0xff);
    accel[0] = (double)((int16_t)(buf[0] | (buf[1] << 8))) * (double)0.000183;
    accel[1] = (double)((int16_t)(buf[2] | (buf[3] << 8))) * (double)0.000183;
    accel[2] = (double)((int16_t)(buf[4] | (buf[5] << 8))) * (double)0.000183;

    reg = OUT_X_L_M | 0x80;
    HAL_I2C_Master_Transmit(i2c, LSM303D_addr, &reg, 1, 0xff);
    HAL_I2C_Master_Receive(i2c, LSM303D_addr, buf, 6, 0xff);
    mag[0] = (double)((int16_t)(buf[0] | (buf[1] << 8))) * (double)0.000160;
    mag[1] = (double)((int16_t)(buf[2] | (buf[3] << 8))) * (double)0.000160;
    mag[2] = (double)((int16_t)(buf[4] | (buf[5] << 8))) * (double)0.000160;
}
