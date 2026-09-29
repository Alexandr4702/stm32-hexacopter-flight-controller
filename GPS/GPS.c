/*
 * GPS.c
 *
 *  Created on: Feb 16, 2018
 *      Author: root
 */

#include "GPS.h"

#define freq 10
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

#define trans(a) ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8)

uint16_t Table_CRC[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108, 0x9129, 0xA14A, 0xB16B,
    0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738,
    0xF7DF, 0xE7FE, 0xD79D, 0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5, 0x4AD4, 0x7AB7, 0x6A96,
    0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
    0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB,
    0xD10C, 0xC12D, 0xF14E, 0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1, 0x1290, 0x22F3, 0x32D2,
    0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
    0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827,
    0x18C0, 0x08E1, 0x3882, 0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E, 0xED0F, 0xDD6C, 0xCD4D,
    0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
    0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};
// crc – CS battery value (0 – at the first call)
// c – data byte
void add_CRC(uint16_t *crc, uint8_t c)
{
    uint16_t cval = ((*crc >> 8) ^ c) & 0xff;
    *crc = (*crc << 8) ^ Table_CRC[cval];
}

uint16_t calc_CRC(uint8_t *data, uint16_t size_data)
{
    uint16_t crc = 0;
    for (uint16_t i = 0; i < size_data; i++)
    {
        uint16_t cval = ((crc >> 8) ^ (*(data + i))) & 0xff;
        crc = (crc << 8) ^ Table_CRC[cval];
    }
    return crc;
}

char checksum(char *data, int size)
{
    char crc = 0;
    int i;
    for (i = 0; i < size - 1; i++)
    {
        crc ^= data[i];
    }
    return crc;
}

void GPS_Write_Reg(uint8_t reg, uint8_t *data, uint8_t size_data, uint8_t crc_status)
{
    if (crc_status == 1)
    {
        uint8_t buff[size_data + 8];
        buff[0] = 0x10, buff[1] = reg;
        memcpy((char *)&buff[2], data, size_data);
        buff[size_data + 2] = 0x10, buff[size_data + 3] = 0xff;
        uint16_t crc = calc_CRC((uint8_t *)&buff[1], size_data + 1);
        buff[size_data + 4] = (uint8_t)((crc & 0x00FF)),
                         buff[size_data + 5] =
                             (uint8_t)(((crc) & 0xFF00) >> 8); // срезает старшие байты
        buff[size_data + 6] = 0x10, buff[size_data + 7] = 0x03;
        HAL_UART_Transmit(&huart3, (uint8_t *)&buff, size_data + 8, 0xff);
    }
    else
    {
        uint8_t buff[size_data + 4];
        buff[0] = 0x10, buff[1] = reg;
        memcpy((char *)&buff[2], data, size_data);
        buff[size_data + 2] = 0x10, buff[size_data + 3] = 0x03;
        HAL_UART_Transmit(&huart3, (uint8_t *)&buff, size_data + 4, 0xff);
    }
}

void GPS_Restart(uint8_t temp) // if temp=0 then cold restart,else warm
{
    uint8_t data[6] = {0x00, 0x01, 0x21, 0x01, 0x00};
    data[5] = temp;
    GPS_Write_Reg(0x01, (uint8_t *)&data, 6, 1);
    HAL_Delay(500);
}

uint8_t GPS_Check_connection(void)
{
    uint8_t buff[8];
    // uint8_t data[8]={0x10,0x54,0x10,0xFF,0x71,0x1A,0x10,0x03};
    uint8_t data[8] = {0x10, 0x54, 0x10, 0x03};
    GPS_Write_Reg(0x26, NULL, 0, 0);
    if (HAL_UART_Receive(&huart3, (uint8_t *)buff, 5, 0xFFFF) != HAL_OK)
        return 0;
    else
    {
        if (memcmp(buff + 1, data, 4) == 0)
            return 1;
    }

    return 0;
}

void GPS_set_baud_rate(uint32_t baud_rate)
{
    uint8_t str[6] = {0x00, 0x00, 0x84, 0x03, 0x00, 0x04};
    memcpy((void *)(str + 1), (void *)&baud_rate, 4);
    GPS_Write_Reg(0x0B, str, 6, 1);
    huart3.Init.BaudRate = 230400;
    HAL_UART_Init(&huart3);
    HAL_Delay(500);
}

uint8_t GPS_Init(void)
{

    HAL_NVIC_EnableIRQ(USART3_IRQn); // для инициализации gps необходимы прерывания по приему
    GPS_Restart(0);

    if (GPS_Check_connection() == 0)
        return 0;

    uint8_t str1[2] = {0x02, 0x00};
    GPS_Write_Reg(0xb2, str1, 2, 0); // включение CRC (здесь CRC еще нет )
    GPS_Write_Reg(0x0E, NULL, 0, 1); // Отмена всех пакетов

#if freq == 10
    uint8_t str2[2] = {0x02, 0x0a};
#else
    uint8_t str2[2] = {0x02, 0x01};
#endif

    GPS_Write_Reg(0xd7, (uint8_t *)&str2, 2, 1); // темп решения навигационной задачи

    uint8_t str4 = 0x01;
    GPS_Write_Reg(0x27, (uint8_t *)&str4, 1, 1); // выдача решение

    return 1;
}

uint32_t pars_N8IS(uint8_t *ptr, uint16_t cnt_bytes, GPS_DATA *GPS_out)
{
    static uint8_t flag_dec = 0;
    static uint8_t flag_mess = 0;

    uint8_t buff[200];
    static uint16_t crc = 0;

    static uint32_t mess_cnt = 0;

    static uint16_t length_read = 0;

    for (uint16_t i = 0; i < cnt_bytes; i++)
    {
        if (ptr[i] == 0x10 && flag_dec == 0 && flag_mess == 0)
        {
            flag_dec = 1;
            continue;
        }
        if (flag_dec == 1 && flag_mess == 0)
        {
            switch (ptr[i])
            {
            case 0x10:
                flag_dec = 0;
                continue;
            case 0x88:
                flag_mess = 1;
                flag_dec = 0;
                continue;
            case 0xff:
                flag_dec = 0;
                continue;
            case 0x03:
                flag_dec = 0;
                continue;
            default:
                flag_mess = 0;
                flag_dec = 0;
                continue;
            }
        }

        if (flag_mess == 1)
        {
            if (ptr[i] == 0x10 && flag_dec == 0)
            {
                flag_dec = 1;
                add_CRC(&crc, ptr[i]);
                buff[length_read] = ptr[i];
                length_read++;
                continue;
            }
            if (flag_dec == 1)
            {
                switch (ptr[i])
                {
                case 0x10:
                    flag_dec = 0;
                    add_CRC(&crc, ptr[i]);
                    continue;
                case 0xff:
                    flag_dec = 0;
                    continue;
                case 0x03:
                    flag_dec = 0;
                    flag_mess = 0;
                    GPS_out->latitude = *((double *)(&buff[0]));
                    GPS_out->longitude = *((double *)(&buff[8]));
                    GPS_out->altitude = *((double *)(&buff[16]));
                    GPS_out->velocity = *((double *)(&buff[24]));
                    GPS_out->latitude_vel = *((double *)(&buff[40]));
                    GPS_out->longitude_vel = *((double *)(&buff[48]));
                    GPS_out->altitude_vel = *((double *)(&buff[56]));
                    GPS_out->solution_state = *((uint8_t *)(&buff[68]));
                    GPS_out->solution_state_2D = (GPS_out->solution_state & 0x02) >> 1;
                    GPS_out->solution_state_3D =
                        (GPS_out->solution_state & 0x01) & (~GPS_out->solution_state_2D);
                    GPS_out->_mess_ready = 1;
                    mess_cnt++;
                    return GPS_out->_mess_ready;
                }
            }
            add_CRC(&crc, ptr[i]);
            buff[length_read] = ptr[i];
        }
    }

    return GPS_out->_mess_ready;
}
