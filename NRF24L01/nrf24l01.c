/*
 * nrf24l01.c
 *
 *  Created on: Aug 3, 2018
 *      Author: gilg
 */

#include "nrf24l01.h"

//------------------------------------------------
#define TIM_HANDLE htim14
extern TIM_HandleTypeDef TIM_HANDLE;

#define TX_ADR_WIDTH 3
#define TX_PLOAD_WIDTH 32
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xb6, 0xb4, 0x01};
uint8_t RX_ADDRESS[TX_ADR_WIDTH] = {0x27, 0x32, 0x44};
uint8_t RX_BUF[TX_PLOAD_WIDTH] = {0};
//------------------------------------------------

__STATIC_INLINE void DelayMicro(__IO uint32_t micros)
{
    (&TIM_HANDLE)->Instance->CNT = 0;
    while ((&TIM_HANDLE)->Instance->CNT < micros)
        ;
}

static uint16_t UBX_checksum(uint8_t *ptr, uint16_t len)
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

static char checksum(char *data, int size)
{
    char crc = 0;
    int i;
    for (i = 0; i < size - 1; i++)
    {
        crc ^= data[i];
    }
    return crc;
}

//-------------------------------------------------------
nrf_handle init_nrf_handle(SPI_HandleTypeDef *spi, GPIO_TypeDef *PORT_CS, uint16_t PIN_CS,
                           GPIO_TypeDef *PORT_CE, uint16_t PIN_CE, GPIO_TypeDef *PORT_IRQ,
                           uint16_t PIN_IRQ)
{
    nrf_handle nrf;
    nrf.spi = spi;
    nrf.PORT_CS = PORT_CS;
    nrf.PIN_CS = PIN_CS;
    nrf.PORT_CE = PORT_CE;
    nrf.PIN_CE = PIN_CE;
    nrf.PORT_IRQ = PORT_IRQ;
    nrf.PIN_IRQ = PIN_IRQ;
    return nrf;
}
//--------------------------------------------------

uint8_t NRF24_ReadReg(nrf_handle *nrf, uint8_t addr)
{
    uint8_t dt = 0, cmd;
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(nrf->spi, &addr, &dt, 1, 1000);
    if (addr != STATUS) // если адрес равен адрес регистра статус то и возварщаем его состояние
    {
        cmd = 0xFF;
        HAL_SPI_TransmitReceive(nrf->spi, &cmd, &dt, 1, 1000);
    }
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);

    return dt;
}

//------------------------------------------------

void NRF24_WriteReg(nrf_handle *nrf, uint8_t addr, uint8_t dt)
{
    addr |= W_REGISTER; // включим бит записи в адрес
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, &addr, 1, 1000); // отправим адрес в шину
    HAL_SPI_Transmit(nrf->spi, &dt, 1, 1000);   // отправим данные в шину
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
}

//------------------------------------------------

void NRF24_ToggleFeatures(nrf_handle *nrf)
{
    uint8_t dt[1] = {ACTIVATE};
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, dt, 1, 1000);
    DelayMicro(1);
    dt[0] = 0x73;
    HAL_SPI_Transmit(nrf->spi, dt, 1, 1000);
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
}

//-----------------------------------------------

void NRF24_FlushRX(nrf_handle *nrf)
{
    uint8_t dt[1] = {FLUSH_RX};
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, dt, 1, 1000);
    DelayMicro(1);
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
}

//------------------------------------------------

void NRF24_FlushTX(nrf_handle *nrf)
{
    uint8_t dt[1] = {FLUSH_TX};
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, dt, 1, 1000);
    DelayMicro(1);
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
}

//-----------------------------------------------

void NRF24_Read_Buf(nrf_handle *nrf, uint8_t addr, uint8_t *pBuf, uint8_t bytes)
{
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, &addr, 1, 1000);   // отправим адрес в шину
    HAL_SPI_Receive(nrf->spi, pBuf, bytes, 1000); // отправим данные в буфер
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
}

//------------------------------------------------

void NRF24_Write_Buf(nrf_handle *nrf, uint8_t addr, uint8_t *pBuf, uint8_t bytes)
{
    addr |= W_REGISTER; // включим бит записи в адрес
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, &addr, 1, 1000); // отправим адрес в шину
    DelayMicro(1);
    HAL_SPI_Transmit(nrf->spi, pBuf, bytes, 1000); // отправим данные в буфер
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
}

//------------------------------------------------

void NRF24L01_RX_Mode(nrf_handle *nrf)
{
    uint8_t regval = 0x00;
    regval = NRF24_ReadReg(nrf, CONFIG);
    // разбудим модуль и переведём его в режим приёмника, включив биты PWR_UP и PRIM_RX
    regval |= (1 << PWR_UP) | (1 << PRIM_RX);
    NRF24_WriteReg(nrf, CONFIG, regval);
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_SET);
    DelayMicro(150); // Задержка минимум 130 мкс
    // Flush buffer
    NRF24_FlushRX(nrf);
    NRF24_FlushTX(nrf);
}
//-----------------------------------------------------
void NRF24L01_TX_Mode(nrf_handle *nrf)
{
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);
    // Flush buffers
    NRF24_FlushRX(nrf);
    NRF24_FlushTX(nrf);
}
//------------------------------------------------
void NRF24_Transmit(nrf_handle *nrf, uint8_t addr, uint8_t *pBuf, uint8_t bytes)
{

    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(nrf->spi, &addr, 1, 1000);
    DelayMicro(1);
    HAL_SPI_Transmit(nrf->spi, pBuf, bytes, 1000);
    HAL_GPIO_WritePin(nrf->PORT_CS, nrf->PIN_CS, GPIO_PIN_SET);
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_SET);
}
//------------------------------------------------------------------
uint8_t NRF24L01_Send(nrf_handle *nrf, uint8_t *pBuf)
{
    uint8_t status = 0x00, regval = 0x00;
    NRF24L01_TX_Mode(nrf);
    regval = NRF24_ReadReg(nrf, CONFIG);
    /* Power up the radio in transmit mode. */
    regval |= (1 << PWR_UP);
    regval &= ~(1 << PRIM_RX);
    NRF24_WriteReg(nrf, CONFIG, regval);
    DelayMicro(150);
    NRF24_Transmit(nrf, WR_TX_PLOAD, pBuf, TX_PLOAD_WIDTH);
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_SET);
    DelayMicro(15); // minimum 10us high pulse (Page 21)
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);
    while (HAL_GPIO_ReadPin(nrf->PORT_IRQ, nrf->PIN_IRQ) == GPIO_PIN_SET)
    {
    }
    status = NRF24_ReadReg(nrf, STATUS);
    if (status & TX_DS) // tx_ds == 0x20
    {
        NRF24_WriteReg(nrf, STATUS, 0x20);
    }
    else if (status & MAX_RT)
    {

        NRF24_WriteReg(nrf, STATUS, 0x10);
        NRF24_FlushTX(nrf);
    }
    regval = NRF24_ReadReg(nrf, OBSERVE_TX);
    DelayMicro(70);

    NRF24L01_RX_Mode(nrf);
    return regval & 0xF;
}

uint8_t NRF24L01_Send_freertos_irq(nrf_handle *nrf, uint8_t *pBuf)
{
    nrf->interrupt_stat = 1;
    uint32_t signals;

    uint8_t status = 0x00, regval = 0x00;
    NRF24L01_TX_Mode(nrf);
    regval = NRF24_ReadReg(nrf, CONFIG);
    /* Power up the radio in transmit mode. */
    regval |= (1 << PWR_UP);
    regval &= ~(1 << PRIM_RX);
    NRF24_WriteReg(nrf, CONFIG, regval);
    DelayMicro(150);
    NRF24_Transmit(nrf, WR_TX_PLOAD, pBuf, TX_PLOAD_WIDTH);
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_SET);
    DelayMicro(15); // minimum 10us high pulse (Page 21)
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);

    BaseType_t ret = xTaskNotifyWait(0, (uint32_t)0xffffffff, &signals, 250);

    status = NRF24_ReadReg(nrf, STATUS);
    if (status & TX_DS) // tx_ds == 0x20
    {
        NRF24_WriteReg(nrf, STATUS, 0x20);
    }
    else if (status & MAX_RT)
    {

        NRF24_WriteReg(nrf, STATUS, 0x10);
        NRF24_FlushTX(nrf);
    }
    regval = NRF24_ReadReg(nrf, OBSERVE_TX);
    DelayMicro(70);

    NRF24L01_RX_Mode(nrf);

    return (regval & 0x0F) | ((ret == pdTRUE && signals == 0x02) ? 0x00 : 0xFF);
}
//------------------------------------------------

void NRF24L01_Receive(nrf_handle *nrf, void *RX_BUF)
{
    uint8_t status = 0x01;
    uint32_t wait_started = HAL_GetTick();
    GPIO_PinState k = HAL_GPIO_ReadPin(nrf->PORT_IRQ, nrf->PIN_IRQ);
    while (k == GPIO_PIN_SET)
    {
        if ((HAL_GetTick() - wait_started) >= 10U)
        {
            return;
        }
        k = HAL_GPIO_ReadPin(nrf->PORT_IRQ, nrf->PIN_IRQ);
    }
    status = NRF24_ReadReg(nrf, STATUS);
    if (status & 0x40)
    {
        NRF24_Read_Buf(nrf, RD_RX_PLOAD, RX_BUF, TX_PLOAD_WIDTH);
        NRF24_WriteReg(nrf, STATUS, 0x40);
    }
}

BaseType_t NRF24L01_Receive_freertos_irq(nrf_handle *nrf, void *RX_BUF, TickType_t blocktime)
{
    nrf->interrupt_stat = 0x00;
    uint32_t signals;
    //---------------------
    uint8_t status = 0x01;
    BaseType_t ret = xTaskNotifyWait(0, (uint32_t)0xffffffff, &signals, blocktime);
    status = NRF24_ReadReg(nrf, STATUS);
    if ((status & 0x40) && ret == pdPASS)
    {
        NRF24_Read_Buf(nrf, RD_RX_PLOAD, RX_BUF, TX_PLOAD_WIDTH);
        NRF24_WriteReg(nrf, STATUS, 0x40);
    }
    return ret;
}

void NRF24L01_Send_NO_AA(nrf_handle *nrf, uint8_t *pBuf)
{
    nrf->interrupt_stat = 1;
    uint32_t signals;

    uint8_t status = 0x00, regval = 0x00;
    NRF24L01_TX_Mode(nrf);
    regval = NRF24_ReadReg(nrf, CONFIG);
    /* Power up the radio in transmit mode. */
    regval |= (1 << PWR_UP);
    regval &= ~(1 << PRIM_RX);
    NRF24_WriteReg(nrf, CONFIG, regval);
    DelayMicro(150);
    NRF24_Transmit(nrf, WR_TX_PLOAD, pBuf, TX_PLOAD_WIDTH);
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_SET);
    DelayMicro(15); // minimum 10us high pulse (Page 21)
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);

    xTaskNotifyWait(0, (uint32_t)0xffffffff, &signals, 5);

    status = NRF24_ReadReg(nrf, STATUS);
    if (status & TX_DS) // tx_ds == 0x20
    {
        NRF24_WriteReg(nrf, STATUS, 0x20);
    }

    DelayMicro(70);
}

void NRF24L01_Send_N_byte_no_aa(nrf_handle *nrf, uint8_t *ptr, uint16_t n)
{
    uint16_t offset = 0;
    uint8_t buff[32] = {0x10, 0x10};
    memcpy(buff + 2, &n, 2);
    buff[4] = checksum((char *)ptr, n);
    uint16_t chunk_size = n < (TX_PLOAD_WIDTH - 5U) ? n : (TX_PLOAD_WIDTH - 5U);
    memcpy(buff + 5, ptr, chunk_size);
    NRF24L01_Send_NO_AA(nrf, buff);
    offset += chunk_size;

    while (offset < n)
    {
        memset(buff, 0, sizeof(buff));
        chunk_size = (n - offset) < TX_PLOAD_WIDTH ? (n - offset) : TX_PLOAD_WIDTH;
        memcpy(buff, ptr + offset, chunk_size);
        NRF24L01_Send_NO_AA(nrf, buff);
        offset += chunk_size;
    }
}

void NRF24L01_Send_N_byte_no_aa_2CRC(nrf_handle *nrf, uint8_t *ptr, uint16_t n)
{
    uint16_t offset = 0;
    uint8_t buff[32] = {0x10, 0x10};
    memcpy(buff + 2, &n, 2);
    //---------------------------------------------------
    uint16_t crc = UBX_checksum((uint8_t *)ptr, n);
    memcpy(buff + 4, &crc, sizeof(crc));
    //---------------------------------------------------
    uint16_t chunk_size = n < (TX_PLOAD_WIDTH - 6U) ? n : (TX_PLOAD_WIDTH - 6U);
    memcpy(buff + 6, ptr, chunk_size);
    NRF24L01_Send_NO_AA(nrf, buff);
    offset += chunk_size;

    while (offset < n)
    {
        memset(buff, 0, sizeof(buff));
        chunk_size = (n - offset) < TX_PLOAD_WIDTH ? (n - offset) : TX_PLOAD_WIDTH;
        memcpy(buff, ptr + offset, chunk_size);
        NRF24L01_Send_NO_AA(nrf, buff);
        offset += chunk_size;
    }
}
//------------------------------------------------

void NRF24_ini(nrf_handle *nrf)
{
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);

    DelayMicro(5000);
    NRF24_WriteReg(nrf, CONFIG,
                   0x02); // Set PWR_UP bit, enable CRC(1 byte) &Prim_RX:0 (Transmitter)
    DelayMicro(5000); // Дадим модулю включиться, по даташиту около 1,5 мсек, а лучше 5
    NRF24_WriteReg(nrf, EN_AA, 0x00);      // Enable Pipe1
    NRF24_WriteReg(nrf, EN_RXADDR, 0x01);  // Enable Pipe1
    NRF24_WriteReg(nrf, SETUP_AW, 0x01);   // Setup address width=3 bytes
    NRF24_WriteReg(nrf, SETUP_RETR, 0x5F); // // 1500us, 15 retrans
    NRF24_ToggleFeatures(nrf);
    NRF24_WriteReg(nrf, FEATURE, 0);
    NRF24_WriteReg(nrf, DYNPD, 0);
    NRF24_WriteReg(nrf, STATUS, 0x70);   // Reset flags for IRQ
    NRF24_WriteReg(nrf, RF_CH, 76);      // частота 2476 MHz
    NRF24_WriteReg(nrf, RF_SETUP, 0x06); // TX_PWR:0dBm, Datarate:1Mbps 0x27
    NRF24_Write_Buf(nrf, TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_Write_Buf(nrf, RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_WriteReg(nrf, RX_PW_P0, TX_PLOAD_WIDTH); // Number of bytes in RX payload in data pipe 1
    NRF24L01_RX_Mode(nrf);
}

void NRF24_ini_TX(nrf_handle *nrf)
{
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);
    DelayMicro(5000);
    NRF24_WriteReg(nrf, CONFIG,
                   0x0a); // Set PWR_UP bit, enable CRC(1 byte) &Prim_RX:0 (Transmitter)
    DelayMicro(5000);
    NRF24_WriteReg(nrf, EN_AA, 0x01);      // Enable Pipe0
    NRF24_WriteReg(nrf, EN_RXADDR, 0x01);  // Enable Pipe0
    NRF24_WriteReg(nrf, SETUP_AW, 0x01);   // Setup address width=3 bytes
    NRF24_WriteReg(nrf, SETUP_RETR, 0x5F); // // 1500us, 15 retrans
    NRF24_ToggleFeatures(nrf);
    NRF24_WriteReg(nrf, FEATURE, 0);
    NRF24_WriteReg(nrf, DYNPD, 0);
    NRF24_WriteReg(nrf, STATUS, 0x70);   // Reset flags for IRQ
    NRF24_WriteReg(nrf, RF_CH, 76);      // 2476 MHz
    NRF24_WriteReg(nrf, RF_SETUP, 0x06); // TX_PWR:0dBm, Datarate:1Mbps
    NRF24_Write_Buf(nrf, TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_Write_Buf(nrf, RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_WriteReg(nrf, RX_PW_P0, TX_PLOAD_WIDTH); // Number of bytes in RX payload in data pipe 0
    NRF24L01_RX_Mode(nrf);
}

void NRF24_ini_RX(nrf_handle *nrf)
{
    HAL_GPIO_WritePin(nrf->PORT_CE, nrf->PIN_CE, GPIO_PIN_RESET);
    DelayMicro(5000);
    NRF24_WriteReg(nrf, CONFIG,
                   0x0a); // Set PWR_UP bit, enable CRC(1 byte) &Prim_RX:0 (Transmitter)
    DelayMicro(5000);
    NRF24_WriteReg(nrf, EN_AA, 0x02);      // Enable Pipe1
    NRF24_WriteReg(nrf, EN_RXADDR, 0x02);  // Enable Pipe1
    NRF24_WriteReg(nrf, SETUP_AW, 0x01);   // Setup address width=3 bytes
    NRF24_WriteReg(nrf, SETUP_RETR, 0x5F); // // 1500us, 15 retrans
    NRF24_ToggleFeatures(nrf);
    NRF24_WriteReg(nrf, FEATURE, 0);
    NRF24_WriteReg(nrf, DYNPD, 0);
    NRF24_WriteReg(nrf, STATUS, 0x70);   // Reset flags for IRQ
    NRF24_WriteReg(nrf, RF_CH, 76);      // 2476 MHz
    NRF24_WriteReg(nrf, RF_SETUP, 0x06); // TX_PWR:0dBm, Datarate:1Mbps
    NRF24_Write_Buf(nrf, TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_Write_Buf(nrf, RX_ADDR_P1, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_WriteReg(nrf, RX_PW_P1, TX_PLOAD_WIDTH); // Number of bytes in RX payload in data pipe 1
    NRF24L01_RX_Mode(nrf);
}
