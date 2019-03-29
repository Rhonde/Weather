/*
	SFE_BMP180.cpp
	Bosch BMP180 pressure sensor library for the Arduino microcontroller
	Mike Grusin, SparkFun Electronics

	Uses floating-point equations from the Weather Station Data Logger project
	http://wmrx00.sourceforge.net/
	http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf

	Forked from BMP085 library by M.Grusin

	Version 1.0 2013/09/20 initial version
	Version 1.1.2 - Updated for Arduino 1.6.4 5/2015

	Our example code uses the "beerware" license. You can do anything
	you like with this code. No really, anything. If you find it useful,
	buy me a (root) beer someday.
*/

#include <SFE_BMP180.h>
#include <stdio.h>
#include <math.h>


SFE_BMP180::SFE_BMP180(I2C_HandleTypeDef *pI2C)
// Base library Constructor
{
	m_pI2C = pI2C;
}


bool SFE_BMP180::Begin()
// Initialize library for subsequent pressure measurements
{
	float c3, c4, b1;
	byte buf[22];

	// The BMP180 includes factory calibration data stored on the device.
	// Each device has different numbers, these must be retrieved and
	// used in the calculations when taking pressure measurements.

	// Retrieve calibration data from device:
	m_error= HAL_I2C_Mem_Read(m_pI2C, BMP180_ADDR << 1, 0xAA, I2C_MEMADD_SIZE_8BIT, buf, 22, 500);
	
	if(m_error == HAL_OK)
	{
		// All reads completed successfully!

		// If you need to check your math using known numbers,
		// you can un-comment one of these examples.
		// (The correct results are commented in the below functions.)

		// Example from Bosch data sheet
		// AC1 = 408; AC2 = -72; AC3 = -14383; AC4 = 32741; AC5 = 32757; AC6 = 23153;
		// B1 = 6190; B2 = 4; MB = -32768; MC = -8711; MD = 2868;

		// Example from http://wmrx00.sourceforge.net/Arduino/BMP180-Calcs.pdf
		// AC1 = 7911; AC2 = -934; AC3 = -14306; AC4 = 31567; AC5 = 25671; AC6 = 18974;
		// VB1 = 5498; VB2 = 46; MB = -32768; MC = -11075; MD = 2432;

		m_AC1 = (int16_t)((buf[0]<<8)  | buf[1]);		// data from register address 0xAA MSB first
		m_AC2 = (int16_t)((buf[2]<<8)  | buf[3]);
		m_AC3 = (int16_t)((buf[4]<<8)  | buf[5]);
		m_AC4 = (int16_t)((buf[6]<<8)  | buf[7]);
		m_AC5 = (int16_t)((buf[8]<<8)  | buf[9]);
		m_AC6 = (int16_t)((buf[10]<<8) | buf[11]);
		m_VB1 = (int16_t)((buf[12]<<8) | buf[13]);
		m_VB2 = (int16_t)((buf[14]<<8) | buf[15]);
		m_MB  = (int16_t)((buf[16]<<8) | buf[17]);
		m_MC  = (int16_t)((buf[18]<<8) | buf[19]);
		m_MD  = (int16_t)((buf[20]<<8) | buf[21]);

		printf("BMP180: AC1=%d \n", m_AC1);
		printf("BMP180: AC2=%d \n", m_AC2);
		printf("BMP180: AC3=%d \n", m_AC3);
		printf("BMP180: AC4=%d \n", m_AC4);
		printf("BMP180: AC5=%d \n", m_AC5);
		printf("BMP180: AC6=%d \n", m_AC6);
		printf("BMP180: VB1=%d \n", m_VB1);
		printf("BMP180: VB2=%d \n", m_VB2);
		printf("BMP180: MB=%d \n", m_MB);
		printf("BMP180: MC=%d \n", m_MC);
		printf("BMP180: MD=%d \n", m_MD);
		
		// Compute floating-point polynominals:

		c3 = 160.0 * pow(2,-15) * m_AC3;
		c4 = pow(10,-3) * pow(2,-15) * m_AC4;
		b1 = pow(160,2) * pow(2,-30) * m_VB1;
		m_c5 = (pow(2,-15) / 160) * m_AC5;
		m_c6 = m_AC6;
		m_mc = (pow(2,11) / pow(160,2)) * m_MC;
		m_md = m_MD / 160.0;
		m_x0 = m_AC1;
		m_x1 = 160.0 * pow(2,-13) * m_AC2;
		m_x2 = pow(160,2) * pow(2,-25) * m_VB2;
		m_y0 = c4 * pow(2,15);
		m_y1 = c4 * c3;
		m_y2 = c4 * b1;
		m_y0 = (3791.0 - 8.0) / 1600.0;
		m_y1 = 1.0 - 7357.0 * pow(2,-20);
		m_y2 = 3038.0 * 100.0 * pow(2,-36);

		printf("BMP180: c3=%f\n", c3);
		printf("BMP180: c4=%f\n", c4);
		printf("BMP180: m_c5=%f\n", m_c5);
		printf("BMP180: m_c6=%f\n", m_c6);
		printf("BMP180: b1=%f\n", b1);
		printf("BMP180: m_mc=%f\n", m_mc);
		printf("BMP180: m_md=%f\n", m_md);
		printf("BMP180: m_x0=%f\n", m_x0);
		printf("BMP180: m_x1=%f\n", m_x1);
		printf("BMP180: m_x2=%f\n", m_x2);
		printf("BMP180: m_y0=%f\n", m_y0);
		printf("BMP180: m_y1=%f\n", m_y1);
		printf("BMP180: m_y2=%f\n", m_y2);
		printf("BMP180: m_p0=%f\n", m_p0);
		printf("BMP180: m_p1=%f\n", m_p1);
		printf("BMP180: m_p2=%f\n", m_p2);
		
		// Success!
		return true;
	}
	else
	{
		// Error reading calibration data; bad component or connection?
		return false;
	}
}


byte SFE_BMP180::StartTemperature(void)
// Begin a temperature reading.
// Will return delay in ms to wait, or 0 if I2C error
{
	byte data;
	bool result;

	data = BMP180_COMMAND_TEMPERATURE;

	result = WriteBytes(BMP180_REG_CONTROL, &data, 1);
	
	if (result) 	// good write?
		return 5; 	// return the delay in ms (rounded up) to wait before retrieving data
	else
		return 0; 	// or return 0 if there was a problem communicating with the BMP
	
}


bool SFE_BMP180::GetTemperature(float &temperature)
// Retrieve a previously-started temperature reading.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startTemperature() to have been called prior and sufficient time elapsed.
// T: external variable to hold result.
// Returns 1 if successful, 0 if I2C error.
{
	byte data[2];
	bool result;
	float tu, a;
	

	result = ReadBytes(BMP180_REG_RESULT, data, 2);

	if (result) // good read, calculate temperature
	{
		tu = (data[0] * 256.0) + data[1];

		//example from Bosch datasheet
		//tu = 27898;

		//example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf
		//tu = 0x69EC;
		
		a = m_c5 * (tu - m_c6);
		temperature = a + (m_mc / (a + m_md));

		printf("BMP_GetTemperature: tu=%f\n", tu);
		printf("BMP_GetTemperature: a=%f\n", a);
		printf("BMP_GetTemperature: temperature=%f\n", temperature);
	}

	return(result);
}


byte SFE_BMP180::StartPressure(int oversampling)
// Begin a pressure reading.
// Over sampling: 0 to 3, higher numbers are slower, higher-res outputs.
// Will return delay in ms to wait, or 0 if I2C error.
{
	byte data;
	bool result;
	byte delay;
	
	switch (oversampling)
	{
		case 0:
			data = BMP180_COMMAND_PRESSURE0;
			delay = 5;
		break;
		case 1:
			data = BMP180_COMMAND_PRESSURE1;
			delay = 8;
		break;
		case 2:
			data = BMP180_COMMAND_PRESSURE2;
			delay = 14;
		break;
		case 3:
			data = BMP180_COMMAND_PRESSURE3;
			delay = 26;
		break;
		default:
			data = BMP180_COMMAND_PRESSURE0;
			delay = 5;
		break;
	}
	result = WriteBytes(BMP180_REG_CONTROL, &data, 1);

	if (result) // good write?
		return(delay); // return the delay in ms (rounded up) to wait before retrieving data
	else
		return(0); // or return 0 if there was a problem communicating with the BMP
}


bool SFE_BMP180::GetPressure(float &pressure, float temperature)
// Retrieve a previously started pressure reading, calculate absolute pressure in mbars.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startPressure() to have been called prior and sufficient time elapsed.
// Requires recent temperature reading to accurately calculate pressure.

// pressure: external variable to hold pressure.
// temperature: previously-calculated temperature.
// Returns 1 for success, 0 for I2C error.

// Note that calculated pressure value is absolute mbars, to compensate for altitude call SeaLevel().
{
	byte data[3];
	bool result;
	float pu,s,x,y,z;

	result = ReadBytes(BMP180_REG_RESULT, data, 3);
	
	if (result) 	// good read, calculate pressure
	{
		pu = (data[0] * 256.0) + data[1] + (data[2]/256.0);

		//example from Bosch data sheet
		//pu = 23843;

		//example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf, pu = 0x982FC0;	
		//pu = (0x98 * 256.0) + 0x2F + (0xC0/256.0);
		
		s = temperature - 25.0;
		x = (m_x2 * pow(s, 2)) + (m_x1 * s) + m_x0;
		y = (m_y2 * pow(s, 2)) + (m_y1 * s) + m_y0;
		z = (pu - x) / y;
		pressure = (m_p2 * pow(z, 2)) + (m_p1 * z) + m_p0;

		printf("%s(%d): Pressure=%f\n", __FUNCTION__, __LINE__, pressure);
		printf("BMP180: Temp=%f\n", temperature);
		printf("BMP180: s=%f\n", s);
		printf("BMP180: x=%f\n", x);
		printf("BMP180: y=%f\n", y);
		printf("BMP180: z=%f\n", z);
		printf("BMP180: pu=%f\n", pu);
	}
	return(result);
}


float SFE_BMP180::SeaLevel(float basePressure, float altitude)
// Given a pressure P (mb) taken at a specific altitude (meters),
// return the equivalent pressure (mb) at sea level.
// This produces pressure readings that can be used for weather measurements.
{
	return(basePressure/pow(1-(altitude/44330.0),5.255));
}


float SFE_BMP180::Altitude(float pressure, float basePressure)
// Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
// return altitude (meters) above baseline.
{
	return(44330.0*(1-pow(pressure/basePressure,1/5.255)));
}


HAL_StatusTypeDef SFE_BMP180::GetError(void)
// If any library command fails, you can retrieve an extended
// error code using this command. Errors are from the wire library:
// 		HAL_OK       = 0x00,
// 		HAL_ERROR    = 0x01,
// 		HAL_BUSY     = 0x02,
// 		HAL_TIMEOUT  = 0x03
{
	return(m_error);
}

/***************************************************************************************************************/

bool SFE_BMP180::ReadBytes(byte regAddr, byte *values, int length)
// Read an array of bytes from device
// values: external array to hold data. Put starting register in values[0].
// length: number of bytes to read
{
	byte buf[255];

	m_error = HAL_I2C_Mem_Read(m_pI2C, BMP180_ADDR << 1, regAddr, I2C_MEMADD_SIZE_8BIT, buf, length, 500);

	if (m_error == HAL_OK)
	{
		for(int ix = 0; ix < length; ix++)
		{
			values[ix] = buf[ix];
		}
		return true;
	}
	return false;
}

/***************************************************************************************************************/

bool SFE_BMP180::ReadInt(byte regAddr, int16_t &value)
// Read a signed integer (two bytes) from device
// address: register to start reading (plus subsequent register)
// value: external variable to store data (function modifies value)
{
	uint8_t buf[2];

	m_error = HAL_I2C_Mem_Read(m_pI2C, BMP180_ADDR << 1, regAddr, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

	HAL_Delay(1);

	if(m_error == HAL_OK)
	{
		value = (int16_t)((buf[0]<<8) | buf[1]);
		return true;
	}
	return false;
}

/***************************************************************************************************************/

bool SFE_BMP180::ReadUInt(byte registerAddress, uint16_t &value)
// Read an unsigned integer (two bytes) from device
// address: register to start reading (plus subsequent register)
// value: external variable to store data (function modifies value)
{
	uint8_t buf[2];

	m_error= HAL_I2C_Mem_Read(m_pI2C, BMP180_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

	HAL_Delay(1);

	if(m_error == HAL_OK)
	{
		value = (int16_t)((buf[0]<<8) | buf[1]);
		return true;
	}
	return false;
}

/***************************************************************************************************************/

bool SFE_BMP180::WriteBytes(byte regAddr, byte *values, int length)
// Write an array of bytes to device
// registerAddress: register to start writing
// values: external array of data to write. Put starting register in values[0].
// length: number of bytes to write
{
	m_error= HAL_I2C_Mem_Write(m_pI2C, BMP180_ADDR << 1, regAddr, I2C_MEMADD_SIZE_8BIT, values, length, 500);

	if (m_error == HAL_OK)
		return true;
	else
		return false;
}
