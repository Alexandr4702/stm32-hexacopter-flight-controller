/*
 * ADIS.c
 *
 *  Created on: Oct 30, 2017
 *      Author: root
 */

#include "ADIS.h"

#include <math.h>

#define delta(x1, x2) x1 > x2 ? (x1 - x2) : (x2 - x1)

extern SPI_HandleTypeDef hspi1;
static const uint16_t spi_transmit[21] = {
    DUMMY_WORD // 0
    ,
    who_am_i // 1
    ,
    SYS_E_FLAG // 2
    ,
    TEMP_OUT // 3
    ,
    X_GYRO_OUT // 4
    ,
    Y_GYRO_OUT // 5
    ,
    Z_GYRO_OUT // 6
    ,
    X_ACCL_OUT // 7
    ,
    Y_ACCL_OUT // 8
    ,
    Z_ACCL_OUT // 9
    ,
    X_MAGN_OUT // 10
    ,
    Y_MAGN_OUT // 11
    ,
    Z_MAGN_OUT // 12
    ,
    BAROM_OUT // 13
    ,
    BAROM_LOW // 14
    ,
    X_GYRO_LOW // 15
    ,
    Y_GYRO_LOW // 16
    ,
    Z_GYRO_LOW // 17
    ,
    X_ACCL_LOW // 18
    ,
    Y_ACCL_LOW // 19
    ,
    Z_ACCL_LOW // 20
};

static int16_t RAW_DATA[16];

void ADIS_RAW_DAT_CONVERT(ADIS_DATA *DATA)
{
    static int16_t DATA_P_pr;
    static uint32_t cnt = 0;

    DATA->T = ((double)RAW_DATA[0 + 3]) * ((double)0.00565) + 25;
    DATA->omega[0] = ((double)RAW_DATA[1 + 3]) * ((double)0.02) * M_PI / 180;
    DATA->omega[1] = ((double)RAW_DATA[3 + 3]) * ((double)0.02) * M_PI / 180;
    DATA->omega[2] = ((double)RAW_DATA[2 + 3]) * ((double)0.02) * M_PI / 180 * (-1);
    DATA->accel[0] = ((double)RAW_DATA[4 + 3]) * ((double)0.0008);
    DATA->accel[1] = ((double)RAW_DATA[6 + 3]) * ((double)0.0008);
    DATA->accel[2] = ((double)RAW_DATA[5 + 3]) * ((double)0.0008) * (-1);
    DATA->M[0] = ((double)RAW_DATA[7 + 3]) * ((double)1.0E-8);
    DATA->M[1] = ((double)RAW_DATA[8 + 3]) * ((double)1.0E-8);
    DATA->M[2] = ((double)RAW_DATA[9 + 3]) * ((double)1.0E-8) * (-1);

    DATA->P = (((RAW_DATA[10 + 3] == 0x0000) || (delta(RAW_DATA[10 + 3], DATA_P_pr) > 1000)) &&
               (cnt != 0))
                  ? DATA->P
                  : (double)RAW_DATA[10 + 3] * (double)4;

    DATA_P_pr = RAW_DATA[10 + 3];
    cnt++;
}

void ADIS_DATA_CON(ADIS_DATA *DATA)
{
    DATA->T = ((double)RAW_DATA[2]) * ((double)0.00565) + 25;
    DATA->omega[0] = ((double)RAW_DATA[3]) * ((double)0.02);
    DATA->omega[1] = ((double)RAW_DATA[4]) * ((double)0.02);
    DATA->omega[2] = ((double)RAW_DATA[5]) * ((double)0.02);
    DATA->accel[0] = ((double)RAW_DATA[6]) * ((double)0.0008);
    DATA->accel[1] = ((double)RAW_DATA[7]) * ((double)0.0008);
    DATA->accel[2] = ((double)RAW_DATA[8]) * ((double)0.0008);
    DATA->M[0] = ((double)RAW_DATA[9]) * ((double)1.0E-4);
    DATA->M[1] = ((double)RAW_DATA[10]) * ((double)1.0E-4);
    DATA->M[2] = ((double)RAW_DATA[11]) * ((double)1.0E-4);
    DATA->P = ((double)RAW_DATA[12]) * ((double)0.00004);
}

void ADIS_RAW_DAT_CON_float(ADIS_DATA_float *DATA)
{
    DATA->omega[0] = ((float)RAW_DATA[1]) * ((float)0.02);
    DATA->omega[1] = ((float)RAW_DATA[2]) * ((float)0.02);
    DATA->omega[2] = ((float)RAW_DATA[3]) * ((float)0.02);
    DATA->accel[0] = ((float)RAW_DATA[4]) * ((float)0.0008);
    DATA->accel[1] = ((float)RAW_DATA[5]) * ((float)0.0008);
    DATA->accel[2] = ((float)RAW_DATA[6]) * ((float)0.0008);
}

void ADIS_RAW_DAT_CONVERT_float(ADIS_DATA_float *DATA)
{

    DATA->T = ((float)RAW_DATA[0]) * ((float)0.00565) + 25;
    DATA->omega[0] = ((float)RAW_DATA[1]) * ((float)0.02) * M_PI / 180;
    DATA->omega[1] = ((float)RAW_DATA[3]) * ((float)0.02) * M_PI / 180;
    DATA->omega[2] = ((float)RAW_DATA[2]) * ((float)0.02) * M_PI / 180 * (-1);
    DATA->accel[0] = ((float)RAW_DATA[4]) * ((float)0.0008);
    DATA->accel[1] = ((float)RAW_DATA[6]) * ((float)0.0008);
    DATA->accel[2] = ((float)RAW_DATA[5]) * ((float)0.0008) * (-1);
    DATA->M[0] = ((float)RAW_DATA[7]) * ((float)1.0E-8);
    DATA->M[1] = ((float)RAW_DATA[9]) * ((float)1.0E-8);
    DATA->M[2] = ((float)RAW_DATA[8]) * ((float)1.0E-8) * (-1);
    DATA->P = ((float)RAW_DATA[10]) * ((float)0.00004);
}

uint16_t read_reg_ADIS(uint16_t reg)
{
    CS_ON;
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg, 1, 0xFFF);
    HAL_SPI_Receive(&hspi1, (uint8_t *)&reg, 1, 0xFFF);
    CS_OFF;
    return reg;
}

void write_reg_ADIS(uint16_t reg, uint16_t parameter)
{
    reg |= READWRITE_CMD | parameter;
    CS_ON;
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg, 1, 0xFFF);
    CS_OFF;
}
static void write_ddword_ADIS(uint16_t reg, uint16_t parameter)
{
    uint16_t reg_1 = (reg + 0x0100) | READWRITE_CMD | (parameter >> 8);
    reg |= 0x8000 | (parameter & 0x00FF);
    CS_ON;
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg, 1, 0xFFF);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg_1, 1, 0xFFF);
    CS_OFF;
}

uint16_t init_ADIS(void)
{
    reset_ADIS();
    uint16_t I_AM = read_reg_ADIS(who_am_i);
    if (I_AM == 0x4068)
    {
        write_reg_ADIS(PAGE_ID, ID_PAGE_3);
        write_ddword_ADIS(FNCTIO_CTRL, 0x0008);
        write_ddword_ADIS(DEC_RATE, 0x0018);
        write_reg_ADIS(PAGE_ID, ID_PAGE_0);
        return 1;
    }
    else
        return 0;
}

void reset_ADIS(void)
{
    write_reg_ADIS(PAGE_ID, ID_PAGE_3);
    write_ddword_ADIS(GLOB_CMD, 0x0080);
    HAL_Delay(120);
    write_reg_ADIS(PAGE_ID, ID_PAGE_0);
}

void read_high_dword_accel_gyro_mag_ADIS(void)
{
    CS_ON;
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&spi_transmit[3], (uint8_t *)RAW_DATA, 13, 0xfff);
    CS_OFF;
}

void read_high_dword_all_ADIS(void)
{
    RAW_DATA[2] = read_reg_ADIS(spi_transmit[3]);
    RAW_DATA[3] = read_reg_ADIS(spi_transmit[4]);
    RAW_DATA[4] = read_reg_ADIS(spi_transmit[5]);
    RAW_DATA[5] = read_reg_ADIS(spi_transmit[6]);
    RAW_DATA[6] = read_reg_ADIS(spi_transmit[7]);
    RAW_DATA[7] = read_reg_ADIS(spi_transmit[8]);
    RAW_DATA[8] = read_reg_ADIS(spi_transmit[9]);
    RAW_DATA[9] = read_reg_ADIS(spi_transmit[10]);
    RAW_DATA[10] = read_reg_ADIS(spi_transmit[11]);
    RAW_DATA[11] = read_reg_ADIS(spi_transmit[12]);
    RAW_DATA[12] = read_reg_ADIS(spi_transmit[13]);
}

void read_high_dword_accel_gyro_mag_ADIS_DMA(void)
{
    CS_ON;
    HAL_SPI_TransmitReceive_DMA(&hspi1, (uint8_t *)&spi_transmit[3], (uint8_t *)&RAW_DATA, 11);
}
