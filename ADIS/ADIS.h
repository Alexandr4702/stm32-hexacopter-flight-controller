/*
 * ADIS.h
 *
 *  Created on: Oct 25, 2017
 *      Author: root
 */

#ifndef ADIS_H_
#define ADIS_H_

#include "stm32f7xx_hal.h"

#define CS_GPIO_PORT GPIOC
#define CS_PIN GPIO_PIN_5
#define CS_ON HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET)
#define CS_OFF HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET)

#define DUMMY_WORD ((uint16_t)0x0000)
#define READWRITE_CMD 0x8000
#define who_am_i 0x7E00

#define TEMP_OUT 0x0E00
#define X_GYRO_LOW 0x1000
#define X_GYRO_OUT 0x1200
#define Y_GYRO_LOW 0x1400
#define Y_GYRO_OUT 0x1600
#define Z_GYRO_LOW 0x1800
#define Z_GYRO_OUT 0x1A00
#define X_ACCL_LOW 0x1C00
#define X_ACCL_OUT 0x1E00
#define Y_ACCL_LOW 0x2000
#define Y_ACCL_OUT 0x2200
#define Z_ACCL_LOW 0x2400
#define Z_ACCL_OUT 0x2600
#define X_MAGN_OUT 0x2800
#define Y_MAGN_OUT 0x2A00
#define Z_MAGN_OUT 0x2C00
#define BAROM_LOW 0x2E00
#define BAROM_OUT 0x3000

#define SYS_E_FLAG 0x0800
#define FNCTIO_CTRL 0x0600
#define FNCTIO_CTRL_1 0x0700
#define GPIO_CTRL 0x0800
#define GPIO_CTRL_1 0x0900
#define CONFIG_ADIS 0x0A00
#define CONFIG_1 0x0B00
#define DEC_RATE 0x0C00
#define DEC_RATE_1 0x0D00
#define GLOB_CMD 0x0200
#define GLOB_CMD_1 0x0300
#define PAGE_ID 0x0000
#define i_am 0x4068

#define ID_PAGE_0 0x8000
#define ID_PAGE_1 0x8001
#define ID_PAGE_2 0x8002
#define ID_PAGE_3 0x0003

#define FNCTIO_CTRL_SET 0x0008
#define FNCTIO_CTRL_SET_1 0x0000

#define software_reset 0x0080
#define software_reset_1 0x0000

#define DEC_RATE_SET 0x0010
#define DEC_RATE_SET_1 0x0000

typedef struct
{
    double omega[3];
    double accel[3];
    double M[3];
    double T;
    double P;
    unsigned int cnt;
} ADIS_DATA;

typedef struct
{
    float omega[3];
    float accel[3];
    float M[3];
    float T;
    float P;
} ADIS_DATA_float;

uint16_t read_reg_ADIS(uint16_t reg);
void write_reg_ADIS(uint16_t reg, uint16_t parameter);
void reset_ADIS(void);
uint16_t init_ADIS(void);

void read_high_dword_accel_gyro_mag_ADIS(void);

void ADIS_RAW_DAT_CONVERT(ADIS_DATA *DATA);
void ADIS_DATA_CON(ADIS_DATA *DATA);
void ADIS_RAW_DAT_CONVERT_float(ADIS_DATA_float *DATA);
void ADIS_RAW_DAT_CON_float(ADIS_DATA_float *DATA);

void read_high_dword_accel_gyro_mag_ADIS_DMA(void);
void read_high_dword_all_ADIS(void);

#endif /* ADIS_H_ */
