/**
 ******************************************************************************
 * @file           : dht.c
 * @brief          : DHT11 Interface routines
 ******************************************************************************
**/

#include "dwt_stm32_delay.h"
#include "dht11.h"

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

DHT11::DHT11(GPIO_TypeDef  *port, uint32_t pin)
{
	m_port = port;
	m_pin = pin;
}

void DHT11::SetGpioOutput(void)
{
	/*Configure GPIO pin output: PA2 */
	GPIO_InitStruct.Pin = m_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(m_port, &GPIO_InitStruct);
}

void DHT11::SetGpioInput(void)
{
	/*Configure GPIO pin input: PA2 */
	GPIO_InitStruct.Pin = m_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(m_port, &GPIO_InitStruct);
}

void DHT11::Start(void)
{
	SetGpioOutput();  // set the pin as output

	HAL_GPIO_WritePin(m_port, m_pin, GPIO_PIN_RESET); 		 // pull the pin low

	HAL_Delay(18);   // wait for 18ms

	SetGpioInput();   // set as input

}

void DHT11::CheckResponse(void)
{
	DWT_Delay_us(40);

	if (!(HAL_GPIO_ReadPin(m_port, m_pin)))
	{
		DWT_Delay_us(80);
		if ((HAL_GPIO_ReadPin(m_port, m_pin)))
			check = 1;
	}

	while ((HAL_GPIO_ReadPin(m_port, m_pin)))
		;   // wait for the pin to go low
}

uint8_t DHT11::ReadData(void)
{
	uint8_t i, j;

	for (j = 0; j < 8; j++)
	{
		while (!(HAL_GPIO_ReadPin(m_port, m_pin)))			;   // wait for the pin to go high

		DWT_Delay_us(40);   // wait for 40 us
		if ((HAL_GPIO_ReadPin(m_port, m_pin)) == 0)   // if the pin is low
		{
			i &= ~(1 << (7 - j));   // write 0
		}
		else
			i |= (1 << (7 - j));  // if the pin is high, write 1

		while ((HAL_GPIO_ReadPin(m_port, m_pin)));  // wait for the pin to go low
	}
	return i;
}

bool DHT11::ReadSensor(void)
{
	int sum;
	Start();
	CheckResponse();

	m_Rh[0] = ReadData();
	m_Rh[1] = ReadData();
	m_Temp[0] = ReadData();
	m_Temp[1] = ReadData();
	sum = ReadData();

	if (sum != (m_Rh[0] + m_Rh[1] + m_Temp[0] + m_Temp[1])) 	// if the data is correct
		return false;

	return true;
}

float DHT11::GetTemperature(void)
{
	return (float)m_Temp[0] + m_Temp[1]/255.0;
}

float DHT11::GetHumidity(void)
{
	return (float)m_Rh[0] + m_Rh[1]/255.0;
}
