energyShield 2 Library
============

The energyShield 2 library provides software functions that allow the user to easily 
access all of the energyShield's advanced functionality. This functionality comes 
from the energyShield's integrated battery fuel gauge, real-time clock, solar input 
regulation, and voltage measurement ability. 

**Buy the energyShield 2 at NightShade Electronics' online store: https://ns-electric.com/shop/**

Written by Aaron Liebold for NightShade Electronics.

Copyright 2017 NightShade Electronics

Distributed under the MIT License

	Permission is hereby granted, free of charge, to any person obtaining a copy 
	of this software and associated documentation files (the "Software"), to deal 
	in the Software without restriction, including without limitation the rights 
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
	of the Software, and to permit persons to whom the Software is furnished to do so, 
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all 
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
	PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
	FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
	OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
	DEALINGS IN THE SOFTWARE.


Classes

	NS_energyShield2()

Methods

	begin()
	batteryVoltage()
	batteryCurrent()
	SOC()
	fullChargeCapacity()
	remainingCapacity()
	batteryAlert()
	temperature()
	setVMPP()
	readVMPP()
	inputVoltage()
	setTimeAndDate()
	readClock()
	second()
	minute()
	hour()
	dayOfMonth()
	dayOfWeek()
	month()
	year()
	sleepSeconds()

Legacy Methods (for compatibility with origninal energyShield code)

	voltage()
	current()
	percent()
	Vadp()
