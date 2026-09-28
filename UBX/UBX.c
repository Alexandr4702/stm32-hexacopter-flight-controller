/*
 * UBX.c
 *
 *  Created on: Aug 17, 2018
 *      Author: gilg
 */

#include "UBX.h"
#include "stm32f7xx_hal.h"
#include <string.h>

// extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef UART_UBX;

uint8_t recived_mes = 0;

uint16_t UBX_checksum(uint8_t *ptr, uint16_t len)
{
    uint8_t CK_A = 0, CK_B = 0;
    for (uint16_t I = 0; I < len; I++)
    {
        CK_A = CK_A + ptr[I];
        CK_B = CK_B + CK_A;
    }
    uint16_t UBX_CRC = (CK_B << 8) | CK_A;
    return UBX_CRC;
}

void UBX_write_reg(uint8_t class, uint8_t ID, uint8_t *data, int16_t len_payload)
{
    uint8_t output_message[len_payload + 8];
    output_message[0] = 0xb5;
    output_message[1] = 0x62;
    output_message[2] = class;
    output_message[3] = ID;
    memcpy(&output_message[4], &len_payload, 2);
    memcpy(&output_message[6], data, len_payload);
    uint16_t crc = UBX_checksum(output_message + 2, len_payload + 1 + 1 + 2);
    memcpy(&output_message[len_payload + 6], &crc, 2);
    HAL_UART_Transmit(&UART_UBX, output_message, len_payload + 8, 0xff);
}

void UBX_read_reg(uint8_t class, uint8_t ID, uint8_t *data, int16_t len_payload)
{
    int16_t len = 0;
    uint8_t tx_mess[8] = {0xb5, 0x62, class, ID};

    memcpy(&tx_mess[4], &len, 2);
    uint16_t crc = UBX_checksum(tx_mess + 2, len + 1 + 1 + 2);
    memcpy(&tx_mess[len + 6], &crc, 2);
    HAL_UART_Transmit(&UART_UBX, tx_mess, 8, 0xff);
    if (HAL_UART_Receive(&UART_UBX, data, 8 + len_payload, 0xffff) == HAL_OK)
    {
        //		HAL_UART_Transmit(&huart1,data,8+len_payload,0xffff);
    }
}

void UBX_init(void)
{
    /*
    uint8_t RMC_Off[] = {0xF0,0x04,0x00};
    uint8_t VTG_Off[] = {0xF0,0x05,0x00};
    uint8_t GSA_Off[] = {0xF0,0x02,0x00};
    uint8_t GSV_Off[] = {0xF0,0x03,0x00};
    uint8_t GLL_Off[] = {0xF0,0x01,0x00};
    uint8_t GGA_Off[] = {0xF0,0x00,0x00};

    UBX_write_reg(0x06,0x01,RMC_Off,3);
    UBX_write_reg(0x06,0x01,VTG_Off,3);
    UBX_write_reg(0x06,0x01,GGA_Off,3);
    UBX_write_reg(0x06,0x01,GSA_Off,3);
    UBX_write_reg(0x06,0x01,GSV_Off,3);
    UBX_write_reg(0x06,0x01,GLL_Off,3);
    */
    uint8_t NAV_POSLLH_on[] = {0x01, 0x02, 0x01};
    uint8_t NAV_VELNED_on[] = {0x01, 0x12, 0x01};
    uint8_t NAV_STATUS_on[] = {0x01, 0x03, 0x01};
    uint8_t NAV_DOP_on[] = {0x01, 0x04, 0x01};

    uint8_t CFG_RATE[] = {0x64, 0x00, 0x01, 0x00, 0x01, 0x00};

    uint8_t CFG_PRT[] = {0x01,                   // port id
                         0x00,                   // reserved1
                         0x00, 0x00,             // txReady
                         0xC0, 0x08, 0x00, 0x00, // mode
                         0x00, 0xc2, 0x01, 0x00, // baudRate| 115200 00c20100|9600 8025
                         0x07, 0x00,             // inProtoMask
                         0x01, 0x00,             // outProtoMask| only ubx
                         0x00, 0x00,             // flags
                         0x00, 0x00};            // reservd
    HAL_Delay(10);
    UBX_write_reg(0x06, 0x00, CFG_PRT, 20);
    UART_UBX.Init.BaudRate = 115200;
    HAL_UART_Init(&UART_UBX);
    HAL_Delay(200);

    UBX_write_reg(0x06, 0x01, NAV_POSLLH_on, 3);
    UBX_write_reg(0x06, 0x01, NAV_VELNED_on, 3);
    UBX_write_reg(0x06, 0x01, NAV_STATUS_on, 3);
    UBX_write_reg(0x06, 0x01, NAV_DOP_on, 3);

    UBX_write_reg(0x06, 0x08, CFG_RATE, 6);
}
/*
void ONE_PACKET_RECIVE(void)
{
    static uint8_t init_mes=0;
    static uint8_t reciving_mes=0;
    int16_t len_t;
    if(recived_mes==1)return;
    if(reciving_mes)
    {
        recived_mes=1;
        reciving_mes=0;
        return;
    }
    switch(init_mes)
    {
    case 0:
        if(rx_buff[0]==0xb5){init_mes++;HAL_UART_Receive_DMA(&UART_UBX,rx_buff,1);}
        else init_mes=0;
        break;
    case 1:
        if(rx_buff[0]==0x62){init_mes++;HAL_UART_Receive_DMA(&UART_UBX,rx_buff,4);}
        else init_mes=0;
        break;
    case 2:
        memcpy(&len_t,rx_buff+2,2);
        HAL_UART_Receive_DMA(&UART_UBX,rx_buff+4,(uint16_t)len_t+2);
        init_mes=0;
        reciving_mes=1;
        break;
    default:
        init_mes=0;
        reciving_mes=0;
        break;

    }
}
*/
NAV_POSLLH_ handler_NAV_POSLLH(uint8_t *ptr)
{
    NAV_POSLLH_ ret;
    ret.longitude = ((double)*((int32_t *)(ptr + 4))) * 1e-7;
    ret.latitude = ((double)*((int32_t *)(ptr + 8))) * 1e-7;
    ret.Height_above_sea = (double)*((int32_t *)(ptr + 16));
    ret.horizontal_accuracy = (double)*((uint32_t *)(ptr + 20));
    ret.vertical_accuracy = (double)*((uint32_t *)(ptr + 24));
    return ret;
}

NAV_STATUS_ handler_NAV_STATUS(uint8_t *ptr)
{
    NAV_STATUS_ ret;
    ret.gpsFix = *(ptr + 4);
    ret.time_since_restart_ms = *((uint32_t *)(ptr + 12));
    return ret;
}
NAV_DOP_ handler_NAV_DOP_(uint8_t *ptr)
{
    NAV_DOP_ ret;
    ret.Vdop = ((double)*((uint16_t *)(ptr + 10))) * 0.01;
    ret.Hdop = ((double)*((uint16_t *)(ptr + 12))) * 0.01;
    return ret;
}

NAV_VELNED_ handler_NAV_VELNED_(uint8_t *ptr)
{
    NAV_VELNED_ ret;
    ret.velN = ((double)*((int32_t *)(ptr + 4)));
    ret.velE = ((double)*((int32_t *)(ptr + 8)));
    ret.velD = ((double)*((int32_t *)(ptr + 12)));
    ret.speed = ((double)*((uint32_t *)(ptr + 16)));
    ret.gspeed = ((double)*((uint32_t *)(ptr + 20)));
    ret.heading = ((double)*((int32_t *)(ptr + 24))) * 1e-5;
    ret.sAcc = ((double)*((uint32_t *)(ptr + 28)));
    ret.cAcc = ((double)*((uint32_t *)(ptr + 32))) * 1e-5;
    return ret;
}
void pars(uint8_t *ptr, uint16_t cnt_bytes, navigation_mes *mes)
{
    static uint8_t rx_buf[500];

    static uint8_t ident_cnt = 0;
    static uint8_t part_message = 0x00;
    static uint8_t class__message = 0x00;
    static uint8_t ID_message = 0x00; // TODO мб объеденить с классом;
    static uint16_t length_message = 0x00;
    static uint16_t checksum_message = 0x00;
    static uint16_t checksum_calc = 0x00;

    static uint16_t need_read = 0;

    static uint16_t error_cnt = 0x00;
    static uint16_t mess_cnt = 0x00;

    for (uint16_t i = 0; i < cnt_bytes; i++)
    {

        if ((ptr[i] == 0xb5) && (ident_cnt == 0))
        {
            ident_cnt = 1;
            continue;
        }
        if (ident_cnt == 1)
        {
            ident_cnt = (ptr[i] == 0x62) ? 2 : 0;
            part_message = class_;
            continue;
        } // todo мы стали классом, но еще не факт что 62!! (ошибка не на что не влияет ))
        if (ident_cnt == 2)
        {
            switch (part_message)
            {
            case class_:
                class__message = ptr[i];
                part_message = ID;
                break;
            case ID:
                ID_message = ptr[i];
                part_message = lentgh;
                need_read = 2;
                break;
            case lentgh:
                length_message |= (need_read == 2) ? ptr[i] : ptr[i] << 8; // слева направо
                need_read--;
                if (need_read == 0)
                {
                    part_message = payload;
                    need_read = length_message;
                    if (length_message > 500)
                    {
                        part_message = 0x00;
                        ident_cnt = 0x00;
                        length_message = 0x00;
                        checksum_message = 0x00;
                        error_cnt++;
                        mes->_mess_ready |= 16;
                    } // обнуление переменных
                }
                break;
            case payload:
                rx_buf[4 + length_message - need_read] = ptr[i];
                need_read--;
                if (need_read == 0)
                {
                    part_message = checksum_;
                    need_read = 2;
                }
                break;
            case checksum_:
                checksum_message |= (need_read == 2) ? ptr[i] : ptr[i] << 8;
                need_read--;
                if (need_read == 0)
                {
                    rx_buf[0] = class__message;
                    rx_buf[1] = ID_message;
                    rx_buf[2] = 0x00ff & length_message;
                    rx_buf[3] = length_message >> 8; // заполнение для счета контрольной суммы
                    //______________________________________________________________________________________________________________________________________
                    checksum_calc = UBX_checksum(rx_buf, 4 + length_message);
                    if (checksum_message == checksum_calc)
                    {
                        // printf("message %hu errors %hu ID %hhx
                        // \r\n",mess_cnt,error_cnt,ID_message);
                        switch (ID_message)
                        {
                        case NAV_POSLLH:
                            mes->_NAV_POSLLH_ = handler_NAV_POSLLH(rx_buf + 4);
                            mes->_mess_ready |= 1;
                            break;
                        case NAV_STATUS:
                            mes->_NAV_STATUS_ = handler_NAV_STATUS(rx_buf + 4);
                            mes->_mess_ready |= 2;
                            break;
                        case NAV_DOP:
                            mes->_NAV_DOP_ = handler_NAV_DOP_(rx_buf + 4);
                            mes->_mess_ready |= 4;
                            break;
                        case NAV_VELNED:
                            mes->_NAV_VELNED_ = handler_NAV_VELNED_(rx_buf + 4);
                            mes->_mess_ready |= 8;
                            break;
                        }
                        mess_cnt++;
                    }
                    else
                    {
                        error_cnt++;
                        mes->_mess_ready |= 16;
                    }
                    //_____________________________________________________________________________________________________________________________________________
                    part_message = 0x00;
                    ident_cnt = 0x00;
                    length_message = 0x00;
                    checksum_message = 0x00; // обнуление переменных
                }
                break;
            }
        }
    }
}
