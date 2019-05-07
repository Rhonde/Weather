/*
 * Serial.cpp
 *
 *  Created on: May 3, 2019
 *      Author: Archer
 */

#include <utility/Serial.h>

Queue *TxBuf = new Queue(256);
Queue *RxBuf = new Queue(256);

Serial::Serial(UART_HandleTypeDef &uart, int txSize, int rxSize)
{
	m_uart = uart;

	// init HAL Uart -- done by STM32Cube code
	HAL_UART_MspInit(&uart);
	TxBuf->Init();
	RxBuf->Init();
}

Serial::~Serial()
{
	// TODO Auto-generated destructor stub
}

void USART2_IRQHandler(void)
{
	volatile unsigned int ISR;
	uint8_t data;


	ISR = USART2->ISR;								// read interrupt and status register
	if (ISR & USART_ISR_RXNE)
	{                  // read interrupt
		data = (uint8_t)(USART2->RDR & 0xff);				// read the register clears the interrupt
		enumErrFlag error = (enumErrFlag)(ISR & 0x0F);				// bottom 4 bit have receive error bits

		if (!RxBuf->IsFull())
		{
			RxBuf->Push(data);
			if(error != 0)
				RxBuf->SetError(error);
		}
		else
			RxBuf->SetError(errOverrun);

	}
	if (ISR & USART_ISR_TXE)
	{
		if (!TxBuf->IsEmpty())
		{
			data = TxBuf->Pop();
			USART2->TDR = (data & 0xFF);
		}
		else
		{
			USART2->CR1 &= ~USART_CR1_TXEIE;	// disable TX interrupt if nothing to send

		}
	}
}

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
int Serial::Read(uint8_t *ptr, int  count, int waitMs)
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
			if((int)(curTick - startTick) > waitMs)
				break;
		}
	}
	return cnt;
}

//////////////////////////////////////////////////////////////////////////////
// Write(uint8_t *ptr, int  count, int waitMs);
//
// writes to the ring buffer the requested number of uint8_ts,
// if the UART is not busy, it take a 1 to 8 uint8_ts from the front
// and call the HAL write interrupt
//
//		ptr		pointer to the data
//		count	the number of uint8_ts to write
// 		waitMs specifies the time to wait
//			0		no wait, return immediately after it writes what it can
//			-1		wait forever till count is satisfied
//			x		wait x milli-seconds, or till buffer is full
//
//	returns the number of uint8_ts written
//	-1 if an error is encountered (timeout)
//
int Serial::Write(uint8_t *ptr, int count, int waitMs)
{
	int cnt = 0;
	unsigned int startTick = HAL_GetTick();

	for(cnt = 0; cnt < count; )
	{
		if(!TxBuf->IsFull())
			TxBuf->Push(*(ptr + cnt++));
		else if (waitMs == 0)
			break;
		else if(waitMs < 0)
			continue;
		else if((int)(startTick - HAL_GetTick()) > waitMs)
			break;
	}
	if(TxBuf->Size() > 0)
	{
		USART2->TDR = TxBuf->Pop();	// send the data
		USART2->CR1 |= USART_CR1_TXEIE;	// enable TX interrupt if data to send
	}
	return cnt;
}


//////////////////////////////////////////////////////////////////////////////
//	bool Available();
//
//	returns number of uint8_t in RX queue
//	returns 0 for no data
//
int  Serial::Available()
{
	return RxBuf->Size();
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
unsigned int Serial::ParseInt(uint8_t radix, int waitMs)
{
	char buf[80];
	uint8_t data;
	unsigned int value = 0;
	int cnt = 0;
	unsigned int startTick = HAL_GetTick();

	// get the character string
	while(cnt < sizeof(buf))
	{
		if(!RxBuf->IsEmpty())
			data = RxBuf->Pop();
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
	TxBuf->Init();
	RxBuf->Init();
}

