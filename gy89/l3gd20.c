/*
 * l3gd20.c
 *
 *  Created on: Dec 24, 2018
 *      Author: gilg
 */


#include "l3gd20.h"



uint8_t read_reg_l3gd20(I2C_HandleTypeDef* i2c,uint8_t reg)
{
	HAL_I2C_Master_Transmit(i2c,l3gd20_addr,&reg,1,0xff);
	HAL_I2C_Master_Receive(i2c,l3gd20_addr,&reg,1,0xff);
	return reg;
}

void write_reg_l3gd20(I2C_HandleTypeDef* i2c,uint8_t reg,uint8_t parametr)
{
	uint8_t buffer[2];
	buffer[0]=reg;
	buffer[1]=parametr;
	HAL_I2C_Master_Transmit(i2c,l3gd20_addr,(uint8_t *)buffer, 2, 1000);
}


uint8_t Init_l3gd20(I2C_HandleTypeDef* i2c)
{
	if(read_reg_l3gd20(i2c,0x0f)!=0xd4)return 0;
	write_reg_l3gd20(i2c,CTRL_REG_1,CtrlReg1_DR_95hz|CtrlReg1_GYRO_ON|CtrlReg1_BW_3);
	write_reg_l3gd20(i2c,CTRL_REG_4,0x90);
	return 1;
}


void read_data_l3gd20(I2C_HandleTypeDef* i2c,int16_t* buff)
{
	uint8_t reg=OUT_X_L|0x80;
	uint8_t buf[6];
	HAL_I2C_Master_Transmit(i2c,l3gd20_addr,&reg,1,0xff);
	HAL_I2C_Master_Receive(i2c,l3gd20_addr,buf,6,0xff);
	buff[0]=buf[0]|(buf[1]<<8);
	buff[1]=buf[2]|(buf[3]<<8);
	buff[2]=buf[4]|(buf[5]<<8);
}


void read_data_l3gd20_d_(I2C_HandleTypeDef* i2c,double* buff,double a,double b,double c)
{
	uint8_t reg=OUT_X_L|0x80;
	uint8_t buf[6];
	HAL_I2C_Master_Transmit(i2c,l3gd20_addr,&reg,1,0xff);
	HAL_I2C_Master_Receive(i2c,l3gd20_addr,buf,6,0xff);
	buff[0]=((double)((int16_t)(buf[0]|(buf[1]<<8)))-a)*(double)0.0175;
	buff[1]=((double)((int16_t)(buf[2]|(buf[3]<<8)))-b)*(double)0.0175;
	buff[2]=((double)(int16_t)((buf[4]|(buf[5]<<8)))-c)*(double)0.0175;
}

void read_data_l3gd20_d(I2C_HandleTypeDef* i2c,double* buff)
{
	uint8_t reg=OUT_X_L|0x80;
	uint8_t buf[6];
	HAL_I2C_Master_Transmit(i2c,l3gd20_addr,&reg,1,0xff);
	HAL_I2C_Master_Receive(i2c,l3gd20_addr,buf,6,0xff);
	buff[0]=(double)(((int16_t)(buf[0]|(buf[1]<<8))))*(double)0.0175;
	buff[1]=(double)(((int16_t)(buf[2]|(buf[3]<<8))))*(double)0.0175;
	buff[2]=(double)((int16_t)((buf[4]|(buf[5]<<8))))*(double)0.0175;
}
