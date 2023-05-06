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

/*Configuramos los handlers*/
GPIO_Handler_t handlerOnBoardLed 		={0};
GPIO_Handler_t handlerPinTX				={0};
GPIO_Handler_t handlerPinRX				={0};
BasicTimer_Handler_t handlerTimerBlinky ={0};
USART_Handler_t handlerUSART2			={0};

/*Variables*/
uint8_t sendControl 		=0;
uint8_t USARTDataRecieved 	=0;
char msg[] 					= "Prueba del USART\n";
char bufferMsg[64] 			={0};
uint8_t i =0;

void InitHardware (void);
int main(void) {

	InitHardware();

	/* Loop forever*/
	while (1) {

		if(USARTDataRecieved != '\0'){


			}

		}

	}
}

void InitHardware (void){
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
	handlerTimerBlinky.TIMx_Config.TIMx_period				=250;
	handlerTimerBlinky.TIMx_Config.TIMx_interruptEnable		=BTIMER_INTERRUPT_ENABLE;

	//Cargamos las configuraciones
	GPIO_Config(&handlerOnBoardLed);
	BasicTimer_Config(&handlerTimerBlinky);

	/*Configuramos pines para la comunicacion serial*/
	handlerPinTX.pGPIOx 								= GPIOA;
	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_2;
	handlerPinTX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF7;
//	handlerPinTX.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
//	handlerPinTX.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
//	handlerPinTX.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	handlerPinRX.pGPIOx 								= GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_3;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF7;
//	handlerPinRX.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
//	handlerPinRX.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
//	handlerPinRX.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Cargamos las configuraciones
	GPIO_Config(&handlerPinTX);
	GPIO_Config(&handlerPinRX);

	/*Configuramos el USART*/
	handlerUSART2.ptrUSARTx 						= USART2;
	handlerUSART2.USART_Config.USART_baudrate 		= USART_BAUDRATE_115200;
	handlerUSART2.USART_Config.USART_datasize 		= USART_DATASIZE_8BIT;
	handlerUSART2.USART_Config.USART_mode			= USART_MODE_RXTX;
	handlerUSART2.USART_Config.USART_parity 		= USART_PARITY_NONE;
	handlerUSART2.USART_Config.USART_stopbits 		= USART_STOPBIT_1;
	handlerUSART2.USART_Config.USART_enableIntRX 	= USART_RX_INTERRUP_ENABLE;
	handlerUSART2.USART_Config.USART_enableIntTX 	= USART_TX_INTERRUP_DISABLE;

	USART_Config(&handlerUSART2);
}

void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);
	sendControl++;
}

void usart2Rx_Callback(void){
	USARTDataRecieved =getRxData();
}

