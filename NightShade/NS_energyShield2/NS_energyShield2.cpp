/*****************************************
  NS_energyShield2.cpp

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
  
*****************************************/

#include "Arduino.h"
#include "NS_energyShield2.h"
#include "Wire.h"
#include "NS_eS2_Utilities.h"

// Creates an instance of NS_energyShield2
NS_energyShield2::NS_energyShield2()
{
	_batteryCapacity = BATTERY_CAPACITY;		
}

// Creates an instance of NS_energyShield2 and defines custom battery size
NS_energyShield2::NS_energyShield2(uint16_t batteryCapacity_mAh) {
	_batteryCapacity = batteryCapacity_mAh;	
}


// Set RTC time and date
void NS_energyShield2::setTimeDate(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfMonth, uint8_t dayOfWeek, uint8_t month, uint8_t year)
{
  uint8_t timeDate[7];

  // Load timeDate array
  timeDate[0] = B10000000 | encodeBCD(second);
  timeDate[1] = encodeBCD(minute);
  timeDate[2] = encodeBCD(hour);
  timeDate[3] = encodeBCD(dayOfMonth);
  timeDate[4] = dayOfWeek;
  timeDate[5] = encodeBCD(month);
  timeDate[6] = encodeBCD(year);

  // Program RTC registers
  Wire.beginTransmission(RTC_SLAVE_ADDR);
  Wire.write(0x04);
  for (int i = 0; i < 7; i++)
  {
    Wire.write(timeDate[i]);
  }
  Wire.endTransmission();

  return;
}

// Read current time and date into a local buffer
void NS_energyShield2::readClock()
{
	int i=0;	
	
	// Read time and date
	Wire.beginTransmission(RTC_SLAVE_ADDR);
	Wire.write(0x04);
	Wire.endTransmission(0);
	Wire.requestFrom(RTC_SLAVE_ADDR,7);
	for (i=0;i<7;i++)
	{
		_timeDate[i] = Wire.read();
	}
	
	// Convert seconds, minutes, hours, day-of-the-month, and year from BCD to binary (skipping day-of-the-week)
	for (i=0;i<7;i++)
	{
		if (i != 4) _timeDate[i] = decodeBCD(_timeDate[i]);
	}
	
	return;
}

// Returns currrent second(0-59)
uint8_t NS_energyShield2::second()
{
	return _timeDate[0];
}

// Returns currrent minute (0-59)
uint8_t NS_energyShield2::minute()
{
	return _timeDate[1];
}

// Returns currrent hour (0-23)
uint8_t NS_energyShield2::hour()
{
	return _timeDate[2];
}

// Returns currrent day of the month (1-31)
uint8_t NS_energyShield2::dayOfMonth()
{
	return _timeDate[3];
}

// Returns currrent day of the week (1-7)
uint8_t NS_energyShield2::dayOfWeek()
{
	return _timeDate[4];
}

// Returns currrent month (1-12)
uint8_t NS_energyShield2::month()
{
	return _timeDate[5];
}

// Returns currrent year (00-99)
uint8_t NS_energyShield2::year()
{
	return _timeDate[6];
}

// Clears any active RTC alarms
void NS_energyShield2::clearAlarms()
{
  Wire.beginTransmission(RTC_SLAVE_ADDR);
  Wire.write(0x0B); // Address of first alarm register
  for (int i = 0; i < 5; i++)
  {
    Wire.write(0xFF); // Clear all alarms
  }
  Wire.endTransmission();

  return;
}

void NS_energyShield2::writeAlarms(long alarmTimeSeconds) {
	uint8_t secondAlarm, minuteAlarm, hourAlarm, dayAlarm, chksum = 0, received;

	do {    
		NS_energyShield2::clearAlarms(); // Clear all active alarms
		
		secondAlarm = encodeBCD((second() + alarmTimeSeconds) % 60);
		minuteAlarm = alarmTimeSeconds >= 60 ? encodeBCD((NS_energyShield2::minute() + alarmTimeSeconds/60)%60) : 0;
		hourAlarm = alarmTimeSeconds >= 3600 ? encodeBCD((NS_energyShield2::hour() + alarmTimeSeconds/3600)%24) : 0;
		dayAlarm = alarmTimeSeconds >= 86400 ? encodeBCD((NS_energyShield2::dayOfWeek() + alarmTimeSeconds/86400)%7) : 0;

		TWI_writeByte(RTC_SLAVE_ADDR, 0x0B, secondAlarm);
		if (alarmTimeSeconds >= 60) TWI_writeByte(RTC_SLAVE_ADDR, 0x0C, minuteAlarm);
		if (alarmTimeSeconds >= 3600) TWI_writeByte(RTC_SLAVE_ADDR, 0x0D, hourAlarm);
		if (alarmTimeSeconds >= 86400) TWI_writeByte(RTC_SLAVE_ADDR, 0x0F, dayAlarm);  
		
		delay(1);
		Wire.beginTransmission(RTC_SLAVE_ADDR);
		Wire.write(0x0B);
		Wire.endTransmission();
		Wire.requestFrom(RTC_SLAVE_ADDR, 5);
		for (int i=0;i<5;++i) {
			received = Wire.read();
			if (received < 128) chksum += received;
		}		
	} while (chksum != secondAlarm + minuteAlarm + hourAlarm + dayAlarm);
}

// Turns off 5V and 3.3V output for timeInSeconds seconds
void NS_energyShield2::sleepSeconds(long timeInSeconds)
{	
  NS_energyShield2::readClock(); // Get current time
  
  // Write alarm registers
  NS_energyShield2::writeAlarms(timeInSeconds);
  
  // Sleep
  TWI_writeByte(RTC_SLAVE_ADDR, 0x01, B10000111);

  // If not sleeping...
  while (1) {
    delay(100);

    // Get control of EN net
    NS_energyShield2::clearAlarms();
    TWI_writeByte(RTC_SLAVE_ADDR, 0x0E, encodeBCD(dayOfMonth()));
    delay(1500);

	// Write alarm registers
    NS_energyShield2::writeAlarms(timeInSeconds);
	
    // Sleep
    TWI_writeByte(RTC_SLAVE_ADDR, 0x01, B10000111);
  }
}

// Read the current VMPP setting from DAC
int NS_energyShield2::readVMPP() {
	int voltage, data[2];
	do {
		Wire.requestFrom(DAC_SLAVE_ADDR, 2);
		data[0] = Wire.read();
		data[1] = Wire.read();
		Wire.endTransmission();
	} while (!(data[0] & 0b10000000));
	
	if (data[0] & 0b00000110) {
		return -1;
	}
	else {
		return ((unsigned long) (357 - data[1]) << 16) / 984;
	}	
}

// Set regulated MPP voltage of solar panel and writes to EEPROM
void NS_energyShield2::setVMPP(int MPP_Voltage_mV, bool writeEEPROM)
{
	uint8_t DAC_setting, Control, Hbyte, Lbyte, data[2];
	
	do {
		Wire.requestFrom(DAC_SLAVE_ADDR, 2);
		data[0] = Wire.read();
		data[1] = Wire.read();
		Wire.endTransmission();
	} while (!(data[0] & 0b10000000));
	
	// Check to see if regulation is already disabled
	if ((MPP_Voltage_mV <= 0) && (data[0] & 0b00000110)) return;
	
	// Calculates the required DAC voltage to bias the feedback 
	DAC_setting = 357 - ((unsigned long) 984*MPP_Voltage_mV >> 16); 
			
	if (data[1] != DAC_setting)	{
		if (MPP_Voltage_mV > 0)
		{			
			// Formats data for transmission
			Control = writeEEPROM ? 0x70 : 0x50;
			Hbyte = DAC_setting;
			Lbyte = 0x00;
		}
		else
		{
			// Sets DAC to high impedance, low power state
			Control = writeEEPROM ? 0x76 : 0x56;
			Hbyte = 0x00;
			Lbyte = 0x00;
		}

		// Write value to DAC
		Wire.beginTransmission(DAC_SLAVE_ADDR);
		Wire.write(Control);
		Wire.write(Hbyte);
		Wire.write(Lbyte);
		Wire.endTransmission();
	}
	
	return;	
}

// Returns battery voltage from fuel gauge in mV
uint16_t NS_energyShield2::batteryVoltage() {
	uint16_t voltage = readCommand(FG_SLAVE_ADDR, 0x04);
	return voltage;
}

// Returns 1 second average of current from fuel gauge in mA
int16_t NS_energyShield2::batteryCurrent() {
	int16_t current = readCommand(FG_SLAVE_ADDR, 0x10);
	return current;
}

// Returns temperature from fuel gauge in tenths of a degree Celsius (0.1 C)
int16_t NS_energyShield2::temperature() {
	int16_t temp = readCommand(FG_SLAVE_ADDR, 0x02);
	temp -= 2732;
	return temp;
}

// Returns state-of-charge from fuel gauge in percent of full charge (1%)
uint16_t NS_energyShield2::SOC() {
	uint16_t SOC = readCommand(FG_SLAVE_ADDR, 0x1C);
	return SOC;
}

// Returns full-charge capacity from fuel gauge in mAh
uint16_t NS_energyShield2::fullChargeCapacity() {
	uint16_t fullChargeCapacity = readCommand(FG_SLAVE_ADDR, 0x0E);
	return fullChargeCapacity;
}

// Returns remaining capacity from fuel gauge in mAh
uint16_t NS_energyShield2::remainingCapacity() {
	uint16_t remainingCapacity = readCommand(FG_SLAVE_ADDR, 0x0C);
	return remainingCapacity;
}

// Sets GPOUT pin to BAT Low indication
int NS_energyShield2::batteryAlert(uint8_t alarmSOC) {
	uint16_t flags, oldOpConfig, newOpConfig, oldSOC1Set, oldSOC1Clear;
	uint8_t oldCheckSum, tempCheckSum, newCheckSum, checkSum, i;
	
	i=0;
	do {
		++i;
		if (i > 2) return 42; // Failed		
		// Unseal
		writeCommand(FG_SLAVE_ADDR, 0x00, 0x8000);
		writeCommand(FG_SLAVE_ADDR, 0x00, 0x8000);
		delay(100);
		for (int x=0; x<100; ++x) if (!checkIfSealed(FG_SLAVE_ADDR)) break;
		delay(5000);
	} while (checkIfSealed(FG_SLAVE_ADDR));
	
	//Change to CONFIG UPDATE mode
	writeCommand(FG_SLAVE_ADDR, 0x00, 0x0013);
	do {
	flags = readCommand(FG_SLAVE_ADDR, 0x06);		
	} while (!(flags & 0x0010));
	
	// Setup Block RAM update for Subclass ID 0x40 (64)
	TWI_writeByte(FG_SLAVE_ADDR, 0x61, 0x00); // Enable block access
	TWI_writeByte(FG_SLAVE_ADDR, 0x3E, 0x40); // Set subclass ID
	TWI_writeByte(FG_SLAVE_ADDR,0x3F, 0x00);  // Set block offset 0 or 32		
	oldCheckSum = TWI_readByte(FG_SLAVE_ADDR, 0x60);
	
	i = 0;
	do {		
		++i;
		if (i > 100) return 2; // Failed
		
		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;
		
		oldOpConfig = readCommand(FG_SLAVE_ADDR,0x40);
		
		newOpConfig = oldOpConfig | 0x0004; // Enable BATLOWEN Bit
		
		// Check if already set correctly
		if (newOpConfig == oldOpConfig) break;
		
		tempCheckSum -= oldOpConfig >> 8;
		tempCheckSum -= oldOpConfig & 0xFF;
		
		TWI_writeByte(FG_SLAVE_ADDR, 0x40, newOpConfig >> 8);
		TWI_writeByte(FG_SLAVE_ADDR, 0x41, newOpConfig & 0xFF);
		
		// Finish computing new checksum		
		tempCheckSum += newOpConfig >> 8;
		tempCheckSum += newOpConfig & 0xFF;
		
		newCheckSum = 0xFF - tempCheckSum;
		TWI_writeByte(FG_SLAVE_ADDR, 0x60, newCheckSum);
		
		// Verify RAM update is complete
		TWI_writeByte(FG_SLAVE_ADDR, 0x3E, 0x40);
		TWI_writeByte(FG_SLAVE_ADDR, 0x3F, 0x00);		
		checkSum = TWI_readByte(FG_SLAVE_ADDR, 0x60);
	
	} while (checkSum != newCheckSum);

	
	// Setup Block RAM update for Subclass ID 0x40 (64)
	TWI_writeByte(FG_SLAVE_ADDR, 0x3E, 0x31); // Set subclass ID
	TWI_writeByte(FG_SLAVE_ADDR,0x3F, 0x00);  // Set block offset 0 or 32	
	oldCheckSum = TWI_readByte(FG_SLAVE_ADDR, 0x60);
	
	i = 0;
	do {		
		++i;
		if (i > 100) return 2; // Failed
		
		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;
		
		oldSOC1Set = TWI_readByte(FG_SLAVE_ADDR,0x40);
		oldSOC1Clear = TWI_readByte(FG_SLAVE_ADDR,0x41);
		
		// Check if already set correctly
		if (oldSOC1Set == alarmSOC && oldSOC1Clear == alarmSOC) break;
		
		tempCheckSum -= oldSOC1Set;
		tempCheckSum -= oldSOC1Clear;
		
		TWI_writeByte(FG_SLAVE_ADDR, 0x40, alarmSOC);
		TWI_writeByte(FG_SLAVE_ADDR, 0x41, alarmSOC);
		
		// Finish computing new checksum		
		tempCheckSum += alarmSOC;
		tempCheckSum += alarmSOC;
		
		newCheckSum = 0xFF - tempCheckSum;
		TWI_writeByte(FG_SLAVE_ADDR, 0x60, newCheckSum);
		
		// Verify RAM update is complete
		TWI_writeByte(FG_SLAVE_ADDR, 0x3E, 0x31);
		TWI_writeByte(FG_SLAVE_ADDR, 0x3F, 0x00);		
		checkSum = TWI_readByte(FG_SLAVE_ADDR, 0x60);
	
	} while (checkSum != newCheckSum);
	
	// Exit CONFIG UPDATE mode
	writeCommand(FG_SLAVE_ADDR, 0x00, 0x0042);
	do {
		flags = readCommand(FG_SLAVE_ADDR, 0x06);		
	} while (flags & 0x0010);	
	
	// Seal fuel gauge
	writeCommand(FG_SLAVE_ADDR, 0x00, 0x0020);	
	
	return 0;
}

// Returns solar/adapter input voltage in mV (default pin, A0)
uint16_t NS_energyShield2::inputVoltage() {
	unsigned long voltage;
	
	// Oversample ADC to achieve 12-bit measurement
	for (int i=0;i<16;i++) voltage += analogRead(0);
	voltage = voltage >> 2;
	voltage = (unsigned long) 25000*voltage/4095;
	
	return voltage;
}

// Returns solar/adapter input voltage in mV
uint16_t NS_energyShield2::inputVoltage(uint8_t analogChannel) {
	uint16_t voltage;
	
	// Oversample ADC to achieve 12-bit measurement
	for (int i=0;i<16;i++) voltage += analogRead(analogChannel);
	voltage = voltage >> 2;
	voltage = (unsigned long) 25000*voltage/4095;
	
	return voltage;
}

// Set up energyShield 2 for use
int NS_energyShield2::begin()
{
	int error=0;
	
	// Initialize TWI
	Wire.begin(); 
	
	// Setup RTC
	TWI_writeByte(RTC_SLAVE_ADDR, 0x00, B01001001); // Normal Mode, Run (Not Stop), No Reset, No Correction, 24-Hour, 12.5pF
	TWI_writeByte(RTC_SLAVE_ADDR, 0x01, B11000111); // Alarm Interrupt Enabled, Leave Alarm Flag Unchanged, Disable MI, HMI, and TF, No CLKOUT
	NS_energyShield2::clearAlarms();		
	
	// Setup Fuel Gauge	
	error |= setupFuelGauge(FG_SLAVE_ADDR, _batteryCapacity, BATTERY_TERMVOLT_MV, BATTERY_TERMCUR_MA, ALARM_SOC); // Write correct RAM values
		
	return error;
}

// Added for backwards compatibility
int NS_energyShield2::voltage() {
	return (int) NS_energyShield2::batteryVoltage();
}

// Added for backwards compatibility
int NS_energyShield2::current() {
	return (int) NS_energyShield2::batteryCurrent();
}

// Added for backwards compatibility
int NS_energyShield2::percent() {
	return (int) NS_energyShield2::SOC() << 1;
}

// Added for backwards compatibility
int NS_energyShield2::Vadp(int pin) {
	return (int) NS_energyShield2::inputVoltage(pin);
}