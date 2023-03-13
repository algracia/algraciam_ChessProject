/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************
*COnfiguracion base del ambiente de desarrollo
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f411xx_hal.h"

int main(void) {

	// Configuracion inicial del MCU
	RCC->AHB1ENR &= ~(1<<0); // Limpiando la posicion 0 del registro
	RCC->AHB1ENR |= (1<<0); // Activamos la señal de reloj del GPIOA

	// Configurando el pin PA5 como salida
	GPIOA->MODER &= ~(0b11 << 10); // Limpiando las posiciones 11:10 del MODER
	GPIOA->MODER |= (0b01 << 10);  // Configurando el pinA5 como salida

	// Config OTYPE
	GPIOA->OTYPER &= ~(1 << 5);    // Limpiamo la posicion, configuracion push-pull

	// Config OSPEED
	GPIOA->OSPEEDR &= ~(0b11 << 10); // Limpiando posiciones 11:10
	GPIOA->OSPEEDR |= (0b10 << 10);  // Velocidad de salida en Fast

	//Configuracion de las resistencias de PU-PD
	GPIOA->PUPDR &= ~(0b11 << 10);  // Limpiamos las posiciones 11:10, no PUPD

	GPIOA->ODR &= ~(1 << 5); //Limpiamos la salida PA5, apaga el LED
//	GPIOA->ODR |= (1 << 5);  // Enciende el LED

	/* Loop forever */
	while (1) {

	}
} {ñ

