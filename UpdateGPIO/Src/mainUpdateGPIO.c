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
#include "BasicTimer.h"
#include "ExtiDriver.h"

GPIO_Handler_t handlerOnBoardLed 	={0};
GPIO_Handler_t handlerUserButton	={0};
BasicTimer_Handler_t handlerTimer 	={0};
EXTI_Config_t handlerEXTI			={0};

uint8_t ExtiCounter=0;

void initHardware (void);

int main(void) {

	initHardware ();

	/* Loop forever*/
	while (1) {

	}
}

void BasicTimerX_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);
}

void initHardware (void){
	 //En esta funcion se configura el hardware del LED y el Timer
	//Deseamos trabajar con el puerto GPIOA
		handlerOnBoardLed.pGPIOx 								= GPIOA;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		/*Configuramos el user button -PC13*/
		handlerUserButton.pGPIOx								= GPIOC;
		handlerUserButton.GPIO_PinConfig.GPIO_PinNumber			= PIN_13;
		handlerUserButton.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
		handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;



		//Cargamos las configuraciones de los pines
		GPIO_Config(&handlerOnBoardLed);

		/*Configuracion del EXTI*/
		handlerEXTI.pGPIOHandler							=&handlerUserButton;
		handlerEXTI.edgeType								=EXTERNAL_INTERRUPT_RISING_EDGE;

		//Cargamos la configuracion del EXTI
		ExtInt_Config(&handlerEXTI);

		/* Configuracion del TIMER */
		handlerTimer.ptrTIMx									=TIM2;
		handlerTimer.TIMx_Config.TIMx_mode						=BTIMER_MODE_UP;
		handlerTimer.TIMx_Config.TIMx_speed						=BTIMER_SPEED_1ms;
		handlerTimer.TIMx_Config.TIMx_period					=1000;
		handlerTimer.TIMx_Config.TIMx_interruptEnable			=BTIMER_INTERRUPT_ENABLE;

		//Cargamos la configuracion del timer especifico
		BasicTimer_Config(&handlerTimer);



}//Termina el initHardware

void callback_extInt13(void){
	ExtiCounter++;
}


