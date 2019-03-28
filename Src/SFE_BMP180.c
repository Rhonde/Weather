/*
	SFE_BMP180.cpp
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

#include <stdio.h>
#include <math.h>

#include "SFE_BMP180.h"

// globals for interface
int16_t AC1, AC2, AC3, VB1, VB2, MB, MC, MD;
uint16_t AC4, AC5, AC6;
double c5, c6, mc, md, x0, x1, x2;
double bmpY0, bmpY1, bmpY2;
double bmpP0, bmpP1, bmpP2;
HAL_StatusTypeDef _error;

/***************************************************************************************************************/

bool BMP_ReadBytes(I2C_HandleTypeDef *pI2C, byte registerAddress, byte *values, int8_t length)
// Read an array of bytes from device
// values: external array to hold data. Put starting register in values[0].
// length: number of bytes to read
{
	byte buf[255];

	_error = HAL_I2C_Mem_Read(pI2C, BMP180_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buf, length, 500);

	if (_error == HAL_OK)
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

bool BMP_ReadInt(I2C_HandleTypeDef *pI2C, char registerAddress, int16_t *value)
// Read a signed integer (two bytes) from device
// address: register to start reading (plus subsequent register)
// value: external variable to store data (function modifies value)
{
	uint8_t buf[2];

	_error = HAL_I2C_Mem_Read(pI2C, BMP180_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

	HAL_Delay(1);

	if(_error == HAL_OK)
	{
		*value = (int16_t)((buf[0]<<8) | buf[1]);
		return true;
	}
	return false;
}

/***************************************************************************************************************/

bool BMP_ReadUInt(I2C_HandleTypeDef *pI2C, char registerAddress, uint16_t *value)
// Read an unsigned integer (two bytes) from device
// address: register to start reading (plus subsequent register)
// value: external variable to store data (function modifies value)
{
	uint8_t buf[2];

	_error= HAL_I2C_Mem_Read(pI2C, BMP180_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, buf, 2, 100);

	HAL_Delay(1);

	if(_error == HAL_OK)
	{
		*value = (int16_t)((buf[0]<<8) | buf[1]);
		return true;
	}
	return false;
}

/***************************************************************************************************************/

bool BMP_WriteBytes(I2C_HandleTypeDef *pI2C, char registerAddress, byte *values, uint8_t length)
// Write an array of bytes to device
// registerAddress: register to start writing
// values: external array of data to write. Put starting register in values[0].
// length: number of bytes to write
{
	_error= HAL_I2C_Mem_Write(pI2C, BMP180_ADDR << 1, registerAddress, I2C_MEMADD_SIZE_8BIT, values, length, 500);

	if (_error == HAL_OK)
		return true;
	else
		return false;
}

/***************************************************************************************************************/

bool BMP_Begin(I2C_HandleTypeDef *pI2C)
// Initialize library for subsequent pressure measurements
{
	double c3,c4,b1;

	// The BMP180 includes factory calibration data stored on the device.
	// Each device has different numbers, these must be retrieved and
	// used in the calculations when taking pressure measurements.

	// Retrieve calibration data from device:
	
	if (BMP_ReadInt(pI2C,  0xAA, &AC1) &&
		BMP_ReadInt(pI2C,  0xAC, &AC2) &&
		BMP_ReadInt(pI2C,  0xAE, &AC3) &&
		BMP_ReadUInt(pI2C, 0xB0, &AC4) &&
		BMP_ReadUInt(pI2C, 0xB2, &AC5) &&
		BMP_ReadUInt(pI2C, 0xB4, &AC6) &&
		BMP_ReadInt(pI2C,  0xB6, &VB1) &&
		BMP_ReadInt(pI2C,  0xB8, &VB2) &&
		BMP_ReadInt(pI2C,  0xBA, &MB) &&
		BMP_ReadInt(pI2C,  0xBC, &MC) &&
		BMP_ReadInt(pI2C,  0xBE, &MD))
	{

		// All reads completed successfully!

		// If you need to check your math using known numbers,
		// you can uncomment one of these examples.
		// (The correct results are commented in the below functions.)

		// Example from Bosch datasheet
		// AC1 = 408; AC2 = -72; AC3 = -14383; AC4 = 32741; AC5 = 32757; AC6 = 23153;
		// B1 = 6190; B2 = 4; MB = -32768; MC = -8711; MD = 2868;

		// Example from http://wmrx00.sourceforge.net/Arduino/BMP180-Calcs.pdf
		// AC1 = 7911; AC2 = -934; AC3 = -14306; AC4 = 31567; AC5 = 25671; AC6 = 18974;
		// VB1 = 5498; VB2 = 46; MB = -32768; MC = -11075; MD = 2432;

		printf("BMP180: AC1=%d \n", AC1);
		printf("BMP180: AC2=%d \n", AC2);
		printf("BMP180: AC3=%d \n", AC3);
		printf("BMP180: AC4=%d \n", AC4);
		printf("BMP180: AC5=%d \n", AC5);
		printf("BMP180: AC6=%d \n", AC6);
		printf("BMP180: VB1=%d \n", VB1);
		printf("BMP180: VB2=%d \n", VB2);
		printf("BMP180: MB=%d \n", MB);
		printf("BMP180: MC=%d \n", MC);
		printf("BMP180: MD=%d \n", MD);
		
		// Compute floating-point polynominals:

		c3 = 160.0 * pow(2,-15) * AC3;
		c4 = pow(10,-3) * pow(2,-15) * AC4;
		b1 = pow(160,2) * pow(2,-30) * VB1;
		c5 = (pow(2,-15) / 160) * AC5;
		c6 = AC6;
		mc = (pow(2,11) / pow(160,2)) * MC;
		md = MD / 160.0;
		x0 = AC1;
		x1 = 160.0 * pow(2,-13) * AC2;
		x2 = pow(160,2) * pow(2,-25) * VB2;
		bmpY0 = c4 * pow(2,15);
		bmpY1 = c4 * c3;
		bmpY2 = c4 * b1;
		bmpP0 = (3791.0 - 8.0) / 1600.0;
		bmpP1 = 1.0 - 7357.0 * pow(2,-20);
		bmpP2 = 3038.0 * 100.0 * pow(2,-36);

		printf("BMP180: c3=%f\n", c3);
		printf("BMP180: c4=%f\n", c4);
		printf("BMP180: c5=%f\n", c5);
		printf("BMP180: c6=%f\n", c6);
		printf("BMP180: b1=%f\n", b1);
		printf("BMP180: mc=%f\n", mc);
		printf("BMP180: md=%f\n", md);
		printf("BMP180: x0=%f\n", x0);
		printf("BMP180: x1=%f\n", x1);
		printf("BMP180: x2=%f\n", x2);
		printf("BMP180: bmpY0=%f\n", bmpY0);
		printf("BMP180: bmpY1=%f\n", bmpY1);
		printf("BMP180: bmpY2=%f\n", bmpY2);
		printf("BMP180: bmpP0=%f\n", bmpP0);
		printf("BMP180: bmpP1=%f\n", bmpP1);
		printf("BMP180: bmpP2=%f\n", bmpP2);
		
		// Success!
		return true;
	}
	else
	{
		// Error reading calibration data; bad component or connection?
		return false;
	}
}
/***************************************************************************************************************/
int8_t BMP_StartTemperature(I2C_HandleTypeDef *pI2C)
// Begin a temperature reading.
// Will return delay in ms to wait, or 0 if I2C error
{
	byte data[1];
	bool result;

	data[0] = BMP180_COMMAND_TEMPERATURE;

	result = BMP_WriteBytes(pI2C, BMP180_REG_CONTROL, data, 1);
	
	if (result) // good write?
		return 5; // return the delay in ms (rounded up) to wait before retrieving data
	else
		return 0; // or return 0 if there was a problem communicating with the BMP
}

/***************************************************************************************************************/
bool BMP_GetTemperature(I2C_HandleTypeDef *pI2C, double *temperature)
// Retrieve a previously-started temperature reading.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startTemperature() to have been called prior and sufficient time elapsed.
// T: external variable to hold result.
// Returns 1 if successful, 0 if I2C error.
{
	byte data[2];
	bool result;
	double tu, a;
	

	result = BMP_ReadBytes(pI2C, BMP180_REG_RESULT, data, 2);

	if (result) // good read, calculate temperature
	{
		tu = (data[0] * 256.0) + data[1];

		//example from Bosch datasheet
		//tu = 27898;

		//example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf
		//tu = 0x69EC;
		
		a = c5 * (tu - c6);
		*temperature = a + (mc / (a + md));

		printf("BMP_GetTemperature: tu=%f\n", tu);
		printf("BMP_GetTemperature: a=%f\n", a);
		printf("BMP_GetTemperature: temperature=%f\n", *temperature);
	}

	return(result);
}

/********************
char SFE_BMP180::startPressure(char oversampling)
// Begin a pressure reading.
// Oversampling: 0 to 3, higher numbers are slower, higher-res outputs.
// Will return delay in ms to wait, or 0 if I2C error.
{
	unsigned char data[2], result, delay;
	
	data[0] = BMP180_REG_CONTROL;

	switch (oversampling)
	{
		case 0:
			data[1] = BMP180_COMMAND_PRESSURE0;
			delay = 5;
		break;
		case 1:
			data[1] = BMP180_COMMAND_PRESSURE1;
			delay = 8;
		break;
		case 2:
			data[1] = BMP180_COMMAND_PRESSURE2;
			delay = 14;
		break;
		case 3:
			data[1] = BMP180_COMMAND_PRESSURE3;
			delay = 26;
		break;
		default:
			data[1] = BMP180_COMMAND_PRESSURE0;
			delay = 5;
		break;
	}
	result = writeBytes(data, 2);
	if (result) // good write?
		return(delay); // return the delay in ms (rounded up) to wait before retrieving data
	else
		return(0); // or return 0 if there was a problem communicating with the BMP
}


char SFE_BMP180::getPressure(double &P, double &T)
// Retrieve a previously started pressure reading, calculate abolute pressure in mbars.
// Requires begin() to be called once prior to retrieve calibration parameters.
// Requires startPressure() to have been called prior and sufficient time elapsed.
// Requires recent temperature reading to accurately calculate pressure.

// P: external variable to hold pressure.
// T: previously-calculated temperature.
// Returns 1 for success, 0 for I2C error.

// Note that calculated pressure value is absolute mbars, to compensate for altitude call sealevel().
{
	unsigned char data[3];
	char result;
	double pu,s,x,y,z;
	
	data[0] = BMP180_REG_RESULT;

	result = readBytes(data, 3);
	if (result) // good read, calculate pressure
	{
		pu = (data[0] * 256.0) + data[1] + (data[2]/256.0);

		//example from Bosch datasheet
		//pu = 23843;

		//example from http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf, pu = 0x982FC0;	
		//pu = (0x98 * 256.0) + 0x2F + (0xC0/256.0);
		
		s = T - 25.0;
		x = (x2 * pow(s,2)) + (x1 * s) + x0;
		y = (bmpY2 * pow(s,2)) + (bmpY1 * s) + bmpY0;
		z = (pu - x) / y;
		P = (bmpP2 * pow(z,2)) + (bmpP1 * z) + bmpP0;

		Serial.println();
		printf("BMP180: pu: ", pu);
		printf("BMP180: T: ", *T);
		printf("BMP180: s: ", s);
		printf("BMP180: x: ", x);
		printf("BMP180: y: ", y);
		printf("BMP180: z: ", z);
		printf("BMP180: P: ", *P);
	}
	return(result);
}


double SFE_BMP180::sealevel(double P, double A)
// Given a pressure P (mb) taken at a specific altitude (meters),
// return the equivalent pressure (mb) at sea level.
// This produces pressure readings that can be used for weather measurements.
{
	return(P/pow(1-(A/44330.0),5.255));
}


double SFE_BMP180::altitude(double P, double P0)
// Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
// return altitude (meters) above baseline.
{
	return(44330.0*(1-pow(P/P0,1/5.255)));
}


char SFE_BMP180::getError(void)
	// If any library command fails, you can retrieve an extended
	// error code using this command. Errors are from the wire library: 
	// 0 = Success
	// 1 = Data too long to fit in transmit buffer
	// 2 = Received NACK on transmit of address
	// 3 = Received NACK on transmit of data
	// 4 = Other error
{
	return(_error);
}
***********************************************************/
