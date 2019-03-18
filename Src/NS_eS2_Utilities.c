/*****************************************
 NS_eS2_Utilies.cpp

 Created by Aaron D. Liebold
 on January 30, 2017

 Distributed under the MIT license
 Copyright 2017 NightShade Electronics
 https://opensource.org/licenses/MIT

 *****************************************/
#include "NS_eS2_Utilities.h"

#define ES2_DELAY 100

// Write one byte via TWI
void ES2_writeByte(uint8_t slaveAddress, uint8_t registerAddress, uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c3, slaveAddress << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

	HAL_Delay(ES2_DELAY);

	return;
}

// Read one byte via TWI
uint8_t ES2_readByte(uint8_t slaveAddress, uint8_t registerAddress)
{
	uint8_t data;

	HAL_I2C_Mem_Read(&hi2c3, slaveAddress << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
	HAL_Delay(ES2_DELAY);

	return data;
}

// Write standard command to BQ fuel gauge (TI)
void ES2_writeCommand(uint8_t slaveAddress, uint8_t registerAddress, uint16_t dataWord)
{
	uint8_t buf[2];

	buf[0] = dataWord & 0xFF;
	buf[1] = dataWord >> 8;

	HAL_I2C_Mem_Write(&hi2c3, slaveAddress << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

	HAL_Delay(ES2_DELAY);

	return;
}

// Read standard command to BQ fuel gauge (TI)
uint16_t ES2_readCommand(uint8_t slaveAddress, uint8_t registerAddress)
{
	uint16_t dataWord;
	uint8_t buf[2];

	HAL_I2C_Mem_Read(&hi2c3, slaveAddress << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

	HAL_Delay(ES2_DELAY);

	dataWord = buf[0];
	dataWord |= buf[1] << 8;

	return dataWord;
}

// Read Control() subcommand to BQ fuel gauge (TI)
uint16_t ES2_readSubCommand(uint8_t slaveAddress, uint16_t controlData)
{
	uint16_t dataWord;
	uint8_t buf[3];

	buf[0] = 0;
	buf[1] = controlData & 0xFF;
	buf[2] = controlData >> 8;

	HAL_I2C_Master_Transmit(&hi2c3, slaveAddress << 1, buf, 3, 100);

	dataWord = ES2_readCommand(slaveAddress, 0x00);

	HAL_Delay(ES2_DELAY);

	return dataWord;
}

// Check if fuel gauge is in "Sealed" state
int ES2_checkIfSealed(uint8_t slaveAddress)
{
	uint16_t flags;

	flags = ES2_readSubCommand(slaveAddress, 0x0000);

	return (flags & 0x2000);
}

int ES2_setupFuelGauge(uint8_t slaveAddress, uint16_t newDesignCapacity_mAh,
		uint16_t newTerminationVoltage_mV, uint16_t chargeTerminationCurrent_mA,
		uint8_t alarmSOC)
{
	uint16_t flags, oldDesignCapacity, oldDesignEnergy, oldTerminationVoltage,
			oldTaperRate, newDesignEnergy, newTerminationRate;
	uint8_t oldCheckSum, tempCheckSum, newCheckSum, checkSum, i;

	uint16_t oldOpConfig, newOpConfig, oldSOC1Set, oldSOC1Clear;

	newDesignEnergy = ((unsigned long) newDesignCapacity_mAh * 37) / 10;
	newTerminationRate = (uint16_t) (newDesignCapacity_mAh * 10)
			/ chargeTerminationCurrent_mA;

	i = 0;
	do
	{
		++i;
		if (i > 100)
			return 1; // Failed
		// Unseal
		ES2_writeCommand(slaveAddress, 0x00, 0x8000);
		ES2_writeCommand(slaveAddress, 0x00, 0x8000);
		HAL_Delay(10);
	} while (ES2_checkIfSealed(slaveAddress));

	//Change to CONFIG UPDATE mode
	ES2_writeCommand(slaveAddress, 0x00, 0x0013);
	do
	{
		flags = ES2_readCommand(slaveAddress, 0x06);
	} while (!(flags & 0x0010));

	// Setup Block RAM update
	ES2_writeByte(slaveAddress, 0x61, 0x00);
	ES2_writeByte(slaveAddress, 0x3E, 0x52);
	ES2_writeByte(slaveAddress, 0x3F, 0x00);

	oldCheckSum = ES2_readByte(slaveAddress, 0x60);

	i = 0;
	do
	{
		++i;
		if (i > 100)
			return 2; // Failed

		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;

		oldDesignCapacity = ES2_readCommand(slaveAddress, 0x4A);
		oldDesignEnergy = ES2_readCommand(slaveAddress, 0x4C);
		oldTerminationVoltage = ES2_readCommand(slaveAddress, 0x50);
		oldTaperRate = ES2_readCommand(slaveAddress, 0x5B);

		tempCheckSum -= oldDesignCapacity >> 8;
		tempCheckSum -= oldDesignCapacity & 0xFF;

		tempCheckSum -= oldDesignEnergy >> 8;
		tempCheckSum -= oldDesignEnergy & 0xFF;

		tempCheckSum -= oldTerminationVoltage >> 8;
		tempCheckSum -= oldTerminationVoltage & 0xFF;

		tempCheckSum -= oldTaperRate >> 8;
		tempCheckSum -= oldTaperRate & 0xFF;

		ES2_writeByte(slaveAddress, 0x4A, newDesignCapacity_mAh >> 8);
		ES2_writeByte(slaveAddress, 0x4B, newDesignCapacity_mAh & 0xFF);

		ES2_writeByte(slaveAddress, 0x4C, newDesignEnergy >> 8);
		ES2_writeByte(slaveAddress, 0x4D, newDesignEnergy & 0xFF);

		ES2_writeByte(slaveAddress, 0x50, newTerminationVoltage_mV >> 8);
		ES2_writeByte(slaveAddress, 0x51, newTerminationVoltage_mV & 0xFF);

		ES2_writeByte(slaveAddress, 0x5B, newTerminationRate >> 8);
		ES2_writeByte(slaveAddress, 0x5C, newTerminationRate & 0xFF);

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
		ES2_writeByte(slaveAddress, 0x60, newCheckSum);

		// Verify RAM update is complete
		ES2_writeByte(slaveAddress, 0x3E, 0x52);
		ES2_writeByte(slaveAddress, 0x3F, 0x00);

		checkSum = ES2_readByte(slaveAddress, 0x60);

	} while (checkSum != newCheckSum);

	// Setup Block RAM update for Subclass ID 0x40 (64)
	ES2_writeByte(slaveAddress, 0x61, 0x00); // Enable block access
	ES2_writeByte(slaveAddress, 0x3E, 0x40); // Set subclass ID
	ES2_writeByte(slaveAddress, 0x3F, 0x00);  // Set block offset 0 or 32
	oldCheckSum = ES2_readByte(slaveAddress, 0x60);

	i = 0;
	do
	{
		++i;
		if (i > 100)
			return 2; // Failed

		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;

		oldOpConfig = ES2_readCommand(slaveAddress, 0x40);

		newOpConfig = oldOpConfig | 0x0004; // Enable BATLOWEN Bit

		// Check if already set correctly
		if (newOpConfig == oldOpConfig)
			break;

		tempCheckSum -= oldOpConfig >> 8;
		tempCheckSum -= oldOpConfig & 0xFF;

		ES2_writeByte(slaveAddress, 0x40, newOpConfig >> 8);
		ES2_writeByte(slaveAddress, 0x41, newOpConfig & 0xFF);

		// Finish computing new checksum		
		tempCheckSum += newOpConfig >> 8;
		tempCheckSum += newOpConfig & 0xFF;

		newCheckSum = 0xFF - tempCheckSum;
		ES2_writeByte(slaveAddress, 0x60, newCheckSum);

		// Verify RAM update is complete
		ES2_writeByte(slaveAddress, 0x3E, 0x40);
		ES2_writeByte(slaveAddress, 0x3F, 0x00);
		checkSum = ES2_readByte(slaveAddress, 0x60);

	} while (checkSum != newCheckSum);

	// Setup Block RAM update for Subclass ID 0x40 (64)
	ES2_writeByte(slaveAddress, 0x3E, 0x31); // Set subclass ID
	ES2_writeByte(slaveAddress, 0x3F, 0x00);  // Set block offset 0 or 32
	oldCheckSum = ES2_readByte(slaveAddress, 0x60);

	i = 0;
	do
	{
		++i;
		if (i > 100)
			return 2; // Failed

		// Compute new checksum
		tempCheckSum = 0xFF - oldCheckSum;

		oldSOC1Set = ES2_readByte(slaveAddress, 0x40);
		oldSOC1Clear = ES2_readByte(slaveAddress, 0x41);

		// Check if already set correctly
		if (oldSOC1Set == alarmSOC && oldSOC1Clear == alarmSOC)
			break;

		tempCheckSum -= oldSOC1Set;
		tempCheckSum -= oldSOC1Clear;

		ES2_writeByte(slaveAddress, 0x40, alarmSOC);
		ES2_writeByte(slaveAddress, 0x41, alarmSOC);

		// Finish computing new checksum		
		tempCheckSum += alarmSOC;
		tempCheckSum += alarmSOC;

		newCheckSum = 0xFF - tempCheckSum;
		ES2_writeByte(slaveAddress, 0x60, newCheckSum);

		// Verify RAM update is complete
		ES2_writeByte(slaveAddress, 0x3E, 0x31);
		ES2_writeByte(slaveAddress, 0x3F, 0x00);
		checkSum = ES2_readByte(slaveAddress, 0x60);

	} while (checkSum != newCheckSum);

	// Exit CONFIG UPDATE mode
	ES2_writeCommand(slaveAddress, 0x00, 0x0042);
	do
	{
		flags = ES2_readCommand(slaveAddress, 0x06);
	} while (flags & 0x0010);

	// Seal fuel gauge
	ES2_writeCommand(slaveAddress, 0x00, 0x0020);

	return 0;
}

// Decode tens place, units place formating (BCD)
uint8_t decodeBCD(uint8_t BCD)
{
	uint8_t _value = ((0x70 & BCD) >> 4) * 10 + (0x0F & BCD);

	return _value;
}

// Encode value into BCD format
uint8_t encodeBCD(uint8_t value)
{
	uint8_t _BCD = (value / 10 << 4) | value % 10;
	;

	return _BCD;
}
