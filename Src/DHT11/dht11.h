/*
 * dht11.h
 *
 *  Created on: Mar 30, 2019
 *      Author: Archer
 */

#ifndef DHT11_DHT11_H_
#define DHT11_DHT11_H_

#include <stdio.h>
#include <stm32l4xx_hal.h>


class DHT11
{
	public:
		DHT11(GPIO_TypeDef *port, uint32_t m_pin);
		bool ReadSensor(void);

	private:
		void SetGpioOutput(void);
		void SetGpioInput(void);
		uint8_t ReadData(void);
		void CheckResponse(void);
		void Start(void);


		GPIO_TypeDef    *m_port;
		uint32_t		 m_pin;

		uint8_t m_Rh[2];
		uint8_t m_Temp[2];
		uint16_t sum, RH, TEMP;

		int temp_low, temp_high, rh_low, rh_high;
		char temp_char1[2], temp_char2, rh_char1[2], rh_char2;

		uint8_t check;

		GPIO_InitTypeDef GPIO_InitStruct;
};

#endif /* DHT11_DHT11_H_ */
