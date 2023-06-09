/*
 * USARTxDriver.c
 *
 *  Created on: Apr 27, 2023
 *      Author: algraciam
 */

#include <stm32f4xx.h>
#include "USARTxDriver.h"

char sendingData =0;
uint8_t busy =0;
/**
 * Configurando el puerto Serial...
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */
uint8_t auxRxData = '\0';
uint8_t iter =0;

void USART_Config(USART_Handler_t *ptrUsartHandler){
	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (USART1, USART2, USART6) */

    /* 1.1 Configuramos el USART1 */
	if(ptrUsartHandler->ptrUSARTx == USART1){
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	}

    /* 1.2 Configuramos el USART2 */
	else if(ptrUsartHandler->ptrUSARTx == USART2){
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	}

    /* 1.3 Configuramos el USART6 */
	else if(ptrUsartHandler->ptrUSARTx == USART6){
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
	}

	/* 2. Configuramos el tamaño del dato, la paridad y los bit de parada */
	/* En el CR1 estan parity (PCE y PS) y tamaño del dato (M) */
	/* Mientras que en CR2 estan los stopbit (STOP)*/
	/* Configuracion del Baudrate (registro BRR) */
	/* Configuramos el modo: only TX, only RX, o RXTX */


	// 2.1 Comienzo por limpiar los registros, para cargar la configuración desde cero
	ptrUsartHandler->ptrUSARTx->CR1 = 0;
	ptrUsartHandler->ptrUSARTx->CR2 = 0;

	// 2.2 Configuracion del Parity:
	// Verificamos si el parity esta activado o no
    // Tenga cuidado, el parity hace parte del tamaño de los datos...
	if(ptrUsartHandler->USART_Config.USART_parity != USART_PARITY_NONE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PCE;

		// Verificamos si se ha seleccionado ODD or EVEN
		if(ptrUsartHandler->USART_Config.USART_parity == USART_PARITY_EVEN){
			// Es even, entonces cargamos la configuracion adecuada
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PS;

		}else{
			// Si es "else" significa que la paridad seleccionada es ODD, y cargamos esta configuracion
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PS;
		}
	}else{
		// Si llegamos aca, es porque no deseamos tener el parity-check
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PCE;
	}

	// 2.3 Configuramos el tamaño del dato
	if(ptrUsartHandler->USART_Config.USART_datasize == USART_DATASIZE_8BIT){
		//El dato tiene 8 bits
		ptrUsartHandler->ptrUSARTx->CR1 &=~USART_CR1_M;

	}else if(ptrUsartHandler->USART_Config.USART_datasize == USART_DATASIZE_9BIT){
		//El dato tiene 9 bits
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;
	}

	// 2.4 Configuramos los stop bits (SFR USART_CR2)
	switch(ptrUsartHandler->USART_Config.USART_stopbits){
	case USART_STOPBIT_1: {
		// Debemoscargar el valor 0b00 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 &= ~USART_CR2_STOP;
		break;
	}
	case USART_STOPBIT_0_5: {
		// Debemoscargar el valor 0b01 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP_0;
		break;
	}
	case USART_STOPBIT_2: {
		// Debemoscargar el valor 0b10 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP_1;
		break;
	}
	case USART_STOPBIT_1_5: {
		// Debemoscargar el valor 0b11 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= USART_CR2_STOP;
		break;
	}
	default: {
		// En el casopor defecto seleccionamos 1 bit de parada
		ptrUsartHandler->ptrUSARTx->CR2 &= ~USART_CR2_STOP;
		break;
	}
	}

	// 2.5 Configuracion del Baudrate (SFR USART_BRR)
	// Ver tabla de valores (Tabla 73), Frec = 16MHz, over8 = 0;

	//Limpiamos el BRR por si las moscas*/
	ptrUsartHandler->ptrUSARTx->BRR = 0;

	if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_9600){
		// El valor a cargar es 104.1875 -> Mantiza = 104,fraction = 0.1875
		// Mantiza = 104 = 0x68, fraction = 16 * 0.1875 = 3
		// Valor a cargar 0x0683
		// Configurando el Baudrate generator para una velocidad de 9600bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0683;
	}

	else if (ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_19200) {
		// El valor a cargar es 52.0625 -> Mantiza = 52,fraction = 0.0625
		// Mantiza = 52 = 0x34, fraction = 16 * 0.0625 = 1
		// Valor a cargar 0x0341
		// Configurando el Baudrate generator para una velocidad de 19200bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0341;
	}

	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_115200){
		// El valor a cargar es 8.6875 -> Mantiza = 8,fraction = 0.6875
		// Mantiza = 8 = 0x8, fraction = 16 * 0.6875 = 11 = 0xB
		// Valor a cargar 0x008b
		// Configurando el Baudrate generator para una velocidad de 115200bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x008B;
	}

	// 2.6 Configuramos el modo: TX only, RX only, RXTX, disable
	switch(ptrUsartHandler->USART_Config.USART_mode){
	case USART_MODE_TX:
	{
		// Activamos la parte del sistema encargada de enviar
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		break;
	}
	case USART_MODE_RX:
	{
		// Activamos la parte del sistema encargada de recibir
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_RXTX:
	{
		// Activamos ambas partes, tanto transmision como recepcion
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RE;
		break;
	}
	case USART_MODE_DISABLE:
	{
		// Desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		break;
	}

	default:
	{
		// Actuando por defecto, desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		break;
	}
	}

	/*como vamos a utilizar interrupciones hay que configurar unos pasos extra
	 * 3.0 Desactivo primero las interrupciones globales
	 */
    __disable_irq();

	/*4.0 Revisamos si se van a utilizar o no interrupciones*/
	//Primero, si la interrupcion sera por RX
	if(ptrUsartHandler->USART_Config.USART_enableIntRX == USART_RX_INTERRUP_ENABLE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RXNEIE;

	}else{
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RXNEIE;
	}


	/* 5.0 Activamos el canal del sistema NVIC para que lea la interrupción*/
	if(ptrUsartHandler->ptrUSARTx == USART1){
		// Activando en NVIC para la interrupción del USART1
		__NVIC_EnableIRQ(USART1_IRQn);
	}
	else if(ptrUsartHandler->ptrUSARTx == USART2){
		// Activando en NVIC para la interrupción del USART2
		__NVIC_EnableIRQ(USART2_IRQn);
	}
	else if(ptrUsartHandler->ptrUSARTx == USART6){
		// Activando en NVIC para la interrupción del USART2
		__NVIC_EnableIRQ(USART6_IRQn);
	}
	else{
		__NOP();
	}

	/* 6. Volvemos a activar las interrupciones del sistema */
	__enable_irq();

	/* Por ultimo activamos el modulo USART cuando todo esta correctamente configurado */
	// 7 Activamos el modulo serial.
	if(ptrUsartHandler->USART_Config.USART_mode != USART_MODE_DISABLE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;
	}

}//Fin de la funcion USART_Config



/* funcion para escribir un solo char */
char writeChar(USART_Handler_t *ptrUsartHandler, char dataToSend ){


	/*Actualizamos la variable que envia el mensaje y hacemos que sea igual
	 * al caracter que queramos mandar
	 */
	sendingData = dataToSend;

	/*Activamos la interrupciones por transmicion*/
	ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_TXEIE;


	/*En esta parte, la interrupcion deberia "saltar" */

	/*Deshabilitamos la interrupcion por transmicion
	 * luego de haber enviado el mensaje
	 */
	ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TXEIE;


	return sendingData;
}


/* Funcion para escribir un mensaje de caracteres*/
void writeMsg(USART_Handler_t *ptrUsartHandler, char *MsgToSend ){

	//Renicializamos la variable iteradora del while
	//Esta ira aumentado dentro de la IRQ
	iter = 0;

	while(MsgToSend[iter] != '\0'){
		writeChar(ptrUsartHandler, MsgToSend[iter]);
	}

	busy =0;
}

/*Funcion de lecutra del caracter que llega por la interface serial*/
uint8_t getRxData(void){
	return auxRxData;
}



/*Handlers de la interrupciones del USART
 * Acada deben estar todas las interrupciones asociadas: TX, RX, PE
 */

void USART1_IRQHandler (void){
	//Evaluamos si la interrupcion fue por RX
	if(USART1->SR & USART_SR_RXNE){
		auxRxData = (uint8_t) USART1->DR;
		usart1Rx_Callback();
	}
	//Evaluamos si fue por TX
	else if (USART1->SR & USART_SR_TXE){

		/*Enviamos el dato*/
		if(busy == 0){
			//Enviamos un primer caracter, solo una vez
			//Para generar un delay
			USART1->DR = '\0';

			//Subimos la bandera de busy
			busy =1;
		}
		else{
			USART1->DR = sendingData;
			/*Aumentamos la variable de iteracion
			 en caso de enviar un mensaje*/
			iter++;
		}
	}
}


void USART2_IRQHandler (void){
	//Evaluamos si la interrupcion fue por RX
	if(USART2->SR & USART_SR_RXNE){
		auxRxData = (uint8_t) USART2->DR;
		usart2Rx_Callback();
	}
	//Evaluamos si fue por TX
	else if (USART2->SR & USART_SR_TXE){

		USART2->DR = sendingData;
		/*Aumentamos la variable de iteracion
		 en caso de enviar un mensaje*/
		iter++;

//		/*Enviamos el dato*/
//		if(busy == 0){
//			//Enviamos un primer caracter, solo una vez
//			//Para generar un delay
//			USART2->DR = '\0';
//
//			//Subimos la bandera de busy
//			busy =1;
//		}
//		else{
//			USART2->DR = sendingData;
//			/*Aumentamos la variable de iteracion
//			 en caso de enviar un mensaje*/
//			iter++;
//		}
	}
}

void USART6_IRQHandler (void){
	//Evaluamos si la interrupcion fue por RX
	if(USART6->SR & USART_SR_RXNE){
		auxRxData = (uint8_t) USART6->DR;
		usart6Rx_Callback();
	}
	//Evaluamos si fue por TX
	else if (USART6->SR & USART_SR_TXE){

		/*Enviamos el dato*/
		if(busy == 0){
			//Enviamos un primer caracter, solo una vez
			//Para generar un delay
			USART6->DR = '\0';

			//Subimos la bandera de busy
			busy =1;
		}
		else{
			USART6->DR = sendingData;
			/*Aumentamos la variable de iteracion
			 en caso de enviar un mensaje*/
			iter++;
		}

	}
}

/*Los callbacks*/
__attribute__((weak)) void usart1Rx_Callback(void){
	  /* NOTE : This function should not be modified, when the callback is needed,
	            the BasicTimerX_Callback could be implemented in the main file
	   */
	__NOP();
}

__attribute__((weak)) void usart2Rx_Callback(void){
	  /* NOTE : This function should not be modified, when the callback is needed,
	            the BasicTimerX_Callback could be implemented in the main file
	   */
	__NOP();
}

__attribute__((weak)) void usart6Rx_Callback(void){
	  /* NOTE : This function should not be modified, when the callback is needed,
	            the BasicTimerX_Callback could be implemented in the main file
	   */
	__NOP();
}
