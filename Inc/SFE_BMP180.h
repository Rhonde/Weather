/*
	SFE_BMP180.h
	Bosch BMP180 pressure sensor library for the Arduino microcontroller
	Mike Grusin, SparkFun Electronics

	Uses floating-point equations from the Weather Station Data Logger project
	http://wmrx00.sourceforge.net/
	http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf

	Forked from BMP085 library by M.Grusin

	version 1.0 2013/09/20 initial version
	Verison 1.1.2 - Updated for Arduino 1.6.4 5/2015
	
	Our example code uses the "beerware" license. You can do anything
	you like with this code. No really, anything. If you find it useful,
	buy me a (root) beer someday.
*/

#ifndef SFE_BMP180_h
#define SFE_BMP180_h

#include "main.h"

bool BMP_Begin(I2C_HandleTypeDef *pI2C);
// call BMP_begin() to initialize BMP180 before use
// returns 1 if success, 0 if failure (bad component or I2C bus shorted?)

int8_t BMP_StartTemperature(I2C_HandleTypeDef *pI2C);
// command BMP180 to start a temperature measurement
// returns (number of mS to wait) for success, 0 for fail

bool BMP_GetTemperature(I2C_HandleTypeDef *pI2C, double *T);
// return temperature measurement from previous startTemperature command
// places returned value in T variable (deg C)
// returns 1 for success, 0 for fail

bool BMP_StartPressure(I2C_HandleTypeDef *pI2C, char oversampling);
// command BMP180 to start a pressure measurement
// over-sampling: 0 - 3 for over-sampling value
// returns (number of ms to wait) for success, 0 for fail

bool BMP_GetPressure(I2C_HandleTypeDef *pI2C, double *P, double *T);
// return absolute pressure measurement from previous startPressure command
// note: requires previous temperature measurement in variable T
// places returned value in P variable (mBar)
// returns 1 for success, 0 for fail

double BMP_Sealevel(I2C_HandleTypeDef *pI2C, double P, double A);
// convert absolute pressure to sea-level pressure (as used in weather data)
// P: absolute pressure (milliBar)
// A: current altitude (meters)
// returns sea level pressure in mBar

double BMP_Altitude(I2C_HandleTypeDef *pI2C, double P, double P0);
// convert absolute pressure to altitude (given baseline pressure; sea-level, runway, etc.)
// P: absolute pressure (mBar)
// P0: fixed baseline pressure (mBar)
// returns signed altitude in meters

HAL_StatusTypeDef BMP_GetError(I2C_HandleTypeDef *pI2C);
// If any library command fails, you can retrieve an extended
// error code using this command. Errors are from the HAL I2C library:
// 		HAL_OK       = 0x00,
// 		HAL_ERROR    = 0x01,
// 		HAL_BUSY     = 0x02,
// 		HAL_TIMEOUT  = 0x03

#define BMP180_ADDR 0x77 // 7-bit address

#define	BMP180_REG_CONTROL 0xF4
#define	BMP180_REG_RESULT 0xF6

#define	BMP180_COMMAND_TEMPERATURE 0x2E
#define	BMP180_COMMAND_PRESSURE0 0x34
#define	BMP180_COMMAND_PRESSURE1 0x74
#define	BMP180_COMMAND_PRESSURE2 0xB4
#define	BMP180_COMMAND_PRESSURE3 0xF4

#endif
