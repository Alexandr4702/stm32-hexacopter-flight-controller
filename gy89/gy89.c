/*
 * gy89.c
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */

#include "gy89.h"

gy89i2c InitHandle_gy89(I2C_HandleTypeDef *i2c)
{
    gy89i2c gy89;
    gy89.i2c = i2c;

    return gy89;
}

uint8_t InitGy89(gy89i2c *handle)
{
    if (Init_bmp180(handle->i2c) != 1)
        return 0;
    if (Init_l3gd20(handle->i2c) != 1)
        return 0;
    if (Init_LSM303D(handle->i2c) != 1)
        return 0;
    return 1;
}
