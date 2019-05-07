/*
 * Queue.h
 *
 *  Created on: May 3, 2019
 *      Author: Archer
 */

#ifndef WIFIESP_UTILITY_QUEUE_H_
#define WIFIESP_UTILITY_QUEUE_H_

#include <ctype.h>
#include <stdio.h>
#include <stm32l4xx_hal.h>

enum enumErrFlag {
		errNone = 0,
		errParity 	= (1<<0),
		errFrame 	= (1<<1),
		errNoise	= (1<<2),
		errOverrun	= (1<<3)
};

#define SIZE 64

class Queue
{

	public:
		Queue(int size = SIZE);		// constructor
		virtual ~Queue();
		uint8_t Pop(void);				// gets a uint8_t from the queue
		bool 	Push(uint8_t val);		// adds a uint8_t to the queue
		bool 	Peek(uint8_t *val);
		int 	Size();
		bool 	IsEmpty();
		bool 	IsFull();
		void 	Init();

		void	SetError(enumErrFlag error) {m_error = error;};
		enumErrFlag	GetError(void) { return m_error; };

	private:
		uint8_t 	*m_arr;   		// array to store queue elements
		int 	m_capacity;   	// maximum capacity of the queue
		int		m_front;  		// front points to front element in the queue (if any)
		int 	m_rear;   		// rear points to last element in the queue
		int 	m_count;  		// current size of the queue
		enumErrFlag	m_error;
};

#endif /* WIFIESP_UTILITY_QUEUE_H_ */
