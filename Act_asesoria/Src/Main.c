/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>

#include"stm32f411xx_hal.h"
#include"GPIOxDriver.h"

#define FIBONACCI	0
#define FACTORIAL	1

uint16_t operacion (uint8_t tipoDeOperacion, uint8_t numero);

int main(void) {
	uint16_t resultadoFinal = 0;
	resultadoFinal = operacion (FIBONACCI, 3);
	resultadoFinal = operacion(FACTORIAL, 5);
	resultadoFinal++;

	//Este es el ciclo principal, donde se ejecuta todo el programa
	while (1) {
	}
}

uint16_t operacion (uint8_t tipoDeOperacion, uint8_t numero){

	uint16_t resultado = 0;

	switch(tipoDeOperacion){
	case FIBONACCI:
	{
		uint8_t  i 	=1;
		uint16_t a 	=0;
		uint16_t b 	=1;
		while(i < numero){
			resultado = a+b;
			a = b;
			b= resultado;
			i++;
		}
		break;
	}

	case FACTORIAL:
	{
		uint8_t j =0;
		resultado = 1;
		while(j < numero){
			resultado *= numero-j;
			j += 1;
		}
		break;
	}
	default:
		break;
}
	return resultado;
}
