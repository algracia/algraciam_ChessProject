/*
 * PwmDriver.c
 *
 *  Created on: May 4, 2023
 *      Author: algraciam
 */
#include "PwmDriver.h"

/**/
void pwm_Config(PWM_Handler_t *ptrPwmHandler){

	/* 1. Activar la señal de reloj del periférico requerido */
	if(ptrPwmHandler->ptrTIMx == TIM2){
		// Registro del RCC que nos activa la señal de reloj para el TIM20
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM3){
		// Registro del RCC que nos activa la señal de reloj para el TIM3
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM4){
			// Registro del RCC que nos activa la señal de reloj para el TIM4
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
		}
	else if(ptrPwmHandler->ptrTIMx == TIM5){
				// Registro del RCC que nos activa la señal de reloj para el TIM5
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
			}
	else{
		__NOP();
	}

	/* 1. Cargamos el periodo deseado */
	setPeriod(ptrPwmHandler);

	/* 2. Cargamos el valor del PulseWidth*/
	setPulseWidth(ptrPwmHandler);

	/* 2a. Estamos en UP_Mode, el limite se carga en ARR y se comienza en 0 */
	//Se configura la direccion
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_DIR;

	//Se comienza el counter desde cero
	ptrPwmHandler->ptrTIMx->CNT = 0;

	/* 3. Configuramos los bits CCxS del registro TIMy_CCMR1, de forma que sea modo salida
	 * (para cada canal hay un conjunto CCxS)
	 *
	 * 4. Además, en el mismo "case" podemos configurar el modo del PWM, su polaridad...
	 *
	 * 5. Y además activamos el preload bit, para que cada vez que exista un update-event
	 * el valor cargado en el CCRx será recargado en el registro "shadow" del PWM */
	switch(ptrPwmHandler->config.channel){
	case PWM_CHANNEL_1:{
		// Seleccionamos el canal como salida
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC1S_Msk;

		// Configuramos el canal como PWM modo 1
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1M_Msk; //Primero limpiamos las posiciones

		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1M_1; //Colocamos el bit 6 del CCMR1 en 1
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1M_2; //Colocamos el bit 5 del CCMR1 en 1
		//Esto nos deja con un 110 en los 3 bits del OC1M

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1PE_Msk;

		//Activamos el fast enable
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1FE_Msk;

		//Vainita que idealmente es mejor dejar en cero
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1CE_Msk;

		break;
	}

	case PWM_CHANNEL_2:{
		// Seleccionamos el canal como salida
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC2S_Msk;

		// Configuramos el canal como PWM modo 1
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC2M_Msk; //Primero limpiamos las posiciones

		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2M_1; //Colocamos el bit 14 del CCMR1 en 1
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2M_2; //Colocamos el bit 13 del CCMR1 en 1
		//Esto nos deja con un 110 en los 3 bits del OC2M

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2PE_Msk;

		//Activamos el fast enable
		ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2FE_Msk;

		//Vainita que idealmente es mejor dejar en cero
		ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC2CE_Msk;

		break;
	}

	//Para los canales 3 y 4 usamos las mismas mascaras de los dos anteriores
	//ya que los registros son iguales

	case PWM_CHANNEL_3:{
		// Seleccionamos el canal como salida
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR1_CC1S_Msk;

		// Configuramos el canal como PWM modo 1
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR1_OC1M_Msk; //Primero limpiamos las posiciones

		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC1M_1; //Colocamos el bit 6 del CCMR2 en 1
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC1M_2; //Colocamos el bit 5 del CCMR2 en 1
		//Esto nos deja con un 110 en los 3 bits del OC3M

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC1PE_Msk;

		//Activamos el fast enable
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC1FE_Msk;

		//Vainita que idealmente es mejor dejar en cero
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR1_OC1CE_Msk;

		break;
	}

	case PWM_CHANNEL_4:{
		// Seleccionamos el canal como salida
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR1_CC2S_Msk;

		// Configuramos el canal como PWM modo 1
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR1_OC2M_Msk; //Primero limpiamos las posiciones

		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC2M_1; //Colocamos el bit 14 del CCMR2 en 1
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC2M_2; //Colocamos el bit 13 del CCMR2 en 1
		//Esto nos deja con un 110 en los 3 bits del OC4M

		// Activamos la funcionalidad de pre-load
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC2PE_Msk;

		//Activamos el fast enable
		ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR1_OC2FE_Msk;

		//Vainita que idealmente es mejor dejar en cero
		ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR1_OC2CE_Msk;

		break;
	}

	default:{
		break;
	}
	}// fin del switch-case

	/* 6. Activamos la salida seleccionada */
	enableOutput(ptrPwmHandler);

	/*7. Configuramos la polaridad del PWM*/
	setPolarity(ptrPwmHandler);

}// Fin de la funcion pwmConfig


/* Función para activar el Timer y activar todo el módulo PWM */
void startPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	//Activamos el timer
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;

}


/* Función para desactivar el Timer y detener todo el módulo PWM*/
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler) {
	//desactivamos el timer
	ptrPwmHandler->ptrTIMx->CR1 &= ~TIM_CR1_CEN;
	disableOutput(ptrPwmHandler);
}


/* Función encargada de activar cada uno de los canales con los que cuenta el TimerX */
void enableOutput(PWM_Handler_t *ptrPwmHandler) {

	switch (ptrPwmHandler->config.channel) {

	case PWM_CHANNEL_1: {
		// Activamos la salida del canal 1
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E_Msk;

		break;
	}

	case PWM_CHANNEL_2: {
		// Activamos la salida del canal 2
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E_Msk;

		break;
	}

	case PWM_CHANNEL_3: {
		// Activamos la salida del canal 3
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E_Msk;

		break;
	}

	case PWM_CHANNEL_4: {
		// Activamos la salida del canal 4
		ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E_Msk;

		break;
	}

	default: {
		break;
	}
	}
}

/* Función encargada de desactiar el canal del TIMx que estemos usando*/
void disableOutput(PWM_Handler_t *ptrPwmHandler) {

	switch (ptrPwmHandler->config.channel) {

	case PWM_CHANNEL_1: {
		// Activamos la salida del canal 1
		ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC1E_Msk;

		break;
	}

	case PWM_CHANNEL_2: {
		// Activamos la salida del canal 2
		ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC2E_Msk;

		break;
	}

	case PWM_CHANNEL_3: {
		// Activamos la salida del canal 3
		ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC3E_Msk;

		break;
	}

	case PWM_CHANNEL_4: {
		// Activamos la salida del canal 4
		ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC4E_Msk;

		break;
	}

	default: {
		break;
	}
	}
}

/*
 * La frecuencia es definida por el conjunto formado por el preescaler (PSC)
 * y el valor límite al que llega el Timer (ARR), con estos dos se establece
 * la frecuencia.
 * */
void setPeriod(PWM_Handler_t *ptrPwmHandler){

	// Cargamos el valor del prescaler, nos define la velocidad o mas bien la unidad a la cual
	// cuenta el Timer
	ptrPwmHandler->ptrTIMx->PSC = ptrPwmHandler->config.prescaler;

	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	// antes de hacer un update y reload.
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo - 1;

	//Le colocamos un pre-load al ARR
	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_ARPE_Msk;
}


/* Función para actualizar la frecuencia, funciona de la mano con setFrequency */
void updatePeriod(PWM_Handler_t *ptrPwmHandler, uint16_t newPeriod){
	// Actualizamos el registro que manipula el periodo
    ptrPwmHandler->config.periodo = newPeriod;

	// Llamamos a la fucnión que cambia la frecuencia
	setPeriod(ptrPwmHandler);
}

/* En este caso es mas intuitivo manejarse por el PulseWidht que el dutty cicle
 * ya que al fin de cuentas eso es lo que estamos cargando
 */
void setPulseWidth(PWM_Handler_t *ptrPwmHandler){

	// Seleccionamos el canal para configurar su pulseWidth
	switch(ptrPwmHandler->config.channel){

	case PWM_CHANNEL_1:{
		ptrPwmHandler->ptrTIMx->CCR1 = ptrPwmHandler->config.pulseWidth;

		break;
	}

	case PWM_CHANNEL_2:{
		ptrPwmHandler->ptrTIMx->CCR2 = ptrPwmHandler->config.pulseWidth;

		break;
	}

	case PWM_CHANNEL_3:{
		ptrPwmHandler->ptrTIMx->CCR3 = ptrPwmHandler->config.pulseWidth;

		break;
	}

	case PWM_CHANNEL_4:{
		ptrPwmHandler->ptrTIMx->CCR4 = ptrPwmHandler->config.pulseWidth;

		break;
	}

	default:{
		break;
	}

	}// fin del switch-case

}


/* Función para actualizar el PulseWidth, funciona de la mano con setDuttyCycle */
void updatePulseWidth(PWM_Handler_t *ptrPwmHandler, uint16_t newPW){
	// Actualizamos el registro que manipula el dutty
	ptrPwmHandler->config.pulseWidth = newPW;

	// Llamamos a la fucnión que cambia el PW y cargamos el nuevo valor
	setPulseWidth(ptrPwmHandler);
}

/*Funcion para designar si durante el duty cicle la señal va a estar activa o inactiva*/
void setPolarity(PWM_Handler_t *ptrPwmHandler){

	if(ptrPwmHandler->config.polarity == PWM_POLARITY_ACTIVE_LOW){
		//Estamos en active low por ende debemos configurar el canal
		//segun esta polaridad

		switch (ptrPwmHandler->config.channel) {

			case PWM_CHANNEL_1: {
				// Configuramos la polaridad del canal 1
				ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1P_Msk;

				break;
			}

			case PWM_CHANNEL_2: {
				// Configuramos la polaridad del canal 2
				ptrPwmHandler->ptrTIMx->CCER  |= TIM_CCER_CC2P_Msk;

				break;
			}

			case PWM_CHANNEL_3: {
				// Configuramos la polaridad del canal 3
				ptrPwmHandler->ptrTIMx->CCER  |= TIM_CCER_CC3P_Msk;

				break;
			}

			case PWM_CHANNEL_4: {
				// Configuramos la polaridad del canal 4
				ptrPwmHandler->ptrTIMx->CCER  |= TIM_CCER_CC4P_Msk;

				break;
			}

			default: {
				break;
			}
		}//Fin del switch case
	}

	else {
		//Estamos en active high por ende debemos configurar el canal
		//segun esta polaridad. Este sera nuestra polaridad por defecto

		switch (ptrPwmHandler->config.channel) {

			case PWM_CHANNEL_1: {
				// Configuramos la polaridad del canal 1
				ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC1P_Msk;

				break;
			}

			case PWM_CHANNEL_2: {
				// Configuramos la polaridad del canal 2
				ptrPwmHandler->ptrTIMx->CCER  &= ~TIM_CCER_CC2P_Msk;

				break;
			}

			case PWM_CHANNEL_3: {
				// Configuramos la polaridad del canal 3
				ptrPwmHandler->ptrTIMx->CCER  &= ~TIM_CCER_CC3P_Msk;

				break;
			}

			case PWM_CHANNEL_4: {
				// Configuramos la polaridad del canal 4
				ptrPwmHandler->ptrTIMx->CCER  &= ~TIM_CCER_CC4P_Msk;

				break;
			}

			default: {
				break;
			}
		}//Fin del switch case
	}

}

