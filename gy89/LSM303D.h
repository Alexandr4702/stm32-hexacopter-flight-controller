/*
 * LSM303D.h
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#ifndef LSM303D_H_
#define LSM303D_H_

#include "stm32f7xx_hal.h"

#define LSM303D_addr 0x3c

#define CTRL0 0x1f
#define CTRL1 0x20
#define CTRL2 0x21
#define CTRL3 0x22
#define CTRL4 0x23
#define CTRL5 0x24
#define CTRL6 0x25
#define CTRL7 0x26

#define STATUS_A 0x27

#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2a
#define OUT_Y_H_A 0x2b
#define OUT_Z_L_A 0x2c
#define OUT_Z_H_A 0x2d

#define STATUS_M 0x07

#define OUT_X_L_M 0x08
#define OUT_X_H_M 0x09
#define OUT_Y_L_M 0x0a
#define OUT_Y_H_M 0x0b
#define OUT_Z_L_M 0x0c
#define OUT_Z_H_M 0x0d

#define FIFO_CTRL 0x2e
#define FIFO_SRC 0x2f

#define CTRL2_2g 0x00
#define CTRL2_4g 0x08
#define CTRL2_6g 0x10
#define CTRL2_8g 0x18
#define CTRL2_16g 0x20

#define CTRL2_ABW_773 0x00
#define CTRL2_ABW_194 0x40
#define CTRL2_ABW_362 0x80
#define CTRL2_ABW_50 0xC0

#define CTRL5_TMP_EN 0x80

#define CTRL5_MODR_3_125 0x00
#define CTRL5_MODR_6_25 0x04
#define CTRL5_MODR_12_5 0x08
#define CTRL5_MODR_25 0x0C
#define CTRL5_MODR_50 0x10
#define CTRL5_MODR_100 0x14
#define CTRL5_MODR_not_use 0x18
#define CTRL5_MODR_reserved 0x1C

#define CTRL6_MFS_2 0x00
#define CTRL6_MFS_4 0x20
#define CTRL6_MFS_8 0x40
#define CTRL6_MFS_12 0x60

uint8_t Init_LSM303D(I2C_HandleTypeDef *i2c);
void read_data_LSM303D_A(I2C_HandleTypeDef *i2c, int16_t *buff);
void read_data_LSM303D_M(I2C_HandleTypeDef *i2c, int16_t *buff);
void read_data_LSM303D(I2C_HandleTypeDef *i2c, int16_t *accel, int16_t *mag);
void read_data_LSM303D_d(I2C_HandleTypeDef *i2c, double *accel, double *mag);

#endif /* LSM303D_H_ */
