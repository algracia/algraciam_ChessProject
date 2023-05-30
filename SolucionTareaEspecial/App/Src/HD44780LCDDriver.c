/*
 * HD44780LCDDriver.c
 *
 *  Created on: May 28, 2023
 *      Author: algraciam
 */

#include <stdint.h>
#include "HD44780LCDDriver.h"
#include "I2CDriver.h"
#include "SysTickDriver.h"

void LCD_Config(I2C_Handler_t *ptrHandlerI2C){

	/*0. Cofiguramos el systick para que controle los tiempos de funcionamiento*/
	config_SysTick_ms(PLL_CLOCK_CONFIGURED);

	/*1. Proceso de inicializacion de la pantalla*/
	//Creamos una variable especial para esto
	uint8_t initConfig = 0;

	/*Enviamos un comando vacio*/
	WriteI2CModule(ptrHandlerI2C, 0x00);

	//hacemos un corto delay para que la pantalla encienda
	delay_ms(50);

	//Enviamos la instruccion de inicializacion
	initConfig = LCD_I2C_INSTRUCTION_INIT_LCD;
	WriteI2CModule(ptrHandlerI2C, initConfig);

	//Apagamos el enable
	WriteI2CModule(ptrHandlerI2C, initConfig & ~ LCD_I2C_ENABLE_BIT);

	//Hacemos otro corto delay
	delay_ms(5);

	//Volemos a enviar la instruccion de inicializacion
	WriteI2CModule(ptrHandlerI2C, initConfig);

	//Apagamos el enable
	WriteI2CModule(ptrHandlerI2C, initConfig & ~ LCD_I2C_ENABLE_BIT);

	//Hacemos otro corto delay
	delay_ms(5);

	//Enviamos por ultima vez la instruccion de inicializacion
	WriteI2CModule(ptrHandlerI2C, initConfig);

	//Apagamos el enable
	WriteI2CModule(ptrHandlerI2C, initConfig & ~ LCD_I2C_ENABLE_BIT);

	//Hacemos otro corto delay
	delay_ms(5);

	/*2. Configuramos la pantalla para que funcione a 4 bits*/
	initConfig = LCD_I2C_4BITOPERATION;
	WriteI2CModule(ptrHandlerI2C,initConfig);

	//Apagamos el enable
	WriteI2CModule(ptrHandlerI2C, initConfig & ~ LCD_I2C_ENABLE_BIT);

	//Hacemos otro corto delay
	delay_ms(5);

	/*3.Configuramos la pantalla a una linea y de 5x8 bits*/
	WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_2LINE_5X8FONT);

	/*4. Apagamos la pantalla*/
	WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_DISPLAY_OFF);

	/*5.Limpiamos el display*/
	WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_DISPLAY_CLEAN);

	/*6. Configuramos un modo de entradas que al escribir, mueva el cursor a la derecha*/
	WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_ENTRYMODE_R);

	/*7. Encendemos la pantalla y el cursor*/
	WriteLCDInstruction(ptrHandlerI2C,LCD_INSTRUCTION_DISPLAY_ON | LCD_INSTRUCTION_CURSOR_ON);

}

/*Funcion para enviar caracteres*/
void WriteLCDCharacter(I2C_Handler_t *ptrHandlerI2C,uint8_t character){

	/*Como se esta trabajando a 4 bits de informacion, las instrucciones se
	 * tienen que mandar a dos partes
	 */
	uint8_t upper4Bits =0;
	uint8_t lower4Bits =0;

	//extraemos de la instruccion los trozos de bits que nos interesan
	upper4Bits = character & (0xF << 4);
	lower4Bits = character & (0xF << 0);

	//Creamos un array para guardar los Bytes a enviar
	uint8_t dataToSend = 0;

	//combinamos los bits de informacion con los bits por defecto para
	//el proceso de escritura con este modulo I2C, ademas, reorganizamos
	//los bits de modo que el envio sea correcto

	/*Enviamos los bits superiores*/
	dataToSend = (upper4Bits) | LCD_I2C_WRITE_CHARACTERS;

	WriteI2CModule(ptrHandlerI2C, dataToSend);

	//Limpiamos el enable
	WriteI2CModule(ptrHandlerI2C, dataToSend & ~ LCD_I2C_ENABLE_BIT);

	/*Enviamos los bits inferiores*/
	dataToSend = (lower4Bits << 4) | LCD_I2C_WRITE_CHARACTERS;

	WriteI2CModule(ptrHandlerI2C, dataToSend);

	//Limpiamos el enable
	WriteI2CModule(ptrHandlerI2C, dataToSend & ~ LCD_I2C_ENABLE_BIT);

	//Hacemos otro corto delay
	delay_ms(5);

}

/*Funcion para enviar instrucciones*/
void WriteLCDInstruction(I2C_Handler_t *ptrHandlerI2C,uint8_t instruccion){

	/*Como se esta trabajando a 4 bits de informacion, las instrucciones se
	 * tienen que mandar a dos partes
	 */
	uint8_t upper4Bits =0;
	uint8_t lower4Bits =0;

	//extraemos de la instruccion los trozos de bits que nos interesan
	upper4Bits = instruccion & (0xF << 4);
	lower4Bits = instruccion & (0xF << 0);

	//Creamos una variable para guardar los Bytes a enviar
	uint8_t dataToSend = 0;

	//combinamos los bits de informacion con los bits por defecto para
	//el proceso de escritura con este modulo I2C, ademas, reorganizamos
	//los bits de modo que el envio sea correcto

	/*Enviamos los bits superiores*/
	dataToSend = (upper4Bits) | LCD_I2C_WRITE_INSTRUCTIONS;

	WriteI2CModule(ptrHandlerI2C, dataToSend);

	//Limpiamos el enable
	WriteI2CModule(ptrHandlerI2C, dataToSend & ~ LCD_I2C_ENABLE_BIT);

	/*Enviamos los bits inferiores*/
	dataToSend = (lower4Bits << 4) | LCD_I2C_WRITE_INSTRUCTIONS;

	WriteI2CModule(ptrHandlerI2C, dataToSend);

	//Limpiamos el enable
	WriteI2CModule(ptrHandlerI2C, dataToSend & ~ LCD_I2C_ENABLE_BIT);

	//Hacemos otro corto delay
	delay_ms(5);

}

/*Funcion para mandar los datos a traves del I2C*/
void WriteI2CModule(I2C_Handler_t *ptrHandlerI2C,uint8_t i2cData){

	//Para esto, vamos a seguir el orden que indica el datasheet
	//del modulo I2C

	/*1. Mandamos una condicion de START*/
	i2c_startTransaction(ptrHandlerI2C);

	/*2. Mandamos el address del esclavo*/
	i2c_sendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	/*3. Mandamos el dato, ya sea una instruccion o un caracter*/
	i2c_sendDataByte(ptrHandlerI2C, i2cData);

	/*4. Generamos la condicion de STOP luego de 1 o 2 Bytes*/
	i2c_stopTransaction(ptrHandlerI2C);

}

void WriteLCDMsg(I2C_Handler_t *ptrHandlerI2C,char *i2cMsg){

	uint8_t iter=0;

	while(i2cMsg[iter] != '\0'){
	WriteLCDCharacter(ptrHandlerI2C, i2cMsg[iter]);
	iter++;
	}

}

/*Funcion para desplazar el cursor de la LCD a una posicion que queramos*/
void MoveLCDCursor(I2C_Handler_t *ptrHandlerI2C, uint8_t cursorLine, uint8_t cursorPos){

	/*Esta funcion basicamente recibe la linea a la cual se va a desplazar el cursor
	 * y la posicion en dicha linea hasta la cual va a llegar
	 */

	//Primero, llevamos el cursor hasta Home
	WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_HOME);

	/*Nos desplazamos hasta la linea que nos interese*/
	switch(cursorLine){

	case 0:{
		__NOP();
		break;
	}

	case 1:{
		__NOP();
		break;
	}
	case 2:{

		for(uint8_t i = 0; i < 40; i++){
			WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_CURSOR_RIGHT);
		}
		break;
	}

	case 3:{

		for(uint8_t i = 0; i < 20; i++){
			WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_CURSOR_RIGHT);
		}
		break;
	}
	case 4:{

		for(uint8_t i = 0; i < 60; i++){
			WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_CURSOR_RIGHT);
		}
		break;
	}
	default:{
		/*Por defecto, simplemente volveremos al "HOME"*/
		WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_HOME);
		break;
	}
	}

	//Ahora, nos movemos en esa misma linea hasta la posicion que se quiera
	for(uint8_t i = 0; i < cursorPos; i++){
		WriteLCDInstruction(ptrHandlerI2C, LCD_INSTRUCTION_CURSOR_RIGHT);
	}

}

/*Funcion para leer la busyflag*/
uint8_t ReadLCDbusyFlag(I2C_Handler_t *ptrHandlerI2C){

	/*0. Creamos una variable auxiliar para recibir el dato que leemos*/
	uint8_t auxRead = 0;

	uint8_t busyFlag = 0;

	/*1. Generamos la condicion Start*/
	i2c_startTransaction(ptrHandlerI2C);

	/*2. Enviamos la direccion del esclavo y la indicacion de ESCRIBIR*/
	i2c_sendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	/*3. Enviamos la instruccion para que se lea la BusyFlag*/
	i2c_sendDataByte(ptrHandlerI2C, LCD_I2C_READBF);

	/*4. Creamos una condicion de reStart*/
	i2c_reStartTransaction(ptrHandlerI2C);

	/*5. Enviamos la direccion del esclavo y la indicacion de LEER*/
	i2c_sendSlaveAddressRW(ptrHandlerI2C, ptrHandlerI2C->slaveAddress, I2C_READ_DATA);

	/*6. Leemos el dato que envia el esclavo*/
	auxRead = i2c_readDataByte(ptrHandlerI2C);

	/*7.Generamos la condicion NoAck, para que el Master no responda y slave solo envie 1 Byte*/
	i2c_sendNoAck(ptrHandlerI2C);

	/*8.Generamos la condicion Stop, para que el slave se detenga 1 Byte*/
	i2c_stopTransaction(ptrHandlerI2C);


	busyFlag = auxRead & LCD_BUSYFLAGBIT;

	busyFlag >>=7;

	return busyFlag;
}
