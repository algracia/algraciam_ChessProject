/*
 * GPIOxDriver.c
 *
 *  Created on: Mar 11, 2023
 *      Author: isabel
 *
 *      Este archivo es la parte del programa donde se escribe adecuadamente el control, para que sea lo mas
 *      genérico posible, de forma que independientemente del puerto GPIO y el PIN seleccionados, el programa
 *      se ejecute y configure todo correctamente
 */

#include "GPIOxDriver.h"

/*
 * Para cualquier periférico, hay varios pasos que siempre se deben seguir en un orden estricto para poder que
 * el sistema permita configurar el periférico x. Lo primero es activar la señal de reloj principal hacia ese
 * elemento específico (relacionado con el periferico RCC), es decir "activar el periferico"
 */

void GPIO_Config (GPIO_Handler_t *pGPIOHandler){

	//variable para hacer todo paso a paso
	uint32_t auxConfig = 0;
	uint32_t auxPosition = 0;

	// 1) activar el periferico
	//verificamos para GPIO
	if(pGPIOHandler->pGPIOx == GPIOA){
		//Escribir 1 (set) en la posicion correspondiente al GPIOA
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOA_EN);
	}

	//verificamos para GPIOB
	else if(pGPIOHandler->pGPIOx == GPIOB){
		//Escribir 1 (set) en la posicion correspondiente a GPIOB
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOB_EN);
	}

	//Verificamos el GPIOC
	else if(pGPIOHandler->pGPIOx == GPIOC){
		//Escribir 1 (set) en la posicion correspondiente a GPIOC
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOC_EN);
	}

	//Verificamos para GPIOD
	else if(pGPIOHandler->pGPIOx == GPIOD){
		//Escribir 1 (set) en la posicion correspondiente a GPIOD
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOD_EN);
	}
	//verificamos para GPIOE
	else if(pGPIOHandler->pGPIOx == GPIOE){
		//Escribir 1 (set) en la posicion correspondiente a GPIOE
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOE_EN);
	}
	//verificamos para GPIOH
	else if(pGPIOHandler->pGPIOx == GPIOH){
		//Escribir 1 (set) en la posicion correspondiente a GPIOH
		RCC->AHB1ENR |= (SET << RCC_AHB1ENR_GPIOH_EN);
	}

	//despues de activado, se comienza a configurar.

	// 2) configurando el registro GPIOx_MODER
	//acá se está leyendo la configuracion, moviendo "PinNumber" veces hacia la izquierda ese valor
	//y todo eso se carga en la variable auxConfig
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinMode << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Antes de cargar el nuevo valor, limpiamos los bits específicos de ese registro (se debe escribir 0b00)
	//usando una máscara y una operación bitwise AND
	pGPIOHandler->pGPIOx->MODER &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//cargamos a auxConfig en el registro MODER
	pGPIOHandler->pGPIOx->MODER |= auxConfig;

	// 3) configurando el registro GPIOx_OTYPER
	//Leemos y movemos el valor un número "PinNumber" de veces
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinOPType << pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Limpiamos antes de cargar
	pGPIOHandler->pGPIOx->OTYPER &= ~(SET << pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//cargamos el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->OTYPER |= auxConfig;

	// 4) configuramos la velocidad
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinSpeed << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//limpiando la posición antes de cargar la nueva configuracion
	pGPIOHandler->pGPIOx->OSPEEDR &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//cargamos el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->OSPEEDR |= auxConfig;

	// 5) configurar si se desea pull-up, pull-down o flotante
	auxConfig = (pGPIOHandler->GPIO_PinConfig.GPIO_PinPuPdControl << 2*pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//limpiando la posición antes de cargar la nueva configuracion
	pGPIOHandler->pGPIOx->PUPDR &= ~(0b11 << 2 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber);

	//cargar el resultado sobre el registro adecuado
	pGPIOHandler->pGPIOx->PUPDR |= auxConfig;

	//Configuración de las funciones alternativas
	if(pGPIOHandler->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN){

		//Primero se selcciona si se debe utilizar el registro bajo (AFRL) o el alto (AFRH)
		if(pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber < 8){
			//estamos en el registro AFRL, que controla los pines del 0 al 7
			auxPosition = 4 * pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber;

			//limpiamos la posicion del registro que deseamos escribir a continuacion
			pGPIOHandler->pGPIOx->AFRL &= ~(0b1111 << auxPosition);

			//escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler->pGPIOx->AFRL |= (pGPIOHandler->GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);
		}
		else{
			//estamos en el registro AFRH, que controla los pines del 8 al 15
			auxPosition = 4 * (pGPIOHandler->GPIO_PinConfig.GPIO_PinNumber -8);

			//limpiamos la posicion del registr que deseamos escribir
			pGPIOHandler->pGPIOx->AFRH &= ~(0b1111 << auxPosition);

			//escribimos el valor configurado en la posicion seleccionada
			pGPIOHandler->pGPIOx->AFRH |= (pGPIOHandler->GPIO_PinConfig.GPIO_PinAltFunMode << auxPosition);
		}
	}
} //Fin del GPIO_config

/*
 * Función utilizada para cambiar de estado el pin entregado en el handler, asignando el valor entregado
 * en la variable newState
 */

void GPIO_WritePin(GPIO_Handler_t *pPinHandler, uint8_t newState){
	//Limpiamos la posicion que deseamos
//	pPinHandler->pGPIOx->ODR &= ~(SET <<pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
	if(newState == SET){
		//Trabajando con la parte baja del registro
		pPinHandler->pGPIOx->BSRR |= (SET << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
	}
	else{
		//Trabajando con la parte alta del registro
		pPinHandler->pGPIOx->BSRR |= (SET << (pPinHandler->GPIO_PinConfig.GPIO_PinNumber + 16));
	}
}

/*
 * Función para leer el estado de un pin específico
 */

uint32_t GPIO_ReadPin(GPIO_Handler_t *pPinHandler){
	//creamos una variable auxiliar la cual luego retornaremos
	uint32_t pinValue = 0;

	//Cargamos el valor del registro IDR, desplazado a derecha tantas veces como la ubicacion del pin específico
	pinValue = (pPinHandler->pGPIOx->IDR >> pPinHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Máscara para conservar únicamente la posición menos significativa del registro.
	pinValue &= (1<<0);

/* Con la máscara anterior, se garantiza que todas las posiciones de registro a la izquierda se vuelvan cero
 * y el único valor que lea sea el del pin que se necesita
 */
	return pinValue;

}

/*
 * Función TooglePin
 */

void GPIOxTooglePin (GPIO_Handler_t *pPinHandler){

	pPinHandler->pGPIOx->ODR ^= (SET <<pPinHandler->GPIO_PinConfig.GPIO_PinNumber);

}



