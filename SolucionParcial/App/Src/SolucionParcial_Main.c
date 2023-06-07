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
#include "RTCDriver.h"
#include "AdcDriver.h"

/*Macros utiles*/
#define MCO_HSI		0
#define MCO_LSE		1
#define MCO_PLL		2

#define MCO_PRE_NODIV	0
#define MCO_PRE_2		1
#define MCO_PRE_3		2
#define MCO_PRE_4		3
#define MCO_PRE_5		4

/*Handlers*/
GPIO_Handler_t handlerOnBoardLed			={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};
GPIO_Handler_t handlerMCO1					={0};

BasicTimer_Handler_t handlerTimerBlinky		={0};

USART_Handler_t handlerUSART				={0};

RTC_Handler_t handlerRTC					={0};

ADC_Config_t handlerADC						={0};


/*Variables*/
//USART
char bufferMsg[100] 		={0};
uint8_t USARTDataRecieved 	=0;

//RTC
uint8_t hora 		=0;
uint8_t minutos		=0;
uint8_t segundos 	=0;
uint8_t am_pm 		=0;
uint8_t mes			=0;
uint8_t año			=0;
uint8_t fecha		=0;
uint8_t diaSemana	=0;

uint8_t MCOclock 	=0;
uint8_t MCOpres 	=0;
uint8_t HSItrim		=0;

uint8_t arreglo[16] ={0};

/*Headers de funciones*/
void InitHardware(void);
void MCO1clock(uint8_t reloj);
void MCO1prescaler(uint8_t division);

int main(void) {

	InitHardware();

	hora=getRTChours();

	/* Loop forever*/
	while (1) {

		hora=getRTChours();
		minutos=getRTCminutes();
		segundos =getRTCseconds();
		am_pm=getRTCAmPm();
		mes=getRTCmonth();
		año=getRTCyear();
		fecha = getRTCdate();
		diaSemana = getRTCweekDay();

		/*Aqui comienza el codigo con los comandos por USART*/
		if (USARTDataRecieved != '\0'){

			//Hacemos un switch para cada mensaje de informacion
			switch(USARTDataRecieved){

			case 'c':{
				//Modificamos el reloj del MCO

				switch(MCOclock){

				case 0:{
					//Estamos en el reloj HSI
					MCO1clock(MCO_HSI);
					sprintf(bufferMsg, "El reloj es el HSI\n");
					break;
				}

				case 1:{
					//Estamos en el reloj LSE
					MCO1clock(MCO_LSE);
					sprintf(bufferMsg, "El reloj es el LSE\n");
					break;
				}

				case 2:{
					//Estamos en el reloj PLL
					MCO1clock(MCO_PLL);
					sprintf(bufferMsg, "El reloj es el PLL\n");
					break;
				}

				default:{
					__NOP();
					break;
				}
				}

				MCOclock++;

				if(MCOclock > 2){
					MCOclock = 0;
				}

				break;
			}//Fin del Case 'c'

			case 'p':{

				//Modificamos el prescaler del MCO
				switch(MCOpres){

				case 0:{
					//El prescaler es de cero
					MCO1prescaler(MCO_PRE_NODIV);
					sprintf(bufferMsg, "El MCO no tiene prescaler\n");
					break;
				}

				case 1:{
					//El prescaler es de 2
					MCO1prescaler(MCO_PRE_2);
					sprintf(bufferMsg, "El MCO tiene prescaler de 2\n");
					break;
				}

				case 2:{
					//El prescaler es de 3
					MCO1prescaler(MCO_PRE_3);
					sprintf(bufferMsg, "El MCO tiene prescaler de 3\n");
					break;
				}

				case 3:{
					//El prescaler es de 4
					MCO1prescaler(MCO_PRE_4);
					sprintf(bufferMsg, "El MCO tiene prescaler de 4\n");
					break;
				}

				case 4:{
					//El prescaler es de 5
					MCO1prescaler(MCO_PRE_5);
					sprintf(bufferMsg, "El MCO tiene prescaler de 5\n");
					break;
				}

				default:{
					__NOP();
					break;
				}
				}

				MCOpres++;

				if(MCOpres > 4){
					MCOpres = 0;
				}

				break;
			}//Fin del case p

			case 'w':{
				//Aumentamos el valor del trimmer y lo cargamos
				HSItrim++;

				if(HSItrim > 31){
					HSItrim = 31;
				}

				//Limpiamos el valor del registro
				RCC->CR &= ~RCC_CR_HSITRIM;

				//Cargamos el nuevo valor
				RCC->CR |= (HSItrim << RCC_CR_HSITRIM_Pos);

				sprintf(bufferMsg,"El trimer del HSI vale: %u\n",HSItrim);
				break;
			}

			case 's':{
				//Aumentamos el valor del trimmer y lo cargamos
				HSItrim--;

				if(HSItrim < 0){
					HSItrim = 0;
				}
				//Limpiamos el valor del registro
				RCC->CR &= ~RCC_CR_HSITRIM;

				//Cargamos el nuevo valor
				RCC->CR |= (HSItrim << RCC_CR_HSITRIM_Pos);

				sprintf(bufferMsg,"El trimer del HSI vale: %u\n",HSItrim);
				break;
			}

			default:{
				sprintf(bufferMsg, "Ingrese comando valido\n");
			}
			}

			writeMsg(&handlerUSART, bufferMsg);
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
	handlerUSART.ptrUSARTx 							= USART2;
	handlerUSART.USART_Config.USART_baudrate 		= USART_BAUDRATE_115200;
	handlerUSART.USART_Config.USART_datasize 		= USART_DATASIZE_8BIT;
	handlerUSART.USART_Config.USART_mode			= USART_MODE_RXTX;
	handlerUSART.USART_Config.USART_parity 			= USART_PARITY_NONE;
	handlerUSART.USART_Config.USART_stopbits 		= USART_STOPBIT_1;
	handlerUSART.USART_Config.USART_enableIntRX 	= USART_RX_INTERRUP_ENABLE;

	USART_Config(&handlerUSART);


	/*Configuramos el RTC*/
	handlerRTC.formatoHora	=RTC_FORMATO_12HORAS;
	handlerRTC.hora			=11;
	handlerRTC.minutos		=37;
	handlerRTC.segundos		=0;
	handlerRTC.am_pm		=RTC_AM;
	handlerRTC.mes			=JUNIO;
	handlerRTC.fecha		=6;
	handlerRTC.diaSemana	=MARTES;
	handlerRTC.año			=23;

	configRTC(&handlerRTC);

	handlerADC.channel[0] = ADC_CHANNEL_2;
	handlerADC.channel[1] = ADC_CHANNEL_4;





}

void MCO1clock(uint8_t reloj){

	/*Apagamos el PLL y el LSE (el HSE no hay necesidad
	 * por que no esta habilitado
	 */
	//Apagamos el PLL
	RCC->CR &= ~RCC_CR_PLLON;

	//Apagamos el LSE
	RCC->BDCR &= ~RCC_BDCR_LSEON;

	/*Limpiamos la parte del registro CFGR
	*Que controla la fuente de reloj
	*/
	RCC->CFGR &= ~RCC_CFGR_MCO1;

	switch(reloj){

	case MCO_HSI:{
		//Seleccionamos el HSI en el MCO
		RCC->CFGR |= (0b00 << RCC_CFGR_MCO1_Pos);
		break;
	}

	case MCO_LSE:{
		//Seleccionamos el LSE en el MCO
		RCC->CFGR |= (0b01 << RCC_CFGR_MCO1_Pos);
		break;
	}
	case MCO_PLL:{
		//Seleccionamos el LSE en el MCO
		RCC->CFGR |= (0b11 << RCC_CFGR_MCO1_Pos);
		break;
	}
	default:{
		__NOP();
	}
	}

	/*Encendemos el PLL y el LSE*/
	//Encendemos el PLL
	RCC->CR |= RCC_CR_PLLON;

	//Encendemos el LSE
	RCC->BDCR |= RCC_BDCR_LSEON;
}

void MCO1prescaler(uint8_t division){

	/*Apagamos el PLL y el LSE (el HSE no hay necesidad
	 * por que no esta habilitado
	 */
	//Apagamos el PLL
	RCC->CR &= ~RCC_CR_PLLON;

	//Apagamos el LSE
	RCC->BDCR &= ~RCC_BDCR_LSEON;

	 /*Limpiamos la parte del registro CFGR
	Que controla el preescaler del MCO1
	*/
	RCC->CFGR &= ~RCC_CFGR_MCO1PRE;

	switch(division){

	case MCO_PRE_NODIV:{
		//El prescaler del MCO sera de 1:1
		RCC->CFGR |= (0b000 << RCC_CFGR_MCO1PRE_Pos);
		break;
	}

	case MCO_PRE_2:{
		//El prescaler del MCO sera de 2
		RCC->CFGR |= (0b100 << RCC_CFGR_MCO1PRE_Pos);
		break;
	}

	case MCO_PRE_3:{
		//El prescaler del MCO sera de 3
		RCC->CFGR |= (0b101 << RCC_CFGR_MCO1PRE_Pos);
		break;
	}

	case MCO_PRE_4:{
		//El prescaler del MCO sera de 4
		RCC->CFGR |= (0b110 << RCC_CFGR_MCO1PRE_Pos);
		break;
	}

	case MCO_PRE_5:{
		//El prescaler del MCO sera de 5
		RCC->CFGR |= (0b111 << RCC_CFGR_MCO1PRE_Pos);
		break;
	}

	default:{
		__NOP();
	}
	}

	/*Encendemos el PLL y el LSE*/
	//Encendemos el PLL
	RCC->CR |= RCC_CR_PLLON;

	//Encendemos el LSE
	RCC->BDCR |= RCC_BDCR_LSEON;
}

/*Callbacks*/

void usart2Rx_Callback(void){
	USARTDataRecieved =getRxData();
}

void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}



