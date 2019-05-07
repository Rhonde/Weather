/*
 * Serial.h
 *
 *  Created on: May 3, 2019
 *      Author: Archer
 */

#ifndef WIFIESP_UTILITY_SERIAL_H_
#define WIFIESP_UTILITY_SERIAL_H_

#include <ctype.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>

#include "Queue.h"


class Serial
{
public:
	Serial(UART_HandleTypeDef &uart, int txSize, int rxSize);
	~Serial();

	//////////////////////////////////////////////////////////////////////////////
	// Read(uint8_t *ptr, int  count, int waitMs);
	//
	// read from the ring buffer the requested number of uint8_ts
	//
	//		ptr		pointer to the data
	//		count	the number of uint8_ts to read
	// 		waitMs 	specifies the time to wait
	//			0		no wait, return immediately after it reads what it can
	//			-1		wait forever till count is satisfied
	//			x		wait x milli-seconds, or till buffer is full
	//
	//	returns the number of uint8_ts read or -1 if an error is encountered (parity, over-run, etc...)
	//
	int Read(uint8_t *ptr, int  count, int waitMs = -1);

	//////////////////////////////////////////////////////////////////////////////
	// Write(uint8_t *ptr, int  count, int waitMs);
	//
	// writes to the ring buffer the requested number of uint8_ts,
	// if the UART is not busy, it take a 1 to 8 uint8_ts from the front
	// and call the HAL write interrup
	//
	//		ptr		pointer to the data
	//		count	the number of uint8_ts to write
	// 		waitMs specifies the time to wait
	//			0		no wait, return immediately after it reads what it can
	//			-1		wait forever till count is satisfied
	//			x		wait x milli-seconds, or till buffer is full
	//
	//	returns the number of uint8_ts written
	//	-1 if an error is encountered (timeout)
	//
	int Write(uint8_t *ptr, int count, int waitMs = -1);


	//////////////////////////////////////////////////////////////////////////////
	//	int Available();
	//
	//	returns true if data is available in the ring buffer
	//	returns false for no data
	//
	int Available();

	//////////////////////////////////////////////////////////////////////////////
	//	int ParseInt(uint8 radix = 10, int waitMs = -1);
	//
	//	Reads and converts a number from the ring buffer.
	//	Conversion stops when an invalid character is received
	//	Values are converted MSB first
	//
	//		radix	number base, allowable values are:
	//			2		binary ('0', '1')
	//			10		decimal ([0-9])
	//			16		hex ([0-9],[A-F],[a-f] -- a prefix on '0X' or '0x' or 'h' is allowed
	//
	// 		waitMs specifies the time to wait
	//			0		no wait, return immediately after it reads what it can
	//			-1		wait forever till count is satisfied
	//			x		wait x milli-seconds, or till buffer is full
	//	returns the converted integer.
	//
	int ParseInt(uint8_t radix = 10, int waitMs = -1);

	//////////////////////////////////////////////////////////////////////////////
	//	void Flush();
	//
	//	empties the buffer
	//
	void Flush();

	uint8_t Peek();		// read data without popping it from the queue
	uint8_t GetChar();	// simply reads 1 character from the queue

private:
	UART_HandleTypeDef m_uart;

};

#endif /* WIFIESP_UTILITY_SERIAL_H_ */
