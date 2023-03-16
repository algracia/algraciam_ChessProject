/*
 *  GPIOxDriver.c
 *
 *  Created on: Mar 10, 2023
 *      Author: algraciam
 *
 *      Este archivo es la parte del programa donde escribimos adecuadamente el control,
 *      para que sea lo mas generico posible, de forma que independiente del puerto GPIO
 *      y el PIN seleccionados, el programa se ejecute y configure todo correctamente.
 */

#include "GPIOxDriver.h"

/**
 * Para cualquier periferico, hay varios pasos que siempre se debem seguir en un
 * orden estricto para poder que el sistema permita configurar el periferico X.
 * Lo primero y mas importante es activar la señal del reloj principal hacia ese
 * elemento especifico (relacionado con el periferico RCC), a esto lo llamaremos
 * simplemente "activar el periferico o activar la señal de reloj del periferico
 */

void GPIO_Config (GPIO_Handler_t *pGPIOHandler){

	// Variable para hacer todo paso a paso
	uint32_t auxConfig = 0;
	uint32_t auxPosition = 0;

	// 1) Activar el periferico
	// Verificamos para GPIOA
	if (pGPIOHandler -> pGPIOx == GPIOA){
		// Escribimos 1 (SET) en la posicion correspondiente al GPIOA
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOA_EN);
	}
	// Verificamos para GPIOB
	else if(pGPIOHandler->pGPIOx == GPIOB){
		// Escribimos 1 (SET) en la posicion correspondiente al GPIOB
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOB_EN);
	}
	// Verificamos para GPIOC
		else if(pGPIOHandler->pGPIOx == GPIOC){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOC
			RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOC_EN);
	}
	// Verificamos para GPIOD
		else if(pGPIOHandler->pGPIOx == GPIOD){
			// Escribimos 1 (SET) en la posicion correspondiente al GPIOD
			RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOD_EN);
	}
	// Verificamos para GPIOE
			else if(pGPIOHandler->pGPIOx == GPIOE){
				// Escribimos 1 (SET) en la posicion correspondiente al GPIOE
				RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOE_EN);
		}
	// Verificamos para GPIOH
			else if(pGPIOHandler->pGPIOx == GPIOH){
				// Escribimos 1 (SET) en la posicion correspondiente al GPIOH
				RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOH_EN);
		}

	//Despues de activdado, podemos comenzar a configurar

	/*2) Configurando el registro GPIOx_MODER
	 * aca estamos leyendo la config, moviendo "PinNUmber" hacia la izquierda ese valor (shift left)
	 * y todo eso lo cargamos en la variable auxConfig
	 */
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinMode << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	/*
	 * Antes de cargar el nuevo valor, limpiamos los bits especificos de ese registro (debemos escribir 0b00)
	 * para lo cual aplicamos una mascara y una operacion bitwise AND
	 */
	pGPIOHandler->pGPIOx->MODER &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos a auxConfig en el registro MODER
	pGPIOHandler->pGPIOx->MODER |= auxConfig;

	/*3) COnfigurando el registro GPIOx_OTYPER
	 * De nuevo, leemos y movemos el valor un numero "PinNUmber" de veces
	 */
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinOPType << pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Limpiamos antes de cargar
	pGPIOHandler->pGPIOx->OTYPER &= ~(SET << pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->OTYPER |= auxConfig;

	/*4) Configurando ahora la velocidad*/
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinSpeed << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Limpiando la posicion antes de cargar la nueva config
	pGPIOHandler->pGPIOx->OSPEEDR &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->OSPEEDR |= auxConfig;

	/*5) Configurando si se desea Pull-up o Pull-down o Flotante*/
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinPuPdControl << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Limpiando la posicion antes de cargar la nueva config
	pGPIOHandler->pGPIOx->PUPDR &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Cargamos el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->PUPDR |= auxConfig;

	//Esta es la parte para la configuracion de las funciones alternativas... se vera luego
	if(pGPIOHandler->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN){

		//Seleccionamos primero si de debe utilizar el registro bajo (AFRL) o el alto (AFRH)
		if(pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber < 8){
			//Estamos en el registro AFRL, que controla los pines del PIN_0 al PIN_7
			auxPosition = 4*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber;

			//Limpiamos primero la posicion del registro que deseamos escribir a continuacion
			pGPIOHandler->pGPIOx->AFRL &= ~(0b1111 << auxPosition);

			//y escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler->pGPIOx->AFRL |= (pGPIOHandler->GPIO_PinConfig.GPIO_PinAltFunMode<< auxPosition);
		}
		else{
			//Estamos en el registro AFRH, que controla los pines del PIN_8 al PIN_15
			auxPosition = 4*(pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber-8);

			//Limpiamos primero la posicion del registro que deseamos escribir a continuacion
			pGPIOHandler->pGPIOx->AFRH &= ~(0b1111 << auxPosition);

			//y escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler->pGPIOx->AFRH |= (pGPIOHandler->GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);
		}
	}
}
//FIN del GPIO_config

/**
 * Funcion utilizada oara cambiar de estado el pin entregado en el handler, asignando el valor entregado en la variable newstate
 */
void GPIO_WritePin(GPIO_Handler_t*pPinHandler, uint8_t newState){
	//Limpiamos la posicion que deseamos
	pPinHandler->pGPIOx->ODR &= ~(SET << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
	if(newState == SET){
		//Trabajando con la parte baja del registro
		pPinHandler->pGPIOx->BSRR |= (SET << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
	}
	else{
		//Trabajando con la parte alta del registro
		pPinHandler->pGPIOx->BSRR |= (SET << (pPinHandler->GPIO_PinConfig.GPIO_PinNumber+16));
	}
}
/**
 * Funcion para leer el estado de un pin especifico
 */
uint32_t GPIO_ReadPin(GPIO_Handler_t*pPinHandler){
	//Creamos una variable auxiliar la cual luego retornaremos
	uint32_t pinValue =0;

	//Cargamos el valor del registro IDR, desplazado a derecha tantas veces como la ubicacion del pin especifico
	pinValue = (pPinHandler->pGPIOx->IDR >> pPinHandler->GPIO_PinConfig.GPIO_PinNumber);

	return pinValue;
}



