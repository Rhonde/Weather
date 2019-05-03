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

class Queue
{

	public:
		Queue(int size = SIZE);		// constructor
		virtual ~Queue();
		bool 	Pop(byte *val);				// gets a byte from the queue
		bool 	Push(byte val);		// adds a byte to the queue
		byte 	Peek(byte *val);
		int 	Size();
		bool 	IsEmpty();
		bool 	IsFull();

	private:
		byte 	*m_arr;   		// array to store queue elements
		int 	m_capacity;   	// maximum capacity of the queue
		int		m_front;  		// front points to front element in the queue (if any)
		int 	m_rear;   		// rear points to last element in the queue
		int 	m_count;  		// current size of the queue
};

#endif /* WIFIESP_UTILITY_QUEUE_H_ */
