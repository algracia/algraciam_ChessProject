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
GPIO_Handler_t handlerUserButton = {0};
BasicTimer_Handler_t handlerTimer ={0};

uint8_t counterExti13 = 0;

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


		// Cargamos la configuracion del PIN especifico
		GPIO_Config(&handlerOnBoardLed);
		GPIO_Config(&handlerUserButton);


		/* Configuracion del TIMER */
		handlerTimer.ptrTIMx									=TIM2;
		handlerTimer.TIMx_Config.TIMx_mode						=BTIMER_MODE_UP;
		handlerTimer.TIMx_Config.TIMx_speed						=BTIMER_SPEED_1ms;
		handlerTimer.TIMx_Config.TIMx_period					=250;
		handlerTimer.TIMx_Config.TIMx_interruptEnable			=BTIMER_INTERRUPT_ENABLE;

		//Cargamos la configuracion del timer especifico
		BasicTimer_Config(&handlerTimer);

		/*COnfigurando el EXTI*/
		//2. Activando la señal de reloj de SYSCFG
		RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

		//3. Conf el Mux13 para que utilice el puerto C 0xF -> 0b1111
		SYSCFG->EXTICR[3] &= ~(0xF << SYSCFG_EXTICR4_EXTI13_Pos);
		SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

		//4.a Configurar el tipo de flanco
		EXTI->FTSR = 0; //Desactivamos todos los posibles flancos de bajada
		EXTI->RTSR = 0; //Llevando el registro a un valor conocido
		EXTI->RTSR |= EXTI_RTSR_TR13;

		//4.b Activar interrupcion
		EXTI->IMR = 0;
		EXTI->IMR |= EXTI_IMR_IM13;

		// 5.a Desactivar TODAS las interrupciones
		__disable_irq();

		//5.b Matricular la interrupcion en el NVIC
		NVIC_EnableIRQ(EXTI15_10_IRQn);

		// 5.c Crear ISR
		//5.d Crear el Callback
		//5.e Activar las interrupciones
		__enable_irq();


}//Termina el initHardware

void callback_exti13(void){
	counterExti13++;
}

void EXTI15_10_IRQHandler(void){
	if((EXTI->PR & EXTI_PR_PR13) !=0){
		EXTI->PR |= EXTI_PR_PR13;	//Limpiar la bandera de exti 13
		callback_exti13();
	}
}
