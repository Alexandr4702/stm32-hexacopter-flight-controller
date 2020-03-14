/*
 * bmp180.h
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#ifndef BMP180_H_
#define BMP180_H_

#include "stm32f7xx_hal.h"


#define bmp180_addr	0xEE

uint8_t Init_bmp180(I2C_HandleTypeDef* i2c);


#endif /* BMP180_H_ */
