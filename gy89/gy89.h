/*
 * gy89.h
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#ifndef GY89_H_
#define GY89_H_

#include "main.h"
#include "bmp180.h"
#include "LSM303D.h"
#include "l3gd20.h"

typedef struct
{
    I2C_HandleTypeDef *i2c;

} gy89i2c;

typedef struct
{

} gy89spi;

gy89i2c InitHandle_gy89(I2C_HandleTypeDef *i2c);
uint8_t InitGy89(gy89i2c *handle);

#endif /* GY89_H_ */
