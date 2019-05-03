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
	Serial(UART_HandleTypeDef &uart, int baud, int parity, int txSize, int rxSize);
	~Serial();

	//////////////////////////////////////////////////////////////////////////////
	// Read(byte *ptr, int  count, int waitMs);
	//
	// read from the ring buffer the requested number of bytes
	//
	//		ptr		pointer to the data
	//		count	the number of bytes to read
	// 		waitMs 	specifies the time to wait
	//			0		no wait, return immediately after it reads what it can
	//			-1		wait forever till count is satisfied
	//			x		wait x milli-seconds, or till buffer is full
	//
	//	returns the number of bytes read or -1 if an error is encountered (parity, over-run, etc...)
	//
	int Read(byte *ptr, int  count, int waitMs = -1);

	//////////////////////////////////////////////////////////////////////////////
	// Write(byte *ptr, int  count, int waitMs);
	//
	// writes to the ring buffer the requested number of bytes,
	// if the UART is not busy, it take a 1 to 8 bytes from the front
	// and call the HAL write interrup
	//
	//		ptr		pointer to the data
	//		count	the number of bytes to write
	// 		waitMs specifies the time to wait
	//			0		no wait, return immediately after it reads what it can
	//			-1		wait forever till count is satisfied
	//			x		wait x milli-seconds, or till buffer is full
	//
	//	returns the number of bytes written
	//	-1 if an error is encountered (timeout)
	//
	bool Write(byte *ptr, int count, int waitMs = -1);


	//////////////////////////////////////////////////////////////////////////////
	//	bool Available();
	//
	//	returns true if data is available in the ring buffer
	//	returns false for no data
	//
	bool Available();

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
	int ParseInt(uint8 radix = 10, int waitMs = -1);

	//////////////////////////////////////////////////////////////////////////////
	//	void Flush();
	//
	//	empties the buffer
	//
	void Flush();

	static Queue TxBuf = Queue(256);
	static Queue RxBuf = Queue(256);

private:
	UART_HandleTypeDef m_uart;

};

#endif /* WIFIESP_UTILITY_SERIAL_H_ */
