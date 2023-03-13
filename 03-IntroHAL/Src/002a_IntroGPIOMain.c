/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************
*Con este programa se desea mostrar el uso vasico de los registros que controlan
*al Micro (SFR) y la forma adecuada para utilizar los operadores &, |, ~ y =,
*para cambiar la configuracion de algun registro.
*Tambien es importante para entender la utilidad de los numeros BIN y HEX
*
*Este programa introduce el periferico mas simple que tiene el micro, que es
*el encargado de manejar los pines de cada puerto al micro.
*
*Debemos definir entonces todos los registros que manejan el periferico GPIOx y luego
*crear algunas funciones para utilizar adecuadamente el equipo
 ******************************************************************************
 */

#include <stdint.h>

#include"stm32f411xx_hal.h"
#include"GPIOxDriver.h"

/*Funcion principal del programa. Es aca donde se ejecuta todo*/
int main(void) {

	//***************************
	//Definimos el handler para el PIN que deseamos configurar
	GPIO_Handler_t handlerUserLedPin ={0};

	//Deseamos trabajar con el puerto GPIOA
	handlerUserLedPin.pGPIOx = GPIOA;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinAltFunMode		=AF0;

	// Cargamos la configuracion del PIN especifico
	GPIO_Config(&handlerUserLedPin);

	// Hacemos que el PIN_A5 quede encendido
	GPIO_WritePin(&handlerUserLedPin,SET);

	//Este es el ciclo principal, donde se ejecuta todo el programa
	while (1) {
		NOP();
	}
}
