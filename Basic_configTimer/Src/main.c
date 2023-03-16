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
#include <stm32f4xx.h>

#include "GPIOxDriver.h"

GPIO_Handler_t handlerOnBoardLed ={0};

int main(void) {

	//Deseamos trabajar con el puerto GPIOA
	handlerOnBoardLed.pGPIOx 								= GPIOA;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	// Cargamos la configuracion del PIN especifico
	GPIO_Config(&handlerOnBoardLed);

	/* Configuracion del TIMER */

	//Activando la señal de reloj
	RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	// Direccion
	TIM2->CR1 &= ~TIM_CR1_DIR;

	// Preescaler
	TIM2->PSC = 8000;

	// Configurar el counter
	TIM2->CNT = 0;

	//Configurar ARR
	TIM2->ARR = 250;

	//Activar el timer
	TIM2->CR1 |= TIM_CR1_CEN;


	/* Loop forever */
	while (1) {
		if (TIM2->SR & TIM_SR_UIF){
			TIM2->SR &= ~TIM_SR_UIF;
			GPIOxTooglePin(&handlerOnBoardLed);
		}
		else{
			continue;
		}
	}
}
