/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Este el main del proyecto final de tallerV
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


/*Configuramos los handlers*/
GPIO_Handler_t handlerOnBoardLed 			={0};
GPIO_Handler_t handlerSeñalM1				={0};
GPIO_Handler_t handlerSeñalM2				={0};
GPIO_Handler_t handlerDireccM1				={0};
GPIO_Handler_t handlerDireccM2				={0};
GPIO_Handler_t handlerSeñalServo			={0};
GPIO_Handler_t handlerUserButton			={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};

BasicTimer_Handler_t handlerTimerBlinky 	={0};

PWM_Handler_t handlerPwmM1					={0};
PWM_Handler_t handlerPwmM2					={0};
PWM_Handler_t handlerPwmServo				={0};

USART_Handler_t handlerUSART2				={0};

EXTI_Config_t handlerEXTIUserButton			={0};


/*Variables*/
uint16_t pulseWidth 		=10;
uint8_t USARTDataRecieved 	=0;
char direccion[]			={0};
char bufferMsg[64] 			={0};

/*Headers de funciones*/
void InitHardware (void);

int main(void) {

	InitHardware();
	disableOutput(&handlerPwmM1);
	disableOutput(&handlerPwmM2);
	disableOutput(&handlerPwmServo);

	/* Loop forever*/
	while (1) {

		if(USARTDataRecieved != '\0'){

			switch(USARTDataRecieved){

			case 'w': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Arriba");

				GPIO_WritePin(&handlerDireccM1, 0); //CW
				GPIO_WritePin(&handlerDireccM2, 1); //CCW

				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				break;
			}

			case 's': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Abajo");

				GPIO_WritePin(&handlerDireccM1, 1); //CCW
				GPIO_WritePin(&handlerDireccM2, 0); //CW

				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				break;
			}

			case 'd': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Derecha");

				GPIO_WritePin(&handlerDireccM1, 0); //CW
				GPIO_WritePin(&handlerDireccM2, 0); //CW

				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				break;
			}


			case 'a': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Izquierda");

				GPIO_WritePin(&handlerDireccM1, 1); //CCW
				GPIO_WritePin(&handlerDireccM2, 1); //CCW

				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				break;
			}

			case ' ': {

				sprintf(bufferMsg,"\n%s", "STOP");

				disableOutput(&handlerPwmM1);
				disableOutput(&handlerPwmM2);

				break;
			}

			default:{
				disableOutput(&handlerPwmM1);
				disableOutput(&handlerPwmM2);

				break;
			}
			}//Fin del Switch

			writeMsg(&handlerUSART2, bufferMsg);

			USARTDataRecieved = '\0';

		}//Fin del 'if'
	}
}

void InitHardware (void){
	/*Configuramos el Blinky*/
	handlerOnBoardLed.pGPIOx 								= GPIOB;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_10;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	handlerTimerBlinky.ptrTIMx								=TIM2;
	handlerTimerBlinky.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
	handlerTimerBlinky.TIMx_Config.TIMx_speed				=BTIMER_SPEED_1ms;
	handlerTimerBlinky.TIMx_Config.TIMx_period				=250;
	handlerTimerBlinky.TIMx_Config.TIMx_interruptEnable		=BTIMER_INTERRUPT_ENABLE;

	GPIO_Config(&handlerOnBoardLed);
	BasicTimer_Config(&handlerTimerBlinky);


	/*Configuramos la Señal de los motores*/
	handlerSeñalM1.pGPIOx 								= GPIOC;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_7;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	handlerSeñalM2.pGPIOx 								= GPIOC;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_6;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	GPIO_Config(&handlerSeñalM1);
	GPIO_Config(&handlerSeñalM2);

	//PWM de los motores
	handlerPwmM1.ptrTIMx				=TIM3;
	handlerPwmM1.config.channel 		=PWM_CHANNEL_2;
	handlerPwmM1.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmM1.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmM1.config.periodo 		=10; //Equivale a un periodo de 1ms
	handlerPwmM1.config.pulseWidth 		=5; //Equivale a un PW de 0.5 ms o un DutyCicle de 50%

	handlerPwmM2.ptrTIMx				=TIM3;
	handlerPwmM2.config.channel 		=PWM_CHANNEL_1;
	handlerPwmM2.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmM2.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmM2.config.periodo 		=10; //Equivale a un periodo de 1ms
	handlerPwmM2.config.pulseWidth 		=5; //Equivale a un PW de 0.5 ms o un DutyCicle de 50%

	//Cargamos las configuraciones
	pwm_Config(&handlerPwmM1);
	pwm_Config(&handlerPwmM2);

	//Activamos las señales,como para ambos motores es un mismo timer
	//basta con hacer esto para uno solo de los handlers
	startPwmSignal(&handlerPwmM1);

	//Direccion motores
	handlerDireccM1.pGPIOx 								= GPIOB;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_2;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	handlerDireccM2.pGPIOx 								= GPIOC;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_8;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerDireccM1);
	GPIO_Config(&handlerDireccM2);


	/*Configuramos la señal del Servomotor*/
	handlerSeñalServo.pGPIOx 								= GPIOB;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinNumber			= PIN_9;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinAltFunMode		= AF2;

	GPIO_Config(&handlerSeñalServo);

	//PWM del Servo
	handlerPwmServo.ptrTIMx					=TIM4;
	handlerPwmServo.config.channel 			=PWM_CHANNEL_4;
	handlerPwmServo.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmServo.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmServo.config.periodo 			=200; //Equivale a un periodo de 20ms
	handlerPwmServo.config.pulseWidth 		=pulseWidth;

	//Cargamos la configuracion
	pwm_Config(&handlerPwmServo);

	//Activamos la señal
	startPwmSignal(&handlerPwmServo);


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



/*Callbacks Timers*/
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}

void usart2Rx_Callback(void){
	//Cada que se lanze la interrupcion, recibimos datos
	USARTDataRecieved =getRxData();
}



