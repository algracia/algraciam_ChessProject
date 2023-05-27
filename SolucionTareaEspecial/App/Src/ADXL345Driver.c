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

	/*Vamos a activar la unidad de punto flotante para ciertos calculos*/
	SCB->CPACR |= (0XF << 20);

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
float GetAccelXDATA(I2C_Handler_t *ptrHandlerI2C){

	//Leemos los registros con los datos instantaneos en X
	uint8_t dataX0 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAX0);
	uint8_t dataX1 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAX1);

	//Como estos son complementarios, los montamos ambos en una misma variable
	int16_t dataX = (dataX1 << 8) | dataX0;

	//Convertimos el dato al sistema metrico
	float convertedDataX = ConvertUnits(ptrHandlerI2C, dataX);

	return convertedDataX;
}

/*Funcion para recibir datos en Y*/
float GetAccelYDATA(I2C_Handler_t *ptrHandlerI2C){

	//Leemos los registros con los datos instantaneos en Y
	uint8_t dataY0 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAY0);
	uint8_t dataY1 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAY1);

	//Como estos son complementarios, los montamos ambos en una misma variable
	int16_t dataY = (dataY1 << 8) | dataY0;

	//Convertimos el dato al sistema metrico
	float convertedDataY = ConvertUnits(ptrHandlerI2C, dataY);

	return convertedDataY;
}

/*Funcion para recibir datos en Z*/
float GetAccelZDATA(I2C_Handler_t *ptrHandlerI2C){

	//Leemos los registros con los datos instantaneos en Z
	uint8_t dataZ0 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAZ0);
	uint8_t dataZ1 = i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATAZ1);

	//Como estos son complementarios, los montamos ambos en una misma variable
	int16_t dataZ = (dataZ1 << 8) | dataZ0;

	//Convertimos el dato al sistema metrico
	float convertedDataZ = ConvertUnits(ptrHandlerI2C, dataZ);

	return convertedDataZ;
}

/*Funcion para cambiar el rango del acelerometro*/
void ChangeAccelRange(I2C_Handler_t *ptrHandlerI2C,uint8_t accelRange){

	i2c_writeSingleRegister(ptrHandlerI2C, ACCEL_DATA_FORMAT,accelRange);
}

/*Funcion para leer el rango en el que esta configurado el acelerometro*/
uint8_t GetAccelRange(I2C_Handler_t *ptrHandlerI2C){

	//Leemos el registro en el que se configura el rango
	 uint8_t accelRangeR= i2c_readSingleRegister(ptrHandlerI2C, ACCEL_DATA_FORMAT);

	//Ahora, aplicamos una mascara para extraer solo los bits que nos interesa
	//analizar de este registro
	 accelRangeR &= (0b11 << 0); //Extraemos el valor

	 return accelRangeR;
}

/*Funcion para convertir los datos al sistema metrico*/
float ConvertUnits(I2C_Handler_t *ptrHandlerI2C, int16_t data){

	//Todas las conversiones seran en terminos de g (9,78 m/s^2)
	float dataConverted = 0;

	//Hacemos un switch case para cada configuracion del rango
	//del acelerometro ya que cada uno tendra su propia conversion de g
	switch(GetAccelRange(ptrHandlerI2C)){

	case ACCEL_RANGE_2g:{

		dataConverted = ((float)data/256) * GRAVEDAD;
		break;
	}

	case ACCEL_RANGE_4g:{

		dataConverted = ((float)data/128) * GRAVEDAD;
		break;
	}

	case ACCEL_RANGE_8g:{

		dataConverted = ((float)data/64) * GRAVEDAD;
		break;
	}

	case ACCEL_RANGE_16g:{

		dataConverted = ((float)data/32) * GRAVEDAD;
		break;
	}

	default:{
		__NOP();
		break;
	}
	}

	return dataConverted;
}

