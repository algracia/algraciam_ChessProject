/*
 * ADXL345Driver.c
 *
 *  Created on: May 26, 2023
 *      Author: algraciam
 */

#include <stdint.h>
#include "ADXL345Driver.h"
#include "I2CDriver.h"

void Accel_Config(I2C_Handler_t *ptrHandlerI2C){
	//Aqui simplemente configuramos valores por defecto del
	//Data Rate y el rango de las medidas

	//Hacemos un switch case para configurar el Data Rate segun
	//el modo de I2C
	switch(ptrHandlerI2C->modeI2C){

	case I2C_MODE_SM:{
		//Escribimos en el registro indicado
		i2c_writeSingleRegister(ptrHandlerI2C, ACCEL_BW_RATE, ACCEL_SM_DATARATE);
		break;
	}
	case I2C_MODE_FM:{
		//Escribimos en el registro indicado
		i2c_writeSingleRegister(ptrHandlerI2C, ACCEL_BW_RATE, ACCEL_FM_DATARATE);
		break;
	}
	default:{
		__NOP();
	}
	}

	/*Configuramos la resolucion por defecto que es +-2g*/
	i2c_writeSingleRegister(ptrHandlerI2C, ACCEL_DATA_FORMAT,ACCEL_RANGE_2g);

	/*No vale la pena configurar un modo por defecto ya que el acelerometro
	 * lo hace por si solo una vez se conecta.
	 */

}//Fin funcion Accel_Config

/*Funcion para cambiar de modo StanBy a Measure y viceversa*/
void ChangeAccelMode(I2C_Handler_t *ptrHandlerI2C, uint8_t accelMode){

	//Creamos una mascara que seleccione el bit que queremos modificar
	uint8_t modeMsk = accelMode << 3;

	//Escribimos en el registro que controla esto
	i2c_writeSingleRegister(ptrHandlerI2C, ACCEL_POWER_CTL, modeMsk);

}

uint8_t GetAccelMode(I2C_Handler_t *ptrHandlerI2C){

	//Leemos este mismo registro solo para corroborar que efectivamente se configuró
	//bien este bit
	 uint8_t accelModeR= i2c_readSingleRegister(ptrHandlerI2C, ACCEL_POWER_CTL);

	//Ahora, aplicamos una mascara para extraer solo el bit que nos interesa
	//analizar de este registro
	 accelModeR &= (1 << 3); //Extraemos el valor
	 accelModeR >>= 3; //Lo corremos hasta la posicion cero

	 return accelModeR;
}

/*Funcion para recibir el ID del acelerometro y verificar*/
uint8_t GetAccelID(I2C_Handler_t *ptrHandlerI2C){

	uint8_t accelID = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_ID);

	return accelID;
}

/*Funcion para recibir datos en X*/
int16_t GetAccelXDATA(I2C_Handler_t *ptrHandlerI2C){

	//Leemos los registros con los datos instantaneos en X
	uint8_t dataX0 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAX0);
	uint8_t dataX1 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAX1);

	//Como estos son complementarios, los montamos ambos en una misma variable
	int16_t dataX = (dataX1 << 8) | dataX0;

	return dataX;
}

/*Funcion para recibir datos en Y*/
int16_t GetAccelYDATA(I2C_Handler_t *ptrHandlerI2C){

	//Leemos los registros con los datos instantaneos en Y
	uint8_t dataY0 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAY0);
	uint8_t dataY1 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAY1);

	//Como estos son complementarios, los montamos ambos en una misma variable
	int16_t dataY = (dataY1 << 8) | dataY0;

	return dataY;
}

/*Funcion para recibir datos en Z*/
int16_t GetAccelZDATA(I2C_Handler_t *ptrHandlerI2C){

	//Leemos los registros con los datos instantaneos en Z
	uint8_t dataZ0 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAZ0);
	uint8_t dataZ1 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAZ1);

	//Como estos son complementarios, los montamos ambos en una misma variable
	int16_t dataZ = (dataZ1 << 8) | dataZ0;

	return dataZ;
}

