/*
 * GPS.h
 *
 *  Created on: Feb 16, 2018
 *      Author: root
 */

#ifndef GPS_H_
#define GPS_H_

#include "stdio.h"
#include "string.h"
#include "stm32f7xx_hal.h"

typedef struct
{
    double latitude;
    double longitude;
    double altitude;
    float velocity;
    long double time;
    int16_t week;
    double latitude_vel;
    double longitude_vel;
    double altitude_vel;
    uint8_t solution_state;
    uint8_t solution_state_2D;
    uint8_t solution_state_3D;
    uint16_t _mess_ready;
} GPS_DATA;

typedef struct
{
    double latitude;
    double longitude;
    double altitude;
    double latitude_vel;
    double longitude_vel;
    double altitude_vel;
    uint8_t solution_state_3D;
} GPS_DATA_FOR_TRANSMIT;

typedef struct
{
    uint8_t recived_message : 1;
    uint8_t recived_message_pack;
    uint8_t status_message;
    volatile uint8_t flag_dec;
    uint8_t flag_message;
    volatile uint8_t message_buff[400];
} GPS_FLAGS_BIN;

typedef struct
{
    uint16_t crc_message;
    uint8_t crc_flag : 1;
    uint16_t crc_recived;
    uint8_t crc_counter;
} CRC_DATA;

enum
{
    Vector_of_condition = 0x88,
    end_of_message = 0x03,
    start_CRC = 0xff,
    dec = 0x10
} service_message;

uint8_t GPS_Check_connection(void);
void GPS_Restart(uint8_t temp); // if temp=0 then cold restart,else warm
uint8_t GPS_Init(void);
uint32_t pars_N8IS(uint8_t *ptr, uint16_t cnt_bytes, GPS_DATA *GPS_out);
char checksum(char *data, int size);
void ONE_PACKET_RECIVE(void);

#endif /* GPS_H_ */
