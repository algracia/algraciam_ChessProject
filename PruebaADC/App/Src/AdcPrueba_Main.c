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
#include "AdcDriver.h"

/*Handlers*/
GPIO_Handler_t handlerOnBoardLed 		={0};
GPIO_Handler_t handlerPinTX				={0};
GPIO_Handler_t handlerPinRX				={0};

BasicTimer_Handler_t handlerTimerBlinky ={0};
BasicTimer_Handler_t handlerTimerADC	={0};

USART_Handler_t handlerUSART2			={0};

ADC_Config_t handlerADC 				={0};

PWM_Handler_t handlerPWM				={0};

/*Variables*/
uint8_t USARTDataRecieved 	=0;
uint8_t adcComplete			=0;
uint16_t adcData[2]			={0};
uint8_t adcCounter			=0;

char bufferMsg[100] 		={0};

/*Headers*/
void initHardware(void);

int main(void) {

	initHardware();

	/* Loop forever*/
	while (1) {

		if(USARTDataRecieved != '\0'){

			if(USARTDataRecieved == 'w'){
				if (adcComplete) {
					//Vamos a enviar un mensaje a traves del USART

					sprintf(bufferMsg, "\nData Canal 1: %u", adcData[0]);
					writeMsg(&handlerUSART2, bufferMsg);

					adcComplete = 0;
				}
		}
			else if(USARTDataRecieved == 's'){
				if (adcComplete) {
					//Vamos a enviar un mensaje a traves del USART

					sprintf(bufferMsg, "\nData Canal 2: %u", adcData[1]);
					writeMsg(&handlerUSART2, bufferMsg);

					adcComplete = 0;
				}
			}
			else{
				__NOP();
			}
		}
	}
}

void initHardware(void){
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

	handlerPinRX.pGPIOx 								= GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_3;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF7;

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

	/*Configuracion ADC*/
	handlerADC.channel[0]			=ADC_CHANNEL_1;
	handlerADC.channel[1]			=ADC_CHANNEL_8;
	handlerADC.dataAlignment		=ADC_ALIGNMENT_RIGHT;
	handlerADC.samplingPeriod[0]	=ADC_SAMPLING_PERIOD_84_CYCLES;
	handlerADC.samplingPeriod[1]	=ADC_SAMPLING_PERIOD_112_CYCLES;
	handlerADC.resolution			=ADC_RESOLUTION_12_BIT;
	handlerADC.edgeType				=ADC_EDGETYPE_RISING;
	handlerADC.extSelect			=ADC_EXTSEL_TIMER3_CC1;

	ADC_ConfigMultichannel(&handlerADC, 2);

	/*Configurar PWM*/
	handlerPWM.ptrTIMx							=TIM3;
	handlerPWM.config.channel 					=PWM_CHANNEL_1;
	handlerPWM.config.polarity 					=PWM_POLARITY_ACTIVE_HIGH;
	handlerPWM.config.prescaler 				=BTIMER_SPEED_1ms;
	handlerPWM.config.periodo 					=50;
	handlerPWM.config.pulseWidth 				=25;

	pwm_Config(&handlerPWM);
	startPwmSignal(&handlerPWM);

	//Dejamos las salidas deshabilitadas en un principio
	enableOutput(&handlerPWM);

}

void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);
}

//void BasicTimer5_Callback(void){
//	startSingleADC();
//}

void usart2Rx_Callback(void){
	USARTDataRecieved =getRxData();
}

void adcComplete_Callback(void){
	adcData[adcCounter] = getADC();
	adcCounter++;
	if(adcCounter > 1){
		adcCounter =0;
	}
	adcComplete = 1;
}



