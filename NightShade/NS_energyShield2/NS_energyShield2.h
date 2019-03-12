/*****************************************
  NS_energyShield2.h

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
  
*****************************************/

#include <Wire.h>
#include "Arduino.h"

#ifndef NS_ENERGYSHIELD2_H
#define NS_ENERGYSHIELD2_H

// Define RTC TWI slave address
#ifndef RTC_SLAVE_ADDR 
#define RTC_SLAVE_ADDR 0x51
#endif

// Define DAC TWI slave address
#ifndef DAC_SLAVE_ADDR 
#define DAC_SLAVE_ADDR 0x60
#endif

// Define Fuel Gauge TWI slave address
#ifndef FG_SLAVE_ADDR 
#define FG_SLAVE_ADDR 0x55
#endif

// Define capacity of battery in mAh
#ifndef BATTERY_CAPACITY 
#define BATTERY_CAPACITY 1800
#endif

// Define termination voltage of battery in mV
#ifndef BATTERY_TERMVOLT_MV 
#define BATTERY_TERMVOLT_MV 3000
#endif

// Define termination current of battery in mV
#ifndef BATTERY_TERMCUR_MA 
#define BATTERY_TERMCUR_MA 65
#endif

// Define alarm state-of-charge in percent (%)
#ifndef ALARM_SOC 
#define ALARM_SOC 10
#endif

class NS_energyShield2 
{
	public:
				NS_energyShield2();
				NS_energyShield2(uint16_t batteryCapacity_mAh);
		
		// RTC Functions
		void 	setTimeDate(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfMonth, uint8_t dayOfWeek, uint8_t month, uint8_t year);
		void 	readClock();	
		uint8_t second();
		uint8_t minute();	
		uint8_t hour();
		uint8_t dayOfMonth();
		uint8_t dayOfWeek();
		uint8_t month();
		uint8_t year();
		void 	clearAlarms();
		void	writeAlarms(long alarmTimeSeconds);
		void	sleepSeconds(long timeInSeconds);
		
		// Solar Functions
		void 	setVMPP(int MPP_Voltage_mV, bool writeEEPROM);
		int		readVMPP();
		uint16_t inputVoltage();
		uint16_t inputVoltage(uint8_t pin);
				
		// Fuel gauge functions
		uint16_t batteryVoltage();
		int16_t	 batteryCurrent();
		int16_t  temperature();
		uint16_t SOC();
		uint16_t fullChargeCapacity();
		uint16_t remainingCapacity();
		int		 batteryAlert(uint8_t alarmSOC);
		
		// Setup function
		int 	 begin();
		
		// Added for compatibility		
		int		 voltage();
		int		 current();
		int		 percent();
		int		 Vadp(int pin);
		
	
	private:
		uint8_t  _timeDate[7];
		uint16_t _batteryCapacity;	
};



#endif