/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************
*COnfiguracion base del ambiente de desarrollo
 ******************************************************************************
 */

// Libreria para los tipos de variables
#include <stdint.h>

// Libreria para variables booleana
#include <stdbool.h>

// Libreria para usar funciones matematicas
#include <math.h>

#define CONSTANTE 	100
#define UNSIGNED	0
#define SIGNED		1

bool variableBooleana = true;
uint8_t parametro1 = 100;
int16_t parametro2 = -173;

// Header funciones
void clearGlobal(void);
uint8_t getMaxChar(void);
uint16_t getMaxValue(uint16_t x, uint16_t y, uint16_t z);
uint64_t getMaxValuePerBit (uint8_t numero_bits, uint8_t signo);

int main(void) {

	clearGlobal();
	parametro1 = getMaxChar();
	//uint64_t valormax = getMaxValuePerBit(8,UNSIGNED);

	uint8_t bin = 0b10010110;
	uint8_t dec = 0;
	uint8_t aux = 0;
	// convertidor de binario a decimal
	for (uint8_t i = 0; i<8; i++){
		aux = bin & (1<<i);
		aux *= pow(2,i);
		dec+=aux;
	}
	/* Loop forever */
	while (1) {

	}
}

void clearGlobal(void){
	variableBooleana = false;
	parametro1 = 0;
	parametro2 = 0;

}

uint8_t getMaxChar(void){
	uint8_t maxChar = pow(2,8)-1;
	return maxChar;
}

uint16_t getMaxValue(uint16_t x, uint16_t y, uint16_t z){
	if ((x>=y) && (x>=z)){
		return x;
	}else if ((y>=x) && (y>=z)){
		return y;
	}else{
		return z;
	}
}

uint64_t getMaxValuePerBit (uint8_t numero_bits, uint8_t signo){

	uint64_t maxVar;

	if (numero_bits ==8 || numero_bits ==16 || numero_bits ==32 || numero_bits ==64){
		if(signo ==SIGNED){
			maxVar = pow(2,numero_bits)/2 -1;
		}else if (signo == UNSIGNED){
			maxVar = pow(2,numero_bits)-1;
		}else{
			maxVar = 0;
		}
	}else{
		maxVar = 0;
	}
	return maxVar;
}

