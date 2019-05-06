/*
 * Queue.cpp
 *
 *  Created on: May 3, 2019
 *      Author: Archer
 */

#include "Queue.h"


// define default capacity of the Queue
#define SIZE 16

// Constructor to initialize Queue
Queue::Queue(int size)
{
	m_arr = new uint8_t[size];
	m_capacity = size;
	m_front = 0;
	m_rear = -1;
	m_count = 0;
}

void Queue::Init()
{
	m_front = 0;
	m_rear = -1;
	m_count = 0;
	m_error = 0;
}
// Utility function to remove front element from the Queue
bool Queue::Pop(uint8_t *value)
{

	// check for Queue underflow
	if (IsEmpty())
		return false;

	// post decrement
	*value = m_arr[front];
	m_front = (m_front + 1) % m_capacity;
	m_count--;
	return true;
}

// Utility function to add an item to the Queue
bool Queue::Push(uint8_t item)
{
	// check for Queue overflow
	if (IsFull())
		return false;

	// pre increment
	m_rear = (m_rear + 1) % m_capacity;
	m_arr[rear] = item;
	m_count++;
	return true;
}

// Utility function to return front element in the Queue
bool Queue::Peek(uint8_t *value)
{
	if (isEmpty())
		return false;

	*value = m_arr[m_front];

	return true;
}

// Utility function to return the size of the Queue
int Queue::Size()
{
	return m_count;
}

// Utility function to check if the Queue is empty or not
bool Queue::IsEmpty()
{
	return (m_count == 0);
}

// Utility function to check if the Queue is full or not
bool Queue::IsFull()
{
	return (m_count == m_capacity);
}


Queue::~Queue()
{
	free m_arr;
}

