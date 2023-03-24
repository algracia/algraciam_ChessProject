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

GPIO_Handler_t handlerOnBoardLed ={0};
BasicTimer_Handler_t handlerTimer ={0};

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
	handlerTimer.ptrTIMx									=TIM2;
	handlerTimer.TIMx_Config.TIMx_mode						=BTIMER_MODE_UP;
	handlerTimer.TIMx_Config.TIMx_speed						=BTIMER_SPEED_1ms;
	handlerTimer.TIMx_Config.TIMx_period					=1000;
	handlerTimer.TIMx_Config.TIMx_interruptEnable			=BTIMER_INTERRUPT_ENABLE;

	//Cargamos la configuracion del timer especifico
	BasicTimer_Config(&handlerTimer);

	/* Loop forever
	while (1) {

	}*/
}

void BasicTimerX_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);
}
