/*
 * RTCDriver.c
 *
 *  Created on: Jun 4, 2023
 *      Author: algraciam
 */

#include <stm32f4xx.h>
#include "RTCDriver.h"

void configRTC(RTC_Handler_t *ptrRTCHandler){

	/*1. Hay que desactivar las protecciones de escritura
	 * en los registros de este periferico
	 */
	/*1a. Debemos habilitar el periferico del Power Control*/
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	/*1b. habilitamos la escritura en el RTC*/
	PWR->CR |= PWR_CR_DBP;

	/*1c. Seleccionamos la fuente de reloj del RTC*/
	//Limpiamos la parte del registro que nos interesa
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;

	//La fuente de reloj será el LSE de 32 768 Hz
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;

	/*1d. Activamos la señal de reloj hacia el RTC*/
	RCC->BDCR |= RCC_BDCR_RTCEN;

	/*1e. Escribimos la claves de seguridad en el WPR*/
	//Limpiamos el registro
	RTC->WPR = 0;

	//Escribimos la primera clave
	RTC->WPR |= 0xCA;
	__NOP();

	//Limpiamos nuevamente el registro
	RTC->WPR = 0;

	//Escribimos la segunda clave
	RTC->WPR |= 0x53;
	__NOP();

	/*2. Configuramos la hora y el calendario*/
	/*2a. Indicamos que estamos en modo de inicializacion*/
	RTC->ISR |= RTC_ISR_INIT;

	//Revisamos la bandera del modo de inicializacion
	while(!(RTC->ISR & RTC_ISR_INITF)){
		_NOP();
	}

	//En este punto se podrian cambiar los valores del RTC_PRE
	//pero con los que viene por defecto, ya el RTC queda con la
	//frecuencia necesitada (ojo, esto usando como reloj el LSE)

	/*2b. Fijamos el formato de la hora*/
	if(ptrRTCHandler->formatoHora == RTC_FORMATO_12HORAS){
		//Estamos en formato de 12 horas
		RTC->CR |= RTC_CR_FMT;

	}else{
		//Estamos en formato de 24 horas
		RTC->CR |=  RTC_CR_FMT;
		RTC->TR &= ~RTC_TR_PM;
	}



}
