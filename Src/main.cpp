/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "lptim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "NS_energyShield2.h"
#include "SFE_BMP180.h"

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
/* USER CODE BEGIN PV */
NS_energyShield2 Es2(&hi2c3);		// use 3rd I2C
SFE_BMP180 Bmp(&hi2c3);
DHT11 dht(GPIOA, GPIO_PIN_1);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
	MX_I2C1_Init();
	MX_I2C2_Init();
	MX_I2C3_Init();
	MX_LPTIM1_Init();
	MX_USART2_UART_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */

	Bmp.Begin();
	byte wait = Bmp.StartTemperature();
	HAL_Delay(wait);
	float temp = 0.0;
	Bmp.GetTemperature(temp);
	printf("MAIN: BMP180 temp = %f\n", temp);

	if (Es2.begin())
	{
		printf("! ES2 ERROR - Initialization Failed !");
		while (1)
			;
	}
	if (Es2.readVMPP() != -1)
		Es2.setVMPP(-1, 1); // Disable VMPP regulation to allow charging from any power supply (7V - 23V) and prevent excessive EEPROM writes

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		//double pressure = 0.0;
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		// Read values from energyShield2
		int batVal = Es2.batteryVoltage();
		int iBattVolt = batVal / 1000;
		int nBattVolt = batVal % 1000;
		float batteryVoltage = (float) batVal / 1000.0;

		float batteryCurrent = (float) Es2.batteryCurrent() / 1000.0;
		uint16_t fullCapacity = Es2.fullChargeCapacity();
		uint16_t remainingCapacity = Es2.remainingCapacity();
		uint16_t stateOfCharge = Es2.SOC();
		float inputVoltage = (float) Es2.inputVoltage(0) / 1000;
		float temperature = (float) Es2.temperature() / 10;

		// Print Results
		printf("Battery Voltage    = %f V\n", batteryVoltage);
		printf("Battery Voltage    = %d.%03d V\n", iBattVolt, nBattVolt);
		printf("Battery Current    = %f V\n", batteryCurrent);
		printf("Full Capacity      = %d mAh\n", fullCapacity);
		printf("Remaining Capacity = %d mAh\n", remainingCapacity);
		printf("State of Charge    = %d %% \n", stateOfCharge);
		printf("Input Voltage      = %f V\n", inputVoltage);
		printf("Batt. Temp         = %f F\n", (temperature * 9.0 / 5.0) + 32.0);

		// check pressure from BMP180
		wait = Bmp.StartTemperature();
		HAL_Delay(wait);
		temp = 0.0;
		Bmp.GetTemperature(temp);
		printf("BMP180 Temp = %f C, %f F\n", temp, ((temp * 9.0) / 5) + 32);
		wait = Bmp.StartPressure(1);
		HAL_Delay(wait);
		float pressure;
		Bmp.GetPressure(pressure, temp);

		// Read time and date from energyShield and store locally
		// Local values will not update until readClock is called again
		Es2.readClock();

		// Print time and date from locally stored values
		printf("\n\tTime:: %d:%d:%d \t Date:: %d-%d-%d\n", Es2.hour(),
				Es2.minute(), Es2.second(), Es2.month(), Es2.dayOfMonth(),
				Es2.year());

		// Print carriage return to start new line
		printf("\n\n\n");

		// Wait between reads
		HAL_Delay(2000);

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit =
	{ 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 10;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2
			| RCC_PERIPHCLK_LPTIM1 | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_I2C2
			| RCC_PERIPHCLK_I2C3 | RCC_PERIPHCLK_ADC;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
	PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
	PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
	PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
	PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
	PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
	PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
	PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(char *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
