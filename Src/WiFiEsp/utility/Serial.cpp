/*
 * Serial.cpp
 *
 *  Created on: May 3, 2019
 *      Author: Archer
 */

#include <utility/Serial.h>

Serial::Serial(UART_HandleTypeDef &uart, int txSize, int rxSize)
{
	m_uart = uart;
	m_txBuf = new Queue(txSize);
	m_rxBuf = new Queue(rxSize);

	// init HAL Uart -- done by STM32Cube code
	HAL_UART_Receive_IT(&huart1, (uint8_t *) aRxBuffer, 10);

}

Serial::~Serial()
{
	// TODO Auto-generated destructor stub
}

void USART2_IRQHandler(void)
{
	volatile unsigned int IIR;
	struct buf_st *p;

	IIR = USART2->SR;
	if (IIR & USART_FLAG_RXNE)
	{                  // read interrupt
		USART2->SR &= ~USART_FLAG_RXNE;                  // clear interrupt

		if (!Serial::RxBuf.IsFull())
		{
			data = (USART2->DR & 0xFF);
			Serial::RxBuf.Push(data);
		}
	}
	if (IIR & USART_FLAG_TXE)
	{
		USART1->SR &= ~USART_FLAG_TXE;	          // clear interrupt

		p = &tbuf;

		if (p->in != p->out)
		{
			USART1->DR = (p->buf[p->out & (TBUF_SIZE - 1)] & 0x1FF);
			p->out++;
			tx_restart = 0;
		}
		else
		{
			tx_restart = 1;
			USART1->CR1 &= ~USART_FLAG_TXE;	// disable TX interrupt if nothing to send

		}
	}
}
