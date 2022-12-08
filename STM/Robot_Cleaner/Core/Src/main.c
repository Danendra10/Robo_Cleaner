/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define v_kiri TIM2 -> CCR1
#define v_kanan TIM2 -> CCR2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
void Stop(void);
void Forward(void);
void Left(void);
void Right(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t flag;
//=-----Serial Communication-----=//

float gyro_z_data;
float dist_to_obj;
uint8_t state;

#define rx_size 20
#define main_size 30

uint8_t rx_msgs[rx_size];
uint8_t main_msgs[main_size];

char *datas;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  if (huart->Instance == USART6)
  {
	  memcpy(&state, rx_msgs, 1);
	  memcpy(&gyro_z_data, rx_msgs + 1, 4);
	  memcpy(&dist_to_obj, rx_msgs + 5, 4);
//    // split the rx_msgs when found a comma
//    datas = strtok((char *)rx_msgs, "~");
//    // get the first data
//    state = atoi(datas);
//    // get the second data
//    datas = strtok(NULL, "~");
//    gyro_z_data = atof(datas);
//    // get the third data
//    datas = strtok(NULL, "~");
//    dist_to_obj = atof(datas);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_msgs, rx_size);
    __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
  }
}

char buff[10];
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

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
  MX_USART6_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  lcd_init();
  lcd_put_cur(0, 3);
  lcd_send_string("Initializing");
  HAL_Delay(1000);
  lcd_send_string(".");
  HAL_Delay(1000);
  lcd_send_string(".");
  HAL_Delay(1000);
  lcd_send_string(".");
  HAL_Delay(1000);
  lcd_clear();

  lcd_put_cur(0, 3);
  lcd_send_string("HELLO SET :)");
  lcd_clear();

  HAL_UARTEx_ReceiveToIdle_DMA(&huart6, rx_msgs, rx_size);
  __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);

  static uint8_t algorithm_state;

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

  v_kanan = 600;
  v_kiri = 820;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  lcd_put_cur(0, 0);
	  sprintf(buff, "%d", algorithm_state);
	  lcd_send_string(buff);
	  lcd_put_cur(0, 15);
	  sprintf(buff, "%d", state);
	  lcd_send_string(buff);

	  lcd_put_cur(1, 0);
	  sprintf(buff, "%.2f", gyro_z_data);
	  lcd_send_string(buff);
	  lcd_put_cur(1, 8);
	  sprintf(buff, "%.2f", dist_to_obj);
	  lcd_send_string(buff);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  Forward();

	  switch(state)
	  {

	  case 0:
		  algorithm_state = 0;
		  Stop();
		  break;
	  case 1:
		  switch(algorithm_state)
		  {
		  case 0:
			  if(!flag)
				  algorithm_state = 10;
			  else
				  algorithm_state = 30;
			  break;

		  case 10:
			  Forward();
			  if(gyro_z_data > 10 )
				  Right();
			  if(gyro_z_data < -10 )
				  Left();

			  if(dist_to_obj < 20)
				  algorithm_state = 11;
			  break;
		  case 11:
			  Right();
			  if(gyro_z_data < -90 + 10)
				  algorithm_state = 12;
			  break;
		  case 12:
			  Forward();
			  if(gyro_z_data > -85)
				  Right();
			  if(gyro_z_data < -95)
				  Left();
			  HAL_Delay(500);
			  algorithm_state = 13;
			  break;
		  case 13:
			  Right();
			  if(gyro_z_data < -180 + 10)
			  {
				  flag = 1;
				  algorithm_state = 0;
			  }
			  break;


		  case 30:
			  Forward();
			  if(gyro_z_data < -170 )
				  Right();
			  if(gyro_z_data > 170 )
				  Left();

			  if(dist_to_obj < 10)
				  algorithm_state = 31;
			  break;
		  case 31:
			  Left();
			  if(gyro_z_data > -90)
				  algorithm_state = 32;
			  break;
		  case 32:
			  Forward();
			  if(gyro_z_data > -85)
				  Right();
			  if(gyro_z_data < -95)
				  Left();
			  HAL_Delay(500);
			  algorithm_state = 33;
			  break;
		  case 33:
			  Left();
			  if(gyro_z_data > - 10)
			  {
				  flag = 0;
				  algorithm_state = 0;
			  }
			  break;


		  }
		  break;
	  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void Stop(void)
{
	lcd_put_cur(0, 5);
	lcd_send_string("Stop");
	lcd_clear();
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
}
void Forward(void)
{
	lcd_put_cur(0, 5);
	lcd_send_string("Forward");
	lcd_clear();
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
}
void Left(void)
{
	lcd_put_cur(0, 5);
	lcd_send_string("Left");
	lcd_clear();
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
}
void Right(void)
{
	lcd_put_cur(0, 5);
	lcd_send_string("Right");
	lcd_clear();
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
