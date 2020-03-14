/*
 * nrf24l01.h
 *
 *  Created on: Aug 3, 2018
 *      Author: gilg
 */

#ifndef NRF24L01_H_
#define NRF24L01_H_

#include <string.h>
#include "main.h"
#include "cmsis_os.h"

//------------------------------------------------
/*
#define nrf_spi_handle	hspi1

#define CS_GPIO_PORT GPIOB
#define CS_PIN GPIO_PIN_12

#define CE_GPIO_PORT GPIOB
#define CE_PIN GPIO_PIN_13

#define IRQ_GPIO_PORT GPIOB
#define IRQ_PIN GPIO_PIN_14

//#define LED_GPIO_PORT GPIOC//LED
//#define LED_PIN GPIO_PIN_13
#define IRQ HAL_GPIO_ReadPin(IRQ_GPIO_PORT, IRQ_PIN)
#define LED_ON HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET)
#define LED_OFF HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_SET)
#define LED_TGL HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN)
#define CS_ON HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_RESET)
#define CS_OFF HAL_GPIO_WritePin(CS_GPIO_PORT, CS_PIN, GPIO_PIN_SET)
#define CE_RESET HAL_GPIO_WritePin(CE_GPIO_PORT, CE_PIN, GPIO_PIN_RESET)
#define CE_SET HAL_GPIO_WritePin(CE_GPIO_PORT, CE_PIN, GPIO_PIN_SET)
*/
//------------------------------------------------

#define ACTIVATE 0x50 //
#define RD_RX_PLOAD 0x61 // Define RX payload register address
#define WR_TX_PLOAD 0xA0 // Define TX payload register address
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2

//------------------------------------------------

#define CONFIG 0x00 //'Config' register address
#define EN_AA 0x01 //'Enable Auto Acknowledgment' register address
#define EN_RXADDR 0x02 //'Enabled RX addresses' register address
#define SETUP_AW 0x03 //'Setup address width' register address
#define SETUP_RETR 0x04 //'Setup Auto. Retrans' register address
#define RF_CH 0x05 //'RF channel' register address
#define RF_SETUP 0x06 //'RF setup' register address
#define STATUS 0x07 //'Status' register address
#define OBSERVE_TX 0x08 //'Transmit observe' register

#define RX_ADDR_P0 0x0A //'RX address pipe0' register address
#define RX_ADDR_P1 0x0B //'RX address pipe1' register address
#define RX_ADDR_P2 0x0C //'RX address pipe0' register address
#define RX_ADDR_P3 0x0D //'RX address pipe1' register address
#define RX_ADDR_P4 0x0E //'RX address pipe0' register address
#define RX_ADDR_P5 0x0F //'RX address pipe1' register address

#define TX_ADDR 0x10 //'TX address' register address

#define RX_PW_P0 0x11 //'RX payload width, pipe0' register address
#define RX_PW_P1 0x12 //'RX payload width, pipe1' register address
#define RX_PW_P2 0x13 //'RX payload width, pipe0' register address
#define RX_PW_P3 0x14 //'RX payload width, pipe1' register address
#define RX_PW_P4 0x15 //'RX payload width, pipe0' register address
#define RX_PW_P5 0x16 //'RX payload width, pipe1' register address


#define FIFO_STATUS 0x17 //'FIFO Status Register' register address
#define DYNPD 0x1C
#define FEATURE 0x1D

//------------------------------------------------

#define PRIM_RX 0x00 //RX/TX control (1: PRX, 0: PTX)
#define PWR_UP 0x01 //1: POWER UP, 0:POWER DOWN
#define RX_DR 0x40 //Data Ready RX FIFO interrupt
#define TX_DS 0x20 //Data Sent TX FIFO interrupt
#define MAX_RT 0x10 //Maximum number of TX retransmits interrupt

//------------------------------------------------

#define W_REGISTER 0x20 //запись в регистр
//----------------------------------------------------------

typedef struct
{
	SPI_HandleTypeDef*	spi;
	GPIO_TypeDef*		PORT_CE;
	uint16_t			PIN_CE;
	GPIO_TypeDef*		PORT_CS;
	uint16_t			PIN_CS;
	GPIO_TypeDef*		PORT_IRQ;
	uint16_t			PIN_IRQ;
	__IO uint8_t interrupt_stat;
}nrf_handle;

//------------------------------------------------
nrf_handle init_nrf_handle(SPI_HandleTypeDef* spi,
		GPIO_TypeDef* PORT_CS ,uint16_t PIN_CS,
		GPIO_TypeDef* PORT_CE ,uint16_t PIN_CE,
		GPIO_TypeDef* PORT_IRQ ,uint16_t PIN_IRQ);
uint8_t NRF24_ReadReg(nrf_handle* nrf,uint8_t addr);
void NRF24_Read_Buf(nrf_handle* nrf,uint8_t addr,uint8_t *pBuf,uint8_t bytes);
void NRF24_WriteReg(nrf_handle* nrf,uint8_t addr, uint8_t dt);
void NRF24_Write_Buf(nrf_handle* nrf,uint8_t addr,uint8_t *pBuf,uint8_t bytes);



uint8_t NRF24L01_Send(nrf_handle* nrf,uint8_t *pBuf);
void NRF24L01_Receive(nrf_handle* nrf,void* RX_BUF);

BaseType_t NRF24L01_Receive_freertos_irq(nrf_handle* nrf,void* RX_BUF,TickType_t blocktime);
uint8_t NRF24L01_Send_freertos_irq(nrf_handle* nrf,uint8_t *pBuf);
void NRF24L01_Send_NO_AA(nrf_handle* nrf,uint8_t *pBuf);
void NRF24L01_RX_Mode(nrf_handle* nrf);


void NRF24L01_Send_N_byte_no_aa(nrf_handle* nrf,uint8_t* ptr,uint16_t n);
void NRF24L01_Send_N_byte_no_aa_2CRC(nrf_handle* nrf,uint8_t* ptr,uint16_t n);



void NRF24_ini(nrf_handle* nrf);
void NRF24_ini_TX(nrf_handle* nrf);
void NRF24_ini_RX(nrf_handle* nrf);

//------------------------------------------------

#endif /* NRF24L01_H_ */
