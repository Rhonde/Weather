/*****************************************
 NS_energyShield2.cpp

 Created by Aaron D. Liebold
 on January 30, 2017

 Distributed under the MIT license
 Copyright 2017 NightShade Electronics
 https://opensource.org/licenses/MIT

 *****************************************/

//#include "Arduino.h"
#include "NS_energyShield2.h"
//#include "Wire.h"
//#include "NS_eS2_Utilities.h"

// Set RTC time and date
void ES2_setTimeDate(uint8_t second, uint8_t minute, uint8_t hour,
		uint8_t dayOfMonth, uint8_t dayOfWeek, uint8_t month, uint8_t year)
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
	HAL_Delay(15);
	HAL_I2C_Mem_Write(&hi2c1, RTC_SLAVE_ADDR << 1, 0x04, I2C_MEMADD_SIZE_8BIT,
			timeDate, 7, 100);
	//device address 0x40 in the data sheet is shifted 1-bit to the left.
	//7 bytes are transmitting to the slave RTC

	return;
}

// Read current time and date into a local buffer
void ES2_readClock()
{
	int i = 0;

	// Read time and date
	HAL_Delay(20);

	// read from register 4 to 0xA from RTC
	HAL_I2C_Mem_Read(&hi2c1, RTC_SLAVE_ADDR << 1, 0x04, I2C_MEMADD_SIZE_8BIT,
			_timeDate, 7, 100);

	// Convert seconds, minutes, hours, day-of-the-month, and year from BCD to binary (skipping day-of-the-week)
	for (i = 0; i < 7; i++)
	{
		if (i != 4)
			_timeDate[i] = decodeBCD(_timeDate[i]);
	}

	return;
}

// Returns currrent second(0-59)
uint8_t ES2_second()
{
	return _timeDate[0];
}

// Returns currrent minute (0-59)
uint8_t ES2_minute()
{
	return _timeDate[1];
}

// Returns currrent hour (0-23)
uint8_t ES2_hour()
{
	return _timeDate[2];
}

// Returns currrent day of the month (1-31)
uint8_t ES2_dayOfMonth()
{
	return _timeDate[3];
}

// Returns currrent day of the week (1-7)
uint8_t ES2_dayOfWeek()
{
	return _timeDate[4];
}

// Returns currrent month (1-12)
uint8_t ES2_month()
{
	return _timeDate[5];
}

// Returns currrent year (00-99)
uint8_t ES2_year()
{
	return _timeDate[6];
}

// Clears any active RTC alarms
void ES2_clearAlarms()
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

void ES2_writeAlarms(long alarmTimeSeconds)
{
	uint8_t secondAlarm, minuteAlarm, hourAlarm, dayAlarm, chksum = 0, received;

	do
	{
		ES2_clearAlarms(); // Clear all active alarms

		secondAlarm = encodeBCD((second() + alarmTimeSeconds) % 60);
		minuteAlarm =
				alarmTimeSeconds >= 60 ?
						encodeBCD((ES2_minute() + alarmTimeSeconds / 60) % 60) :
						0;
		hourAlarm =
				alarmTimeSeconds >= 3600 ?
						encodeBCD((ES2_hour() + alarmTimeSeconds / 3600) % 24) :
						0;
		dayAlarm =
				alarmTimeSeconds >= 86400 ?
						encodeBCD(
								(ES2_dayOfWeek() + alarmTimeSeconds / 86400)
										% 7) :
						0;

		TWI_writeByte(RTC_SLAVE_ADDR, 0x0B, secondAlarm);
		if (alarmTimeSeconds >= 60)
			TWI_writeByte(RTC_SLAVE_ADDR, 0x0C, minuteAlarm);
		if (alarmTimeSeconds >= 3600)
			TWI_writeByte(RTC_SLAVE_ADDR, 0x0D, hourAlarm);
		if (alarmTimeSeconds >= 86400)
			TWI_writeByte(RTC_SLAVE_ADDR, 0x0F, dayAlarm);

		delay(1);
		Wire.beginTransmission(RTC_SLAVE_ADDR);
		Wire.write(0x0B);
		Wire.endTransmission();
		Wire.requestFrom(RTC_SLAVE_ADDR, 5);
		for (int i = 0; i < 5; ++i)
		{
			received = Wire.read();
			if (received < 128)
				chksum += received;
		}
	} while (chksum != secondAlarm + minuteAlarm + hourAlarm + dayAlarm);
}

// Turns off 5V and 3.3V output for timeInSeconds seconds
void ES2_sleepSeconds(long timeInSeconds)
{
	ES2_readClock(); // Get current time

	// Write alarm registers
	ES2_writeAlarms(timeInSeconds);

	// Sleep
	TWI_writeByte(RTC_SLAVE_ADDR, 0x01, B10000111);

	// If not sleeping...
	while (1)
	{
		delay(100);

		// Get control of EN net
		ES2_clearAlarms();
		TWI_writeByte(RTC_SLAVE_ADDR, 0x0E, encodeBCD(dayOfMonth()));
		delay(1500);

		// Write alarm registers
		ES2_writeAlarms(timeInSeconds);

		// Sleep
		TWI_writeByte(RTC_SLAVE_ADDR, 0x01, B10000111);
	}
}

// Read the current VMPP setting from DAC
int ES2_readVMPP()
{
	int voltage, data[2];
	do
	{
		Wire.requestFrom(DAC_SLAVE_ADDR, 2);
		data[0] = Wire.read();
		data[1] = Wire.read();
		Wire.endTransmission();
	} while (!(data[0] & 0b10000000));

	if (data[0] & 0b00000110)
	{
		return -1;
	}
	else
	{
		return ((unsigned long) (357 - data[1]) << 16) / 984;
	}
}

// Set regulated MPP voltage of solar panel and writes to EEPROM
void ES2_setVMPP(int MPP_Voltage_mV, bool writeEEPROM)
{
	uint8_t DAC_setting, Control, Hbyte, Lbyte, data[2];

	do
	{
		Wire.requestFrom(DAC_SLAVE_ADDR, 2);
		data[0] = Wire.read();
		data[1] = Wire.read();
		Wire.endTransmission();
	} while (!(data[0] & 0b10000000));

	// Check to see if regulation is already disabled
	if ((MPP_Voltage_mV <= 0) && (data[0] & 0b00000110))
		return;

	// Calculates the required DAC voltage to bias the feedback 
	DAC_setting = 357 - ((unsigned long) 984 * MPP_Voltage_mV >> 16);

	if (data[1] != DAC_setting)
	{
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
uint16_t ES2_batteryVoltage()
{
	uint16_t voltage = readCommand(FG_SLAVE_ADDR, 0x04);
	return voltage;
}

// Returns 1 second average of current from fuel gauge in mA
int16_t ES2_batteryCurrent()
{
	int16_t current = readCommand(FG_SLAVE_ADDR, 0x10);
	return current;
}

// Returns temperature from fuel gauge in tenths of a degree Celsius (0.1 C)
int16_t ES2_temperature()
{
	int16_t temp = readCommand(FG_SLAVE_ADDR, 0x02);
	temp -= 2732;
	return temp;
}

// Returns state-of-charge from fuel gauge in percent of full charge (1%)
uint16_t ES2_SOC()
{
	uint16_t SOC = readCommand(FG_SLAVE_ADDR, 0x1C);
	return SOC;
}

// Returns full-charge capacity from fuel gauge in mAh
uint16_t ES2_fullChargeCapacity()
{
	uint16_t fullChargeCapacity = readCommand(FG_SLAVE_ADDR, 0x0E);
	return fullChargeCapacity;
}

// Returns remaining capacity from fuel gauge in mAh
uint16_t ES2_remainingCapacity()
{
	uint16_t remainingCapacity = readCommand(FG_SLAVE_ADDR, 0x0C);
	return remainingCapacity;
}

// Sets GPOUT pin to BAT Low indication
int ES2_batteryAlert(uint8_t alarmSOC)
{
	uint16_t flags, oldOpConfig, newOpConfig, oldSOC1Set, oldSOC1Clear;
	uint8_t oldCheckSum, tempCheckSum, newCheckSum, checkSum, i;

	i = 0;
	do
	{
		++i;
		if (i > 2)
			return 42; // Failed
		// Unseal
		writeCommand(FG_SLAVE_ADDR, 0x00, 0x8000);
		writeCommand(FG_SLAVE_ADDR, 0x00, 0x8000);
		delay(100);
		for (int x = 0; x < 100; ++x)
			if (!checkIfSealed(FG_SLAVE_ADDR))
				break;
		delay(5000);
	} while (checkIfSealed(FG_SLAVE_ADDR));

	//Change to CONFIG UPDATE mode
	writeCommand(FG_SLAVE_ADDR, 0x00, 0x0013);
	do
	{
		flags = readCommand(FG_SLAVE_ADDR, 0x06);
	} while (!(flags & 0x0010));

	// Setup Block RAM update for Subclass ID 0x40 (64)
	TWI_writeByte(FG_SLAVE_ADDR, 0x61, 0x00); // Enable block access
	TWI_writeByte(FG_SLAVE_ADDR, 0x3E, 0x40); // Set subclass ID
	TWI_writeByte(FG_SLAVE_ADDR, 0x3F, 0x00);  // Set block offset 0 or 32
	oldCheckSum = TWI_readByte(FG_SLAVE_ADDR, 0x60);

	i = 0;
	do
	{
		++i;
		if (i > 100)
			return 2; // Failed

		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;

		oldOpConfig = readCommand(FG_SLAVE_ADDR, 0x40);

		newOpConfig = oldOpConfig | 0x0004; // Enable BATLOWEN Bit

		// Check if already set correctly
		if (newOpConfig == oldOpConfig)
			break;

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
	TWI_writeByte(FG_SLAVE_ADDR, 0x3F, 0x00);  // Set block offset 0 or 32
	oldCheckSum = TWI_readByte(FG_SLAVE_ADDR, 0x60);

	i = 0;
	do
	{
		++i;
		if (i > 100)
			return 2; // Failed

		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;

		oldSOC1Set = TWI_readByte(FG_SLAVE_ADDR, 0x40);
		oldSOC1Clear = TWI_readByte(FG_SLAVE_ADDR, 0x41);

		// Check if already set correctly
		if (oldSOC1Set == alarmSOC && oldSOC1Clear == alarmSOC)
			break;

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
	do
	{
		flags = readCommand(FG_SLAVE_ADDR, 0x06);
	} while (flags & 0x0010);

	// Seal fuel gauge
	writeCommand(FG_SLAVE_ADDR, 0x00, 0x0020);

	return 0;
}

// Returns solar/adapter input voltage in mV (default pin, A0)
uint16_t ES2_inputVoltage()
{
	unsigned long voltage;

	// Oversample ADC to achieve 12-bit measurement
	for (int i = 0; i < 16; i++)
		voltage += analogRead(0);
	voltage = voltage >> 2;
	voltage = (unsigned long) 25000 * voltage / 4095;

	return voltage;
}

// Returns solar/adapter input voltage in mV
uint16_t ES2_inputVoltage(uint8_t analogChannel)
{
	uint16_t voltage;

	// Oversample ADC to achieve 12-bit measurement
	for (int i = 0; i < 16; i++)
		voltage += analogRead(analogChannel);
	voltage = voltage >> 2;
	voltage = (unsigned long) 25000 * voltage / 4095;

	return voltage;
}

// Set up energyShield 2 for use
int ES2_begin()
{
	int error = 0;

	es_batteryCapacity = BATTERY_CAPACITY;

	// Initialize TWI
	Wire.begin();

	// Setup RTC
	TWI_writeByte(RTC_SLAVE_ADDR, 0x00, B01001001); // Normal Mode, Run (Not Stop), No Reset, No Correction, 24-Hour, 12.5pF
	TWI_writeByte(RTC_SLAVE_ADDR, 0x01, B11000111); // Alarm Interrupt Enabled, Leave Alarm Flag Unchanged, Disable MI, HMI, and TF, No CLKOUT
	ES2_clearAlarms();

	// Setup Fuel Gauge	
	error |= setupFuelGauge(FG_SLAVE_ADDR, _batteryCapacity,
	BATTERY_TERMVOLT_MV, BATTERY_TERMCUR_MA, ALARM_SOC); // Write correct RAM values

	return error;
}

// Added for backwards compatibility
int ES2_voltage()
{
	return (int) ES2_batteryVoltage();
}

// Added for backwards compatibility
int ES2_current()
{
	return (int) ES2_batteryCurrent();
}

// Added for backwards compatibility
int ES2_percent()
{
	return (int) ES2_SOC() << 1;
}

// Added for backwards compatibility
int ES2_Vadp(int pin)
{
	return (int) ES2_inputVoltage(pin);
}
