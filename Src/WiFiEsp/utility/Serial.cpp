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
	HAL_UART_MspInit(&uart);
}

Serial::~Serial()
{
	// TODO Auto-generated destructor stub
}

void USART2_IRQHandler(void)
{
	volatile unsigned int ISR;
	byte data;


	ISR = USART2->ISR;								// read interrupt and status register
	if (ISR & USART_ISR_RXNE)
	{                  // read interrupt
		data = (byte)(USART2->RDR & 0xff);				// read the register clears the interrupt
		byte error = (byte)(ISR & 0x0F);				// bottom 4 bit have receive error bits

		if (!Serial::RxBuf.IsFull())
		{
			Serial::RxBuf.Push(data);
			if(error != 0)
				Serial::RxBuf.SetError(error);
		}
		else
			Serial::RxBuf.SetError(errOverrun);

	}
	if (ISR & USART_ISR_TXE)
	{
		if (!Serial::TxBuf.IsEmptr())
		{
			data = Serial::TxBuf.Pop();
			USART2->TDR = (data & 0xFF);
			tx_restart = 0;
		}
		else
		{
			tx_restart = 1;
			USART2->CR1 &= ~USART_FLAG_TXEIE;	// disable TX interrupt if nothing to send

		}
	}
}

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
int Serial::Read(byte *ptr, int  count, int waitMs = -1)
{
	int cnt = 0;
	unsigned int startTick = HAL_GetTick();

	for(cnt = 0; cnt < count; )
	{
		while(Available() && (cnt < count))
		{
			*(ptr + cnt) = RxBuf::Pop();
			cnt++;
		}
		if((cnt < count) && (waitMs < 0))
			continue;
		if((cnt < count) && (waitMs > 0))
		{
			unsigned int curTick = HAL_GetTick();
			if((curTick - startTick) > waitMs)
				break;
		}
	}
	return cnt;
}

//////////////////////////////////////////////////////////////////////////////
// Write(byte *ptr, int  count, int waitMs);
//
// writes to the ring buffer the requested number of bytes,
// if the UART is not busy, it take a 1 to 8 bytes from the front
// and call the HAL write interrupt
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
bool Serial::Write(byte *ptr, int count, int waitMs = -1)
{
	int cnt = 0;
	unsigned int startTick = HAL_GetTick();

	for(cnt = 0; cnt < count; )
	{
		if(!Serial::TxBuf.IsFull())
			Serial::TxBuf.Push(*(ptr + cnt));
	}
	USART2->TDR = Serial::TxBuf.Pop();	// send the data
	USART2->CR1 |= USART_FLAG_TXEIE;	// enable TX interrupt if data to send
}


//////////////////////////////////////////////////////////////////////////////
//	bool Available();
//
//	returns true if data is available in the ring buffer
//	returns false for no data
//
bool Serial::Available()
{
	return !Serial::RxBuf.IsEmpty();
}

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
unsigned int Serial::ParseInt(uint8 radix = 10, int waitMs = -1)
{
	char buf[80];
	byte data;
	unsigned int value = 0;
	int cnt = 0;
	unsigned int startTick = HAL_GetTick();

	// get the character string
	while(cnt < sizeof(buf))
	{
		if(!Serial::RxBuf.IsEmpty())
			data = Serial::RxBuf.Pop();
		if(radix == 16)
		{
			if(isxdigit(data) || (data == 'X') || (data == 'x') || (data == 'h'))
				buf[cnt++] = data;
		}
		else if(radix == 10)
		{
			if(isdigit(data))
				buf[cnt++] = data;
		}
		else if(radix == 2)
		{
			if((data == '0') || (data == '1'))
				buf[cnt++] = data;
		}
	}
	buf[cnt] = 0;

	// now parse it -- remember the string is backwards in the buffer (i.e. MSB is in 0);
	cnt = 0;
	if(radix == 16)
	{
		if((buf[0] == '0') && ((buf[1] == 'x') || (buf[1] == 'X')))
			cnt = 2;
		if(buf[0] == 'h')
			cnt = 1;
		while((cnt < sizeof(buf)) && (buf[cnt] != 0))
		{
			value = value << 4;
			if((buf[cnt] >= 'A') && (buf[cnt] <= 'F'))
				value = value | ((value[cnt] - 'A') + 10);
			else if((buf[cnt] >= 'a') && (buf[cnt] <= 'f'))
				value = value | ((value[cnt] - 'a') + 10);
			else if((buf[cnt] >= '0') && (buf[cnt] <= '9'))
				value = value | (value[cnt] - '0');
		}
	}
	else if(radix == 10)
	{
		while((cnt < sizeof(buf)) && (buf[cnt] != 0))
		{
			value = value * 10;
			if((buf[cnt] >= '0') && (buf[cnt] <= '9'))
				value = value | (value[cnt] - '0');
		}
	}
	else if(radix = 2)
	{
		if(buf[cnt] == 'b')
			cnt = 1;
		while((cnt < sizeof(buf)) && (buf[cnt] != 0))
		{
			value = value << 1;
			if((buf[cnt] == '0') || (buf[cnt] == '1'))
				value = value | ((value[cnt] - '0') & 0x1);
		}
	}

	return value;
}

//////////////////////////////////////////////////////////////////////////////
//	void Flush();
//
//	empties the buffer
//
void Serial::Flush()
{
	Serial::TxBuf.Init();
	Serial::RxBuf.Init();
}

