/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "SysTickDriver.h"
#include "PwmDriver.h"
#include "I2CDriver.h"
#include "PLLDriver.h"

/*Handlers*/
GPIO_Handler_t handlerOnBoardLed			={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};
GPIO_Handler_t handlerMCO1					={0};

BasicTimer_Handler_t handlerTimerBlinky		={0};

USART_Handler_t handlerUSART6				={0};

/*Variables*/
char bufferMsg[64] 			={0};
char USARTDataRecieved 			=0;

/*Headers de funciones*/
void InitHardware(void);

int main(void) {

	InitHardware();
	//configPLL();
	//ChangeUSART_BRR(&handlerUSART6, 80);



	/* Loop forever*/
	while (1) {

		if (USARTDataRecieved != '\0'){
			if(USARTDataRecieved == 's'){

				sprintf(bufferMsg,"\nSu mensaje se mando exitosamente");
				writeMsg(&handlerUSART6, bufferMsg);


			}
			else if (USARTDataRecieved == 'b'){

				writeChar(&handlerUSART6, 'w');

			}
			USARTDataRecieved = '\0';
		}
	}
}

void InitHardware(void){

	/*Configuramos el Blinky*/
	handlerOnBoardLed.pGPIOx 								= GPIOA;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	handlerTimerBlinky.ptrTIMx								=TIM2;
	handlerTimerBlinky.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
	handlerTimerBlinky.TIMx_Config.TIMx_speed				=BTIMER_SPEED_1ms;
	handlerTimerBlinky.TIMx_Config.TIMx_period				=250;   //Con esto, el blinky va a 250ms
	handlerTimerBlinky.TIMx_Config.TIMx_interruptEnable		=BTIMER_INTERRUPT_ENABLE;

	GPIO_Config(&handlerOnBoardLed);
	GPIO_WritePin(&handlerOnBoardLed, 1);
	BasicTimer_Config(&handlerTimerBlinky);

	/*Configuramos el pin para revisar la frecuencia del micro*/
		handlerMCO1.pGPIOx 								= GPIOA;
		handlerMCO1.GPIO_PinConfig.GPIO_PinNumber 		= PIN_8;
		handlerMCO1.GPIO_PinConfig.GPIO_PinMode 		= GPIO_MODE_ALTFN;
		handlerMCO1.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF0;
		handlerMCO1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
		handlerMCO1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
		handlerMCO1.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerMCO1);

	/*Configuramos la comunicacion serial*/
	//Configuramos pines para la comunicacion serial
	handlerPinTX.pGPIOx 								= GPIOA;
	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_2;
	handlerPinTX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF7;

	handlerPinRX.pGPIOx 								= GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_3;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF7;

	//Cargamos las configuraciones
	GPIO_Config(&handlerPinTX);
	GPIO_Config(&handlerPinRX);

	//Configuramos el USART
	handlerUSART6.ptrUSARTx 						= USART2;
	handlerUSART6.USART_Config.USART_baudrate 		= USART_BAUDRATE_115200;
	handlerUSART6.USART_Config.USART_datasize 		= USART_DATASIZE_8BIT;
	handlerUSART6.USART_Config.USART_mode			= USART_MODE_RXTX;
	handlerUSART6.USART_Config.USART_parity 		= USART_PARITY_NONE;
	handlerUSART6.USART_Config.USART_stopbits 		= USART_STOPBIT_1;
	handlerUSART6.USART_Config.USART_enableIntRX 	= USART_RX_INTERRUP_ENABLE;

	USART_Config(&handlerUSART6);

}


void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}

void usart2Rx_Callback(void){
	USARTDataRecieved =getRxData();
}
