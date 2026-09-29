/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2019 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gy89.h"
#include "UBX.h"
#include "nrf24l01.h"
#include "ADIS.h"
#include "PID.h"
#include "adapt_gps3.h"
#include "GPS.h"

typedef struct
{
    double A_a[3];
    double W_a[3];
} _orentation_A;

typedef struct
{
    double A_gy[3];
    double W_gy[3];
} _orentation_Gy;

typedef struct
{
    _orentation_A A;
    _orentation_Gy Gy;
} _orentation;

#define TX_ADR_WIDTH 3
#define TX_PLOAD_WIDTH 32
extern uint8_t TX_ADDRESS[TX_ADR_WIDTH];
extern uint8_t RX_ADDRESS[TX_ADR_WIDTH];

__IO uint16_t uhIC2Value[6] = {0x00};
__IO uint16_t uhDutyCycle[6] = {0x00};

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c3;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim8;
TIM_HandleTypeDef htim9;
TIM_HandleTypeDef htim12;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
nrf_handle nrf;
gy89i2c gy89;

osThreadId NRF24_thread_id;
osThreadId gy89_thread_id;
osThreadId UBX_thread_id;
osThreadId PID_thread_id;
osThreadId nav_thread_id;
osThreadId n8is_thread_id;

QueueHandle_t navigation_queue;
QueueHandle_t copter_queue;
QueueHandle_t _orentation_queue;

QueueHandle_t n8is_navigation_queue;

#define size_pack 1000
uint8_t buffer_RX1[size_pack];
uint8_t buffer_RX2[size_pack];

#define size_pack_n8is 1000
uint8_t buffer_RX1_n8is[size_pack_n8is];
uint8_t buffer_RX2_n8is[size_pack_n8is];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM14_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM8_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM9_Init(void);
static void MX_TIM12_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void const *argument);

/* USER CODE BEGIN PFP */
void NRF24_thread(void const *argument);
void gy89_thread(void const *argument);
void UBX_thread(void const *argument);
void PID_thread(void const *argument);
void nav_thread(void const *argument);
void n8is_thread(void const *argument);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    nrf = init_nrf_handle(&hspi2, NRF_CS_GPIO_Port, NRF_CS_Pin, NRF_CE_GPIO_Port, NRF_CE_Pin,
                          NRF_IRQ_GPIO_Port, NRF_IRQ_Pin);

    gy89 = InitHandle_gy89(&hi2c3);
    /* USER CODE END 1 */

    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();

    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI2_Init();
    MX_TIM14_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_I2C3_Init();
    MX_SPI1_Init();
    MX_TIM8_Init();
    MX_TIM5_Init();
    MX_TIM1_Init();
    MX_TIM4_Init();
    MX_TIM9_Init();
    MX_TIM12_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_USART3_UART_Init();
    /* USER CODE BEGIN 2 */
    HAL_Delay(250);
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(USART2_IRQn);

    HAL_TIM_Base_Start(&htim14);
    //------------------------------------------------------------------
    TIM2->CCR1 = 1010;
    TIM2->CCR2 = 1010;
    TIM2->CCR3 = 1010;
    TIM2->CCR4 = 1010;

    TIM3->CCR1 = 1010;
    TIM3->CCR2 = 1010;

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    //--------------------------------------------------------------------
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim9, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim9, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim12, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim12, TIM_CHANNEL_1);

    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    //---------------------------------------------------------------------

    NRF24_ini(&nrf);

    uint8_t gy_init = InitGy89(&gy89);
    uint8_t adis_init = init_ADIS();

    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_4, gy_init & adis_init);

    /* USER CODE END 2 */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* Create the thread(s) */
    /* definition and creation of defaultTask */
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    osThreadDef(NRF24_, NRF24_thread, osPriorityNormal, 0, 256);
    NRF24_thread_id = osThreadCreate(osThread(NRF24_), NULL);

    osThreadDef(gy89_, gy89_thread, osPriorityHigh, 0, 512);
    gy89_thread_id = osThreadCreate(osThread(gy89_), NULL);

    osThreadDef(UBX_thread_, UBX_thread, osPriorityHigh, 0, 512);
    UBX_thread_id = osThreadCreate(osThread(UBX_thread_), NULL);

    osThreadDef(PID_thread_, PID_thread, osPriorityHigh, 0, 512);
    PID_thread_id = osThreadCreate(osThread(PID_thread_), NULL);

    osThreadDef(nav_thread_, nav_thread, osPriorityHigh, 0, 768);
    nav_thread_id = osThreadCreate(osThread(nav_thread_), NULL);

    osThreadDef(n8is_thread, n8is_thread, osPriorityHigh, 0, 512);
    n8is_thread_id = osThreadCreate(osThread(n8is_thread), NULL);
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */

    navigation_queue = xQueueCreate(10, sizeof(navigation_mes));

    copter_queue = xQueueCreate(10, sizeof(copter));

    _orentation_queue = xQueueCreate(10, sizeof(_orentation));

    n8is_navigation_queue = xQueueCreate(10, sizeof(navigation_mes));

    if ((navigation_queue == NULL) || (copter_queue == NULL) || (_orentation_queue == NULL) ||
        (n8is_navigation_queue == NULL))
    {
        Error_Handler();
        return 1;
    }

    /* USER CODE END RTOS_QUEUES */

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /**Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /**Activate the Over-Drive mode
     */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }
    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection =
        RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_I2C3;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief I2C3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C3_Init(void)
{

    /* USER CODE BEGIN I2C3_Init 0 */

    /* USER CODE END I2C3_Init 0 */

    /* USER CODE BEGIN I2C3_Init 1 */

    /* USER CODE END I2C3_Init 1 */
    hi2c3.Instance = I2C3;
    hi2c3.Init.Timing = 0x6000030D;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c3) != HAL_OK)
    {
        Error_Handler();
    }
    /**Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    /**Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C3_Init 2 */

    /* USER CODE END I2C3_Init 2 */
}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void)
{

    /* USER CODE BEGIN SPI1_Init 0 */

    /* USER CODE END SPI1_Init 0 */

    /* USER CODE BEGIN SPI1_Init 1 */

    /* USER CODE END SPI1_Init 1 */
    /* SPI1 parameter configuration*/
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN SPI1_Init 2 */

    /* USER CODE END SPI1_Init 2 */
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void)
{

    /* USER CODE BEGIN SPI2_Init 0 */

    /* USER CODE END SPI2_Init 0 */

    /* USER CODE BEGIN SPI2_Init 1 */

    /* USER CODE END SPI2_Init 1 */
    /* SPI2 parameter configuration*/
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 7;
    hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN SPI2_Init 2 */

    /* USER CODE END SPI2_Init 2 */
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void)
{

    /* USER CODE BEGIN TIM1_Init 0 */

    /* USER CODE END TIM1_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM1_Init 1 */

    /* USER CODE END TIM1_Init 1 */
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 215;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 65535;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchronization(&htim1, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM1_Init 2 */

    /* USER CODE END TIM1_Init 2 */
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

    /* USER CODE BEGIN TIM2_Init 0 */

    /* USER CODE END TIM2_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* USER CODE BEGIN TIM2_Init 1 */

    /* USER CODE END TIM2_Init 1 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 107;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM2_Init 2 */

    /* USER CODE END TIM2_Init 2 */
    HAL_TIM_MspPostInit(&htim2);
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

    /* USER CODE BEGIN TIM3_Init 0 */

    /* USER CODE END TIM3_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* USER CODE BEGIN TIM3_Init 1 */

    /* USER CODE END TIM3_Init 1 */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 107;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 1999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM3_Init 2 */

    /* USER CODE END TIM3_Init 2 */
    HAL_TIM_MspPostInit(&htim3);
}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void)
{

    /* USER CODE BEGIN TIM4_Init 0 */

    /* USER CODE END TIM4_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM4_Init 1 */

    /* USER CODE END TIM4_Init 1 */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 107;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 65535;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchronization(&htim4, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM4_Init 2 */

    /* USER CODE END TIM4_Init 2 */
}

/**
 * @brief TIM5 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM5_Init(void)
{

    /* USER CODE BEGIN TIM5_Init 0 */

    /* USER CODE END TIM5_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM5_Init 1 */

    /* USER CODE END TIM5_Init 1 */
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 107;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 65535;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim5) != HAL_OK)
    {
        Error_Handler();
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchronization(&htim5, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM5_Init 2 */

    /* USER CODE END TIM5_Init 2 */
}

/**
 * @brief TIM8 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM8_Init(void)
{

    /* USER CODE BEGIN TIM8_Init 0 */

    /* USER CODE END TIM8_Init 0 */

    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* USER CODE BEGIN TIM8_Init 1 */

    /* USER CODE END TIM8_Init 1 */
    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 214;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 65535;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_IC_Init(&htim8) != HAL_OK)
    {
        Error_Handler();
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchronization(&htim8, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim8, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM8_Init 2 */

    /* USER CODE END TIM8_Init 2 */
}

/**
 * @brief TIM9 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM9_Init(void)
{

    /* USER CODE BEGIN TIM9_Init 0 */

    /* USER CODE END TIM9_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    /* USER CODE BEGIN TIM9_Init 1 */

    /* USER CODE END TIM9_Init 1 */
    htim9.Instance = TIM9;
    htim9.Init.Prescaler = 207;
    htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim9.Init.Period = 65535;
    htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim9) != HAL_OK)
    {
        Error_Handler();
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchronization(&htim9, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim9, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim9, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM9_Init 2 */

    /* USER CODE END TIM9_Init 2 */
}

/**
 * @brief TIM12 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM12_Init(void)
{

    /* USER CODE BEGIN TIM12_Init 0 */

    /* USER CODE END TIM12_Init 0 */

    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    /* USER CODE BEGIN TIM12_Init 1 */

    /* USER CODE END TIM12_Init 1 */
    htim12.Instance = TIM12;
    htim12.Init.Prescaler = 107;
    htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim12.Init.Period = 65535;
    htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim12) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim12) != HAL_OK)
    {
        Error_Handler();
    }
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
    sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    if (HAL_TIM_SlaveConfigSynchronization(&htim12, &sSlaveConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim12, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    if (HAL_TIM_IC_ConfigChannel(&htim12, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM12_Init 2 */

    /* USER CODE END TIM12_Init 2 */
}

/**
 * @brief TIM14 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM14_Init(void)
{

    /* USER CODE BEGIN TIM14_Init 0 */

    /* USER CODE END TIM14_Init 0 */

    /* USER CODE BEGIN TIM14_Init 1 */

    /* USER CODE END TIM14_Init 1 */
    htim14.Instance = TIM14;
    htim14.Init.Prescaler = 108;
    htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim14.Init.Period = 65535;
    htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM14_Init 2 */

    /* USER CODE END TIM14_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 576000;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void)
{

    /* USER CODE BEGIN USART3_Init 0 */

    /* USER CODE END USART3_Init 0 */

    /* USER CODE BEGIN USART3_Init 1 */

    /* USER CODE END USART3_Init 1 */
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_9B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_ODD;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART3_Init 2 */

    /* USER CODE END USART3_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Stream5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    /* DMA1_Stream6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    /* DMA2_Stream7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, RIGHT_LED_Pin | LEFT_LED_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOF, GPS_3dPos_Pin | ADIS_State_Pin | NRF_CS_Pin | NRF_CE_Pin,
                      GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(ADIS_CS_GPIO_Port, ADIS_CS_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, ADIS_DIO1_Pin | ADIS_DIO2_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : RIGHT_LED_Pin LEFT_LED_Pin */
    GPIO_InitStruct.Pin = RIGHT_LED_Pin | LEFT_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : GPS_3dPos_Pin ADIS_State_Pin */
    GPIO_InitStruct.Pin = GPS_3dPos_Pin | ADIS_State_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /*Configure GPIO pin : NRF_IRQ_Pin */
    GPIO_InitStruct.Pin = NRF_IRQ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : NRF_CS_Pin NRF_CE_Pin */
    GPIO_InitStruct.Pin = NRF_CS_Pin | NRF_CE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /*Configure GPIO pin : ADIS_CS_Pin */
    GPIO_InitStruct.Pin = ADIS_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(ADIS_CS_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : ADIS_DIO1_Pin ADIS_DIO2_Pin */
    GPIO_InitStruct.Pin = ADIS_DIO1_Pin | ADIS_DIO2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : BUTTON_LEFT_Pin */
    GPIO_InitStruct.Pin = BUTTON_LEFT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BUTTON_LEFT_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* USER CODE BEGIN 4 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    switch ((uint32_t)htim->Instance)
    {
    case (uint32_t)TIM5:

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            /* Get the Input Capture value */
            uhIC2Value[0] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (uhIC2Value[0] != 0)
            {
                /* Duty cycle computation */
                uhDutyCycle[0] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            }
            else
            {
                uhDutyCycle[0] = 0;
            }
        }
        break;
    case (uint32_t)TIM4:

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            /* Get the Input Capture value */
            uhIC2Value[1] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (uhIC2Value[1] != 0)
            {
                /* Duty cycle computation */
                uhDutyCycle[1] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            }
            else
            {
                uhDutyCycle[1] = 0;
            }
        }
        break;
    case (uint32_t)TIM8:

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            /* Get the Input Capture value */
            uhIC2Value[2] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (uhIC2Value[2] != 0)
            {
                /* Duty cycle computation */
                uhDutyCycle[2] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            }
            else
            {
                uhDutyCycle[2] = 0;
            }
        }
        break;
    case (uint32_t)TIM9:

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            /* Get the Input Capture value */
            uhIC2Value[3] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (uhIC2Value[3] != 0)
            {
                /* Duty cycle computation */
                uhDutyCycle[3] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            }
            else
            {
                uhDutyCycle[3] = 0;
            }
        }
        break;
    case (uint32_t)TIM12:

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            /* Get the Input Capture value */
            uhIC2Value[4] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (uhIC2Value[4] != 0)
            {
                /* Duty cycle computation */
                uhDutyCycle[4] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            }
            else
            {
                uhDutyCycle[4] = 0;
            }
        }
        break;
    case (uint32_t)TIM1:

        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            /* Get the Input Capture value */
            uhIC2Value[5] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (uhIC2Value[5] != 0)
            {
                /* Duty cycle computation */
                uhDutyCycle[5] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
            }
            else
            {
                uhDutyCycle[5] = 0;
            }
        }
        break;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uint8_t str[20];

    if (huart == &huart2)
    {
        int strlength = sprintf((char *)str, "error %lu \r\n", huart->ErrorCode);

        HAL_UART_Transmit(&huart1, str, strlength, 0xff);
        HAL_UART_Receive_IT(&huart2, buffer_RX1, size_pack);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if ((huart->pRxBuffPtr - huart->RxXferSize) == buffer_RX1)
        {
            HAL_UART_Receive_IT(&huart2, buffer_RX2, size_pack);
            xTaskNotifyFromISR(UBX_thread_id, 0x1, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            return;
        }
        else if ((huart->pRxBuffPtr - huart->RxXferSize) == buffer_RX2)
        {
            HAL_UART_Receive_IT(&huart2, buffer_RX1, size_pack);
            xTaskNotifyFromISR(UBX_thread_id, 0x2, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            return;
        }
    }

    if (huart->Instance == USART3)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if ((huart->pRxBuffPtr - huart->RxXferSize) == buffer_RX1_n8is)
        {
            HAL_UART_Receive_IT(&huart3, buffer_RX2_n8is, size_pack_n8is);
            xTaskNotifyFromISR(n8is_thread_id, 0x1, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            return;
        }
        else if ((huart->pRxBuffPtr - huart->RxXferSize) == buffer_RX2_n8is)
        {
            HAL_UART_Receive_IT(&huart3, buffer_RX1_n8is, size_pack_n8is);
            xTaskNotifyFromISR(n8is_thread_id, 0x2, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            return;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == GPIO_PIN_6)
    {
        if (nrf.interrupt_stat == 0x00) // read
        {
            xTaskNotifyFromISR(NRF24_thread_id, 0x1, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else if (nrf.interrupt_stat == 1)
        {
            xTaskNotifyFromISR(NRF24_thread_id, 0x02, eSetValueWithOverwrite,
                               &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void UBX_thread(void const *argument)
{

    uint32_t signals = 0x00;
    BaseType_t ret = 0;

    uint16_t readed;
    uint8_t *huart_ptr;

    uint16_t readed_pr = 0;

    navigation_mes mes;
    osDelay(2000);

    UBX_init();
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_UART_Receive_IT(&huart2, buffer_RX1, size_pack);
    for (;;)
    {

        ret = xTaskNotifyWait(0, (uint32_t)0, &signals, 50);
        if (ret == pdPASS)
        {
            readed = size_pack;
            switch (signals)
            {
            case 0x1:
                pars(buffer_RX1 + readed_pr, readed - readed_pr, &mes);
                break;
            case 0x2:
                pars(buffer_RX2 + readed_pr, readed - readed_pr, &mes);
                break;
            }
            if ((mes._mess_ready & 0xf) == 0xf)
            {

                xQueueSend(navigation_queue, (void *)&mes, 0);
                mes._mess_ready = 0x00;
            }
            readed_pr = 0;
        }
        else
        {
            readed = size_pack - huart2.RxXferCount;
            huart_ptr = huart2.pRxBuffPtr;
            if ((huart_ptr - readed) == buffer_RX1)
            {
                pars(buffer_RX1 + readed_pr, readed - readed_pr, &mes);
            }
            else if ((huart_ptr - readed) == buffer_RX2)
            {
                pars(buffer_RX2 + readed_pr, readed - readed_pr, &mes);
            }

            if ((mes._mess_ready & 0xf) == 0xf)
            {
                xQueueSend(navigation_queue, (void *)&mes, 0);
                mes._mess_ready = 0x00;
            }
            readed_pr = readed;
        }
    }
}

void NRF24_thread(void const *argument)
{
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    NRF24_Write_Buf(&nrf, TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
    NRF24_Write_Buf(&nrf, RX_ADDR_P0, RX_ADDRESS, TX_ADR_WIDTH);

    for (;;)
    {
        osDelay(1000);
    }
}

void gy89_thread(void const *argument)
{
    double accel[3];
    double mag[3];
    double gyro[3];

    TickType_t tick;

    _orentation A_GY;
    ADIS_DATA data;

    for (;;)
    {

        tick = xTaskGetTickCount();

        read_high_dword_all_ADIS();
        ADIS_DATA_CON(&data);
        read_data_LSM303D_d(&hi2c3, accel, mag);
        read_data_l3gd20_d(&hi2c3, gyro);

        A_GY.A.A_a[0] = data.accel[0];
        A_GY.A.A_a[1] = data.accel[2];
        A_GY.A.A_a[2] = -data.accel[1];

        A_GY.A.W_a[0] = data.omega[0] * M_PI / 180.0;
        A_GY.A.W_a[1] = data.omega[2] * M_PI / 180.0;
        A_GY.A.W_a[2] = -data.omega[1] * M_PI / 180.0;

        A_GY.Gy.A_gy[0] = -accel[2];
        A_GY.Gy.A_gy[1] = -accel[1];
        A_GY.Gy.A_gy[2] = -accel[0];

        A_GY.Gy.W_gy[0] = -gyro[2] * M_PI / 180.0;
        A_GY.Gy.W_gy[1] = -gyro[0] * M_PI / 180.0;
        A_GY.Gy.W_gy[2] = gyro[1] * M_PI / 180.0;

        xQueueSend(_orentation_queue, (void *)&A_GY, 0);

        vTaskDelayUntil(&tick, 10);
    }
}

void nav_thread(void const *argument)
{

    _orentation A_GY;
    navigation_mes mes;

    uint8_t str[500];
    int strlength = 0;
    BaseType_t ref;
    adapt_gps3_initialize();

    double angle[3];
    double SNS_V[3];

    uint32_t cnt = 0;

    TickType_t tick;
    TickType_t tick1;

    strlength = sprintf((char *)str,
                        "AdisAx AdisAy AdisAz " // 1
                        "AdisWx AdisWy AdisWz " // 2
                        "GyAx GyAy GyAz "       // 3
                        "GyWx GyWy GyWz "       // 4
                        "SnsVN SnsVU SnsVE "    // 5
                        "gps3DFix "
                        "gpsLatit gpsLong gpsHighmm "
                        "outPsi outTheta outGamma "
                        "gpsNewData gpsFix time timeExec \r\n");
    HAL_UART_Transmit(&huart1, str, strlength, 0xff);

    for (;;)
    {
        xQueueReceive(_orentation_queue, (void *)&A_GY, portMAX_DELAY);
        ref = xQueueReceive(navigation_queue, (void *)&mes, 0);
        SNS_V[0] = mes._NAV_VELNED_.velN / 100.0;
        SNS_V[1] = -mes._NAV_VELNED_.velD / 100.0;
        SNS_V[2] = mes._NAV_VELNED_.velE / 100.0;
        tick = xTaskGetTickCount();

        adapt_gps3(A_GY.Gy.W_gy, A_GY.Gy.A_gy, SNS_V, mes._NAV_STATUS_.gpsFix == 3,
                   mes._NAV_POSLLH_.latitude * M_PI / 180.0, &angle[1], &angle[2], &angle[0]);
        tick1 = xTaskGetTickCount();

        strlength =
            sprintf((char *)str,
                    "%10.5f %10.5f %10.5f " // 1
                    "%10.5f %10.5f %10.5f " // 2
                    "%10.5f %10.5f %10.5f " // 3
                    "%10.5f %10.5f %10.5f " // 4

                    "%20.15f %20.15f %20.15f " // 5
                    "%3u "
                    "%20.15f %20.15f %20.15f "

                    "%10.5f %10.5f %10.5f "
                    "%lu %u %8.3f %8lu \r\n",
                    A_GY.A.A_a[0], A_GY.A.A_a[1], A_GY.A.A_a[2], A_GY.A.W_a[0], A_GY.A.W_a[1],
                    A_GY.A.W_a[2], A_GY.Gy.A_gy[0], A_GY.Gy.A_gy[1], A_GY.Gy.A_gy[2],
                    A_GY.Gy.W_gy[0], A_GY.Gy.W_gy[1], A_GY.Gy.W_gy[2],

                    SNS_V[0], SNS_V[1], SNS_V[2], mes._NAV_STATUS_.gpsFix == 3,
                    mes._NAV_POSLLH_.latitude * M_PI / 180.0,
                    mes._NAV_POSLLH_.longitude * M_PI / 180.0, mes._NAV_POSLLH_.Height_above_sea,

                    angle[0] * 180.0 / M_PI, angle[1] * 180.0 / M_PI, angle[2] * 180.0 / M_PI, ref,
                    mes._NAV_STATUS_.gpsFix, cnt * 0.01, tick1 - tick);

        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, mes._NAV_STATUS_.gpsFix == 3);
        HAL_UART_Transmit_DMA(&huart1, str, strlength);
        cnt++;
    }
}

void PID_thread(void const *argument)
{
    uint16_t m_power[6] = {1000, 1000, 1000, 1000, 1000, 1000};
    adapt_gps3_initialize();

    for (;;)
    {

        TIM2->CCR1 = m_power[0];
        TIM2->CCR2 = m_power[1];
        TIM2->CCR3 = m_power[2];
        TIM2->CCR4 = m_power[3];

        TIM3->CCR1 = m_power[4];
        TIM3->CCR2 = m_power[5];
        osDelay(10);
    }
}

void n8is_thread(void const *argument)
{

    uint32_t signals = 0x00;
    BaseType_t ret = 0;

    uint16_t readed;
    uint8_t *huart_ptr;

    uint16_t readed_pr = 0;

    GPS_DATA mes;
    osDelay(2000);

    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_UART_Receive_IT(&huart3, buffer_RX1_n8is, size_pack);
    for (;;)
    {

        ret = xTaskNotifyWait(0, (uint32_t)0, &signals, 50);
        if (ret == pdPASS)
        {
            readed = size_pack;
            switch (signals)
            {
            case 0x1:
                pars_N8IS(buffer_RX1_n8is + readed_pr, readed - readed_pr, &mes);
                break;
            case 0x2:
                pars_N8IS(buffer_RX2_n8is + readed_pr, readed - readed_pr, &mes);
                break;
            }
            if ((mes._mess_ready & 0xf) == 0xf)
            {

                xQueueSend(navigation_queue, (void *)&mes, 0);
                mes._mess_ready = 0x00;
            }
            readed_pr = 0;
        }
        else
        {
            readed = size_pack - huart3.RxXferCount;
            huart_ptr = huart3.pRxBuffPtr;
            if ((huart_ptr - readed) == buffer_RX1_n8is)
            {
                pars_N8IS(buffer_RX1_n8is + readed_pr, readed - readed_pr, &mes);
            }
            else if ((huart_ptr - readed) == buffer_RX2_n8is)
            {
                pars_N8IS(buffer_RX2_n8is + readed_pr, readed - readed_pr, &mes);
            }

            if ((mes._mess_ready & 0xf) == 0xf)
            {
                xQueueSend(n8is_navigation_queue, (void *)&mes, 0);
                mes._mess_ready = 0x00;
            }
            readed_pr = readed;
        }
    }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const *argument)
{

    /* USER CODE BEGIN 5 */

    /* Infinite loop */
    for (;;)
    {

        osThreadTerminate(PID_thread_id);
        osThreadTerminate(NRF24_thread_id);
        osThreadTerminate(defaultTaskHandle);

        osDelay(100);
    }
    /* USER CODE END 5 */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM6)
    {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */
    if (htim->Instance == TIM8)
    {
    }
    /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
