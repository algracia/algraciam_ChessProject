/*
 * ADXL345Driver.h
 *
 *  Created on: May 26, 2023
 *      Author: algraciam
 */
#include <stm32f4xx.h>
#include "I2CDriver.h"

#ifndef INC_ADXL345DRIVER_H_
#define INC_ADXL345DRIVER_H_

#define ACCEL_ADDRESS_SDO_HIGH		0x1D
#define ACCEL_ADDRESS_SDO_LOW		0x53

#define	ACCEL_ID					0x00
#define ACCEL_BW_RATE				0x2C
#define ACCEL_POWER_CTL				0X2D
#define ACCEL_DATA_FORMAT			0x31

#define ACCEL_DATAX0				0x32
#define ACCEL_DATAX1				0x33
#define ACCEL_DATAY0				0x34
#define ACCEL_DATAY1				0x35
#define ACCEL_DATAZ0				0x36
#define ACCEL_DATAZ1				0x37

#define ACCEL_SM_DATARATE			0b1011
#define ACCEL_FM_DATARATE			0b1101

#define ACCEL_MODE_STANDBY			0
#define ACCEL_MODE_MEASURE			1

#define ACCEL_RANGE_2g				0b00
#define ACCEL_RANGE_4g				0b01
#define ACCEL_RANGE_8g				0b10
#define ACCEL_RANGE_16g				0b11

#define GRAVEDAD					9.78

/*Headers de funciones*/
void Accel_Config(I2C_Handler_t *ptrHandlerI2C);
void ChangeAccelMode(I2C_Handler_t *ptrHandlerI2C, uint8_t accelMode);
uint8_t GetAccelMode(I2C_Handler_t *ptrHandlerI2C);
void ChangeAccelRange(I2C_Handler_t *ptrHandlerI2C,uint8_t accelRange);
uint8_t GetAccelRange(I2C_Handler_t *ptrHandlerI2C);

uint8_t GetAccelID(I2C_Handler_t *ptrHandlerI2C);
int16_t GetAccelXDATA(I2C_Handler_t *ptrHandlerI2C);
int16_t GetAccelYDATA(I2C_Handler_t *ptrHandlerI2C);
int16_t GetAccelZDATA(I2C_Handler_t *ptrHandlerI2C);

float ConvertUnits(I2C_Handler_t *ptrHandlerI2C, int16_t data);


#endif /* INC_ADXL345DRIVER_H_ */
