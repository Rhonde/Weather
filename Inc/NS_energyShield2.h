/*****************************************
  NS_energyShield2.h

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
  
*****************************************/

//#include <Wire.h>
// #include "Arduino.h"

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

// RTC Functions
void 	ES2_setTimeDate(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfMonth, uint8_t dayOfWeek, uint8_t month, uint8_t year);
void 	ES2_readClock();
uint8_t ES2_second();
uint8_t ES2_minute();
uint8_t ES2_hour();
uint8_t ES2_dayOfMonth();
uint8_t ES2_dayOfWeek();
uint8_t ES2_month();
uint8_t ES2_year();
void 	ES2_clearAlarms();
void	ES2_writeAlarms(long alarmTimeSeconds);
void	ES2_sleepSeconds(long timeInSeconds);
		
// Solar Functions
void 	ES2_setVMPP(int MPP_Voltage_mV, bool writeEEPROM);
int		ES2_readVMPP();
uint16_t ES2_inputVoltage();
uint16_t ES2_inputVoltage(uint8_t pin);
				
// Fuel gauge functions
uint16_t ES2_batteryVoltage();
int16_t	 ES2_batteryCurrent();
int16_t  ES2_temperature();
uint16_t ES2_SOC();
uint16_t ES2_fullChargeCapacity();
uint16_t ES2_remainingCapacity();
int		 ES2_batteryAlert(uint8_t alarmSOC);
		
// Setup function
int 	 ES2_begin();

// Added for compatibility
int		 ES2_voltage();
int		 ES2_current();
int		 ES2_percent();
int		 ES2_Vadp(int pin);

	
extern uint8_t  es_timeDate[7];
extern uint16_t es_batteryCapacity;
};



#endif
