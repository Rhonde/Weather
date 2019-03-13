/*****************************************
  NS_eS2_Utilies.h

  Created by Aaron D. Liebold
  on January 30, 2017

  Distributed under the MIT license
  Copyright 2017 NightShade Electronics
  https://opensource.org/licenses/MIT
  
*****************************************/

#ifndef NS_ES2_UTILITIES_H
#define NS_ES2_UTILITIES_H

#include "main.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;


void 		ES2_writeByte(uint8_t slaveAddress, uint8_t registerAddress, uint8_t data);
uint8_t 	ES2_readByte(uint8_t slaveAddress, uint8_t registerAddress);

void 		ES2_writeCommand(uint8_t slaveAddress, uint8_t cmdByte1, uint16_t dataWord);
uint16_t 	ES2_readCommand(uint8_t slaveAddress, uint8_t cmdByte1);

uint16_t 	ES2_readSubCommand(uint8_t slaveAddress, uint16_t controlData);

int 		ES2_setupFuelGauge(uint8_t slaveAddress, uint16_t newDesignCapacity_mAh, uint16_t newTerminationVoltage_mV, uint16_t chargeTerminationCurrent_mA, uint8_t alarmSOC);
int 		ES2_checkIfSealed(uint8_t slaveAddress);

uint8_t 	decodeBCD(uint8_t BCD);
uint8_t 	encodeBCD(uint8_t value);


#endif
