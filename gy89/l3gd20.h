/*
 * l3gd20.h
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#ifndef L3GD20_H_
#define L3GD20_H_

#include "main.h"

#define WHO_AM_I_ADDR 0x0F
#define I_AM 0xD4
#define OUT_X_L 0x28
#define CTRL_REG_1 0x20
#define CTRL_REG_3 0x22
#define CTRL_REG_4 0x23
#define DUMMY_BYTE 0x00

#define l3gd20_addr 0xD4

#define CtrlReg1_DR_95hz 0x00
#define CtrlReg1_DR_190hz 0x40
#define CtrlReg1_DR_380hz 0x80
#define CtrlReg1_DR_760hz 0xC0

#define CtrlReg1_BW_0 0x00
#define CtrlReg1_BW_1 0x10
#define CtrlReg1_BW_2 0x20
#define CtrlReg1_BW_3 0x30

#define CtrlReg1_GYRO_ON 0x0f

uint8_t Init_l3gd20(I2C_HandleTypeDef *i2c);
void read_data_l3gd20_d(I2C_HandleTypeDef *i2c, double *buff);
void read_data_l3gd20_d_(I2C_HandleTypeDef *i2c, double *buff, double a, double b, double c);
void read_data_l3gd20(I2C_HandleTypeDef *i2c, int16_t *buff);

#endif /* L3GD20_H_ */
