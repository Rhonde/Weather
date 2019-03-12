/*****************************************
  NS_eS2_Utilies.cpp

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
  
*****************************************/

#define TWI_DELAY 100

#include "Arduino.h"
#include "Wire.h"

// Write one byte via TWI
void TWI_writeByte(uint8_t slaveAddress, uint8_t registerAddress, uint8_t data)
{
	Wire.beginTransmission(slaveAddress);
	Wire.write(registerAddress);
	Wire.write(data);
	Wire.endTransmission();
	delayMicroseconds(TWI_DELAY);
	
	return;	
}

// Read one byte via TWI
uint8_t TWI_readByte(uint8_t slaveAddress, uint8_t registerAddress)
{
	uint8_t _data;
	Wire.beginTransmission(slaveAddress);
	Wire.write(registerAddress);
	Wire.endTransmission(0); // Send restart
	Wire.requestFrom(slaveAddress, 1u);
	_data = Wire.read();
	delayMicroseconds(TWI_DELAY);
	
	uint8_t dataByte;
	
	return _data;
}

// Write standard command to BQ fuel gauge (TI)
void writeCommand(uint8_t slaveAddress, uint8_t registerAddress, uint16_t dataWord) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(registerAddress);
  Wire.write(dataWord & 0xFF);
  Wire.write(dataWord >> 8);
  Wire.endTransmission(); 

  delayMicroseconds(TWI_DELAY);

  return;
}

// Read standard command to BQ fuel gauge (TI)
uint16_t readCommand(uint8_t slaveAddress, uint8_t registerAddress) {
  uint16_t dataWord;
  
  Wire.beginTransmission(slaveAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(0);  
  Wire.requestFrom(slaveAddress, 2u);
  dataWord = Wire.read();
  dataWord += Wire.read() << 8;

  delayMicroseconds(TWI_DELAY);

  return dataWord;
} 

// Read Control() subcommand to BQ fuel gauge (TI)
uint16_t readSubCommand(uint8_t slaveAddress, uint16_t controlData) {
  uint16_t dataWord;  
  
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x00);
  Wire.write(controlData & 0xFF);
  Wire.write(controlData >> 8);
  Wire.endTransmission();  
  
  dataWord = readCommand(slaveAddress, 0x00);

  delayMicroseconds(TWI_DELAY);

  return dataWord;
} 

// Check if fuel gauge is in "Sealed" state
int checkIfSealed(uint8_t slaveAddress) {
  uint16_t flags;

  flags = readSubCommand(slaveAddress, 0x0000);
  
  return (flags & 0x2000);
}

int setupFuelGauge(uint8_t slaveAddress, uint16_t newDesignCapacity_mAh, uint16_t newTerminationVoltage_mV, uint16_t chargeTerminationCurrent_mA, uint8_t alarmSOC) {
	uint16_t flags, oldDesignCapacity, oldDesignEnergy, oldTerminationVoltage, oldTaperRate, newDesignEnergy, newTerminationRate;
	uint8_t oldCheckSum, tempCheckSum, newCheckSum, checkSum, i;
	
	uint16_t oldOpConfig, newOpConfig, oldSOC1Set, oldSOC1Clear;
		
	newDesignEnergy = ((unsigned long) newDesignCapacity_mAh*37)/10;
	newTerminationRate = (uint16_t) (newDesignCapacity_mAh*10)/chargeTerminationCurrent_mA;
	
	i=0;
	do {
		++i;
		if (i > 100) return 1; // Failed		
		// Unseal
		writeCommand(slaveAddress, 0x00, 0x8000);
		writeCommand(slaveAddress, 0x00, 0x8000);
		delay(10);
	} while (checkIfSealed(slaveAddress));
	
	//Change to CONFIG UPDATE mode
	writeCommand(slaveAddress, 0x00, 0x0013);
	do {
	flags = readCommand(slaveAddress, 0x06);		
	} while (!(flags & 0x0010));
	
	// Setup Block RAM update
	TWI_writeByte(slaveAddress, 0x61, 0x00);
	TWI_writeByte(slaveAddress, 0x3E, 0x52);
	TWI_writeByte(slaveAddress,0x3F, 0x00);
		
	oldCheckSum = TWI_readByte(slaveAddress, 0x60);
	
	i = 0;
	do {		
		++i;
		if (i > 100) return 2; // Failed
		
		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;
		
		oldDesignCapacity = readCommand(slaveAddress,0x4A);
		oldDesignEnergy = readCommand(slaveAddress, 0x4C);
		oldTerminationVoltage = readCommand(slaveAddress,0x50);
		oldTaperRate = readCommand(slaveAddress, 0x5B);
		
		tempCheckSum -= oldDesignCapacity >> 8;
		tempCheckSum -= oldDesignCapacity & 0xFF;
		
		tempCheckSum -= oldDesignEnergy >> 8;
		tempCheckSum -= oldDesignEnergy & 0xFF;
		
		tempCheckSum -= oldTerminationVoltage >> 8;
		tempCheckSum -= oldTerminationVoltage & 0xFF;
		
		tempCheckSum -= oldTaperRate >> 8;
		tempCheckSum -= oldTaperRate & 0xFF;
		
		TWI_writeByte(slaveAddress, 0x4A, newDesignCapacity_mAh >> 8);
		TWI_writeByte(slaveAddress, 0x4B, newDesignCapacity_mAh & 0xFF);
		
		TWI_writeByte(slaveAddress, 0x4C, newDesignEnergy >> 8);
		TWI_writeByte(slaveAddress, 0x4D, newDesignEnergy & 0xFF);
		
		TWI_writeByte(slaveAddress, 0x50, newTerminationVoltage_mV >> 8);
		TWI_writeByte(slaveAddress, 0x51, newTerminationVoltage_mV & 0xFF);
		
		TWI_writeByte(slaveAddress, 0x5B, newTerminationRate >> 8);
		TWI_writeByte(slaveAddress, 0x5C, newTerminationRate & 0xFF);
		
		// Finish computing new checksum		
		tempCheckSum += newDesignCapacity_mAh >> 8;
		tempCheckSum += newDesignCapacity_mAh & 0xFF;
		
		tempCheckSum += newDesignEnergy >> 8;
		tempCheckSum += newDesignEnergy & 0xFF;
		
		tempCheckSum += newTerminationVoltage_mV >> 8;
		tempCheckSum += newTerminationVoltage_mV & 0xFF;
		
		tempCheckSum += newTerminationRate >> 8;
		tempCheckSum += newTerminationRate & 0xFF;
		
		newCheckSum = 0xFF - tempCheckSum;
		TWI_writeByte(slaveAddress, 0x60, newCheckSum);
		
		// Verify RAM update is complete
		TWI_writeByte(slaveAddress, 0x3E, 0x52);
		TWI_writeByte(slaveAddress, 0x3F, 0x00);
		
		checkSum = TWI_readByte(slaveAddress, 0x60);
	
	} while (checkSum != newCheckSum);
	
	// Setup Block RAM update for Subclass ID 0x40 (64)
	TWI_writeByte(slaveAddress, 0x61, 0x00); // Enable block access
	TWI_writeByte(slaveAddress, 0x3E, 0x40); // Set subclass ID
	TWI_writeByte(slaveAddress,0x3F, 0x00);  // Set block offset 0 or 32		
	oldCheckSum = TWI_readByte(slaveAddress, 0x60);
	
	i = 0;
	do {		
		++i;
		if (i > 100) return 2; // Failed
		
		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;
		
		oldOpConfig = readCommand(slaveAddress,0x40);
		
		newOpConfig = oldOpConfig | 0x0004; // Enable BATLOWEN Bit
		
		// Check if already set correctly
		if (newOpConfig == oldOpConfig) break;
		
		tempCheckSum -= oldOpConfig >> 8;
		tempCheckSum -= oldOpConfig & 0xFF;
		
		TWI_writeByte(slaveAddress, 0x40, newOpConfig >> 8);
		TWI_writeByte(slaveAddress, 0x41, newOpConfig & 0xFF);
		
		// Finish computing new checksum		
		tempCheckSum += newOpConfig >> 8;
		tempCheckSum += newOpConfig & 0xFF;
		
		newCheckSum = 0xFF - tempCheckSum;
		TWI_writeByte(slaveAddress, 0x60, newCheckSum);
		
		// Verify RAM update is complete
		TWI_writeByte(slaveAddress, 0x3E, 0x40);
		TWI_writeByte(slaveAddress, 0x3F, 0x00);		
		checkSum = TWI_readByte(slaveAddress, 0x60);
	
	} while (checkSum != newCheckSum);
	
	
	// Setup Block RAM update for Subclass ID 0x40 (64)
	TWI_writeByte(slaveAddress, 0x3E, 0x31); // Set subclass ID
	TWI_writeByte(slaveAddress,0x3F, 0x00);  // Set block offset 0 or 32	
	oldCheckSum = TWI_readByte(slaveAddress, 0x60);
	
	i = 0;
	do {		
		++i;
		if (i > 100) return 2; // Failed
		
		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;
		
		oldSOC1Set = TWI_readByte(slaveAddress,0x40);
		oldSOC1Clear = TWI_readByte(slaveAddress,0x41);
		
		// Check if already set correctly
		if (oldSOC1Set == alarmSOC && oldSOC1Clear == alarmSOC) break;
		
		tempCheckSum -= oldSOC1Set;
		tempCheckSum -= oldSOC1Clear;
		
		TWI_writeByte(slaveAddress, 0x40, alarmSOC);
		TWI_writeByte(slaveAddress, 0x41, alarmSOC);
		
		// Finish computing new checksum		
		tempCheckSum += alarmSOC;
		tempCheckSum += alarmSOC;
		
		newCheckSum = 0xFF - tempCheckSum;
		TWI_writeByte(slaveAddress, 0x60, newCheckSum);
		
		// Verify RAM update is complete
		TWI_writeByte(slaveAddress, 0x3E, 0x31);
		TWI_writeByte(slaveAddress, 0x3F, 0x00);		
		checkSum = TWI_readByte(slaveAddress, 0x60);
	
	} while (checkSum != newCheckSum);
	
	// Exit CONFIG UPDATE mode
	writeCommand(slaveAddress, 0x00, 0x0042);
	do {
		flags = readCommand(slaveAddress, 0x06);		
	} while (flags & 0x0010);	
	
	// Seal fuel gauge
	writeCommand(slaveAddress, 0x00, 0x0020);	
	
	return 0;
}

// Decode tens place, units place formating (BCD)
uint8_t decodeBCD(uint8_t BCD)
{
	uint8_t _value = ((B01110000 & BCD) >> 4) * 10 + (B00001111 & BCD);

	return _value;
}

// Encode value into BCD format
uint8_t encodeBCD(uint8_t value)
{
	uint8_t _BCD = (value / 10 << 4) | value % 10;;

	return _BCD;
}
