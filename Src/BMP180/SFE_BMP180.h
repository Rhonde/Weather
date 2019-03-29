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

#include <stdio.h>
#include <stm32l4xx_hal.h>
#include <main.h>

class SFE_BMP180
{
	public:
		SFE_BMP180(I2C_HandleTypeDef *pI2C); // base type

		bool Begin();
			// call pressure.begin() to initialize BMP180 before use
			// returns 1 if success, 0 if failure (bad component or I2C bus shorted?)
		
		byte StartTemperature(void);
			// command BMP180 to start a temperature measurement
			// returns (number of ms to wait) for success, 0 for fail

		bool GetTemperature(float &temperature);
			// return temperature measurement from previous startTemperature command
			// places returned value in T variable (deg C)
			// returns 1 for success, 0 for fail

		byte StartPressure(int oversampling);
			// command BMP180 to start a pressure measurement
			// over sampling: 0 - 3 for oversampling value
			// returns (number of ms to wait) for success, 0 for fail

		bool GetPressure(float &pressure, float temperature);
			// return absolute pressure measurement from previous startPressure command
			// note: requires previous temperature measurement in variable T
			// places returned value in P variable (mbar)
			// returns 1 for success, 0 for fail

		float SeaLevel(float P, float A);
			// convert absolute pressure to sea-level pressure (as used in weather data)
			// P: absolute pressure (mbar)
			// A: current altitude (meters)
			// returns sealevel pressure in mbar

		float Altitude(float P, float P0);
			// convert absolute pressure to altitude (given baseline pressure; sea-level, runway, etc.)
			// P: absolute pressure (mbar)
			// P0: fixed baseline pressure (mbar)
			// returns signed altitude in meters

		HAL_StatusTypeDef GetError(void);
			// If any library command fails, you can retrieve an extended
			// error code using this command. Errors are from the HAL library:
			// 		HAL_OK       = 0x00,
			// 		HAL_ERROR    = 0x01,
			// 		HAL_BUSY     = 0x02,
			// 		HAL_TIMEOUT  = 0x03


	private:
	
		bool ReadInt(byte address, int16_t &value);
			// read an signed int (16 bits) from a BMP180 register
			// address: BMP180 register address
			// value: external signed int for returned value (16 bits)
			// returns 1 for success, 0 for fail, with result in value

		bool ReadUInt(byte address, uint16_t &value);
			// read an unsigned int (16 bits) from a BMP180 register
			// address: BMP180 register address
			// value: external unsigned int for returned value (16 bits)
			// returns 1 for success, 0 for fail, with result in value

		bool ReadBytes(byte regAddr, byte *values, int length);
			// read a number of bytes from a BMP180 register
			// values: array of char with register address in first location [0]
			// length: number of bytes to read back
			// returns 1 for success, 0 for fail, with read bytes in values[] array
			
		bool WriteBytes(byte regAddr, byte *values, int length);
			// write a number of bytes to a BMP180 register (and consecutive subsequent registers)
			// values: array of char with register address in first location [0]
			// length: number of bytes to write
			// returns 1 for success, 0 for fail
			
		int16_t m_AC1, m_AC2, m_AC3, m_VB1, m_VB2, m_MB, m_MC, m_MD;
		uint16_t m_AC4, m_AC5, m_AC6;
		float m_c5, m_c6, m_mc, m_md;
		float m_x0, m_x1, m_x2;
		float m_y0, m_y1, m_y2;
		float m_p0, m_p1, m_p2;
		HAL_StatusTypeDef m_error;
		I2C_HandleTypeDef *m_pI2C;
};

#define BMP180_ADDR 0x77 // 7-bit address

#define	BMP180_REG_CONTROL 0xF4
#define	BMP180_REG_RESULT 0xF6

#define	BMP180_COMMAND_TEMPERATURE 0x2E
#define	BMP180_COMMAND_PRESSURE0 0x34
#define	BMP180_COMMAND_PRESSURE1 0x74
#define	BMP180_COMMAND_PRESSURE2 0xB4
#define	BMP180_COMMAND_PRESSURE3 0xF4

#endif
