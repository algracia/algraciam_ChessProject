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

/*Macros utiles*/
#define SERVO_UP 	16
#define SERVO_DOWN 	14

/*Configuramos los handlers*/
GPIO_Handler_t handlerOnBoardLed 			={0};
GPIO_Handler_t handlerSeñalM1				={0};
GPIO_Handler_t handlerSeñalM2				={0};
GPIO_Handler_t handlerDireccM1				={0};
GPIO_Handler_t handlerDireccM2				={0};
GPIO_Handler_t handlerEnableM1				={0};
GPIO_Handler_t handlerEnableM2				={0};
GPIO_Handler_t handlerSeñalServo			={0};
GPIO_Handler_t handlerEndStopX				={0};
GPIO_Handler_t handlerEndStopY				={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};

BasicTimer_Handler_t handlerTimerBlinky 	={0};

PWM_Handler_t handlerPwmM1					={0};
PWM_Handler_t handlerPwmM2					={0};
PWM_Handler_t handlerPwmServo				={0};

USART_Handler_t handlerUSART2				={0};

EXTI_Config_t handlerEXTIEndStopX			={0};
EXTI_Config_t handlerEXTIEndStopY			={0};


/*Variables*/

//Las relacionadas al USART
uint8_t USARTDataRecieved 		=0;
char bufferMsg[64] 				={0};
char recievedMsg[100]			={0};
uint8_t counter 				=0;

//Las relacionadas a los pasos
uint16_t pasosMotores 			=500;
uint16_t pasosServo 			=40;
uint16_t contadorPasosMotores	=0;
uint16_t contadorPasosServo		=0;

//Las banderas
uint8_t endStopXFlag			=1;
uint8_t endStopYFlag			=1;

/*Headers de funciones*/
void InitHardware (void);
void Home(void);
void BasicMove (void);

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

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM1, 0); //CCW
				GPIO_WritePin(&handlerDireccM2, 1); //CW

				//Habilitamos las salidas de los PWM
				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case 's': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Abajo");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM1, 1); //CW
				GPIO_WritePin(&handlerDireccM2, 0); //CCW

				//Habilitamos las salidas de los PWM
				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case 'd': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Derecha");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM1, 0); //CCW
				GPIO_WritePin(&handlerDireccM2, 0); //CCW

				//Habilitamos las salidas de los PWM
				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}


			case 'a': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Izquierda");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM1, 1); //CW
				GPIO_WritePin(&handlerDireccM2, 1); //CW

				//Habilitamos las salidas de los PWM
				enableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case 'e': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Diagonal derecha arriba");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM1, 0); //CW

				//Habilitamos las salida del PWM
				//solo del motor que se mueve y
				//deshabilitamos la del otro
				enableOutput(&handlerPwmM1);
				disableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case 'q': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Diagonal izquierda arriba");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM2, 1); //CCW

				//Habilitamos las salida del PWM
				//solo del motor que se mueve y
				//deshabilitamos la del otro
				disableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case 'E': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Diagonal derecha abajo");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM1, 1); //CCW

				//Habilitamos las salida del PWM
				//solo del motor que se mueve y
				//deshabilitamos la del otro
				enableOutput(&handlerPwmM1);
				disableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case 'Q': {

				sprintf(bufferMsg,"\nLa direccion es: %s", "Diagonal izquierda abajo");

				//Activamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 1);
				GPIO_WritePin(&handlerEnableM2, 1);

				//Ajustamos la direccion
				GPIO_WritePin(&handlerDireccM2, 0); //CW

				//Habilitamos las salida del PWM
				//solo del motor que se mueve y
				//deshabilitamos la del otro
				disableOutput(&handlerPwmM1);
				enableOutput(&handlerPwmM2);

				contadorPasosMotores =0;

				break;
			}

			case ' ': {

				sprintf(bufferMsg,"\n%s", "STOP");

				//Desactivamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 0);
				GPIO_WritePin(&handlerEnableM2, 0);

				//Deshabilitamos todas las señales de PWM
				disableOutput(&handlerPwmM1);
				disableOutput(&handlerPwmM2);
				disableOutput(&handlerPwmServo);

				break;
			}

			/*Probamos el PW para el servo*/
			//Esto hay que separarlo luego
			case 'u': {
				//Hacemos que el servo suba
				updatePulseWidth(&handlerPwmServo, SERVO_UP);

				sprintf(bufferMsg,"\nEl servo esta arriba");

				//Habilitamos el movimiento
				enableOutput(&handlerPwmServo);

				//Reiniciamos la variable para los pasos
				contadorPasosServo =0;

				break;
			}

			case 'o': {
				//Hacemos que el servo baje
				updatePulseWidth(&handlerPwmServo, SERVO_DOWN);

				sprintf(bufferMsg,"\nEl servo esta abajo");

				//Habilitamos el movimiento
				enableOutput(&handlerPwmServo);

				//Reiniciamos la variable para los pasos
				contadorPasosServo =0;

				break;
			}

			/*Ire cambiando el numero de pasos para los motores*/
			case 't':{
				//Aumentamos el numero de pasos en 10
				pasosMotores += 10;

				sprintf(bufferMsg,"\nEl numero de pasos aumentó a: %u",pasosMotores);

				break;
			}

			case 'y':{
				//disminuimos el numero de pasos en 10
				pasosMotores -= 10;

				sprintf(bufferMsg,"\nEl numero de pasos disminuyó a: %u",pasosMotores);

				break;
			}

			/*Probamos la funcion Home*/
			case 'H':{
				sprintf(bufferMsg,"\nRegresando a Home");

				Home();


				break;
			}

			case 'h':{
				sprintf(bufferMsg,"\nPosicionando en cero");

				BasicMove();

				break;
			}

			/*Creamos una funcion que reciba del USART comandos y los imprima*/
			case 'r':{
				sprintf(bufferMsg,"\nPor favor, escriba su comando: ");
				writeMsg(&handlerUSART2, bufferMsg);

				counter =0;
				USARTDataRecieved = '\0';

				while(USARTDataRecieved != '#'){

					if(USARTDataRecieved != '\0' && USARTDataRecieved != '#'){
						recievedMsg[counter] =USARTDataRecieved;
						counter++;
						USARTDataRecieved ='\0';

					}
				}

				recievedMsg[counter] = '\0';
				sprintf(bufferMsg, "\nTu comando ingresado fue: ");
				writeMsg(&handlerUSART2, bufferMsg);
				writeMsg(&handlerUSART2, recievedMsg);

				sprintf(bufferMsg," ");
				break;
			}
			default:{

				sprintf(bufferMsg,"\nIngrese un caracter valido para continuar");

				//Desactivamos los enable de los drivers
				GPIO_WritePin(&handlerEnableM1, 0);
				GPIO_WritePin(&handlerEnableM2, 0);

				//Deshabilitamos todas las salidas de PWM
				disableOutput(&handlerPwmM1);
				disableOutput(&handlerPwmM2);
				disableOutput(&handlerPwmServo);

				break;
			}
			}//Fin del Switch

			writeMsg(&handlerUSART2, bufferMsg);

			USARTDataRecieved = '\0';

		}//Fin del 'if'


		if(contadorPasosMotores > pasosMotores){
			//Desactivamos los enable de los drivers
			GPIO_WritePin(&handlerEnableM1, 0);
			GPIO_WritePin(&handlerEnableM2, 0);

			//desactivamos las salidad de los PWM
			disableOutput(&handlerPwmM1);
			disableOutput(&handlerPwmM2);

		}

		if(contadorPasosServo > pasosServo){
			disableOutput(&handlerPwmServo);
		}

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
	handlerSeñalM1.pGPIOx 								= GPIOB;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_4;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	handlerSeñalM2.pGPIOx 								= GPIOA;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_7;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	GPIO_Config(&handlerSeñalM1);
	GPIO_Config(&handlerSeñalM2);

	//Iniciamos la señal a un valor conocido
	GPIO_WritePin(&handlerSeñalM1, 1);
	GPIO_WritePin(&handlerSeñalM2, 1);

	//PWM de los motores
	handlerPwmM1.ptrTIMx				=TIM3;
	handlerPwmM1.config.channel 		=PWM_CHANNEL_1;
	handlerPwmM1.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmM1.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmM1.config.periodo 		=10; //Equivale a un periodo de 1ms
	handlerPwmM1.config.pulseWidth 		=5; //Equivale a un PW de 0.5 ms o un DutyCicle de 50%
	handlerPwmM1.config.interruption	=PWM_PERIOD_INTERRUPT_ENABLE;

	handlerPwmM2.ptrTIMx				=TIM3;
	handlerPwmM2.config.channel 		=PWM_CHANNEL_2;
	handlerPwmM2.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmM2.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmM2.config.periodo 		=10; //Equivale a un periodo de 1ms
	handlerPwmM2.config.pulseWidth 		=5; //Equivale a un PW de 0.5 ms o un DutyCicle de 50%
	handlerPwmM2.config.interruption	=PWM_PERIOD_INTERRUPT_ENABLE;


	//Cargamos las configuraciones
	pwm_Config(&handlerPwmM1);
	pwm_Config(&handlerPwmM2);

	//Activamos las señales,como para ambos motores es un mismo timer
	//basta con hacer esto para uno solo de los handlers
	startPwmSignal(&handlerPwmM1);

	//Direccion motores
	handlerDireccM1.pGPIOx 								= GPIOB;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_5;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	handlerDireccM2.pGPIOx 								= GPIOA;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_12;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerDireccM1);
	GPIO_Config(&handlerDireccM2);

	//Enable de los motores
	handlerEnableM1.pGPIOx 								= GPIOC;
	handlerEnableM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_4;
	handlerEnableM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerEnableM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerEnableM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerEnableM1.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	handlerEnableM2.pGPIOx 								= GPIOA;
	handlerEnableM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_8;
	handlerEnableM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerEnableM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerEnableM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerEnableM2.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerEnableM1);
	GPIO_Config(&handlerEnableM2);

	//Iniciamos la señal a un valor conocido
	GPIO_WritePin(&handlerEnableM1, 0); //un cero aqui, deshabilita los drivers
	GPIO_WritePin(&handlerEnableM2, 0);


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
	handlerPwmServo.config.pulseWidth 		=15;
	handlerPwmServo.config.interruption		=PWM_PERIOD_INTERRUPT_ENABLE;

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


	/*Configuramos los EXTI para los finales de carrera*/

	//Configuramos los pines como entradas
	handlerEndStopX.pGPIOx								= GPIOA;
	handlerEndStopX.GPIO_PinConfig.GPIO_PinNumber		= PIN_1;
	handlerEndStopX.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerEndStopX.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	handlerEndStopY.pGPIOx								= GPIOA;
	handlerEndStopY.GPIO_PinConfig.GPIO_PinNumber		= PIN_4;
	handlerEndStopY.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerEndStopY.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Configuramos los handler del EXTI
	handlerEXTIEndStopX.pGPIOHandler	=&handlerEndStopX;
	handlerEXTIEndStopX.edgeType		=EXTERNAL_INTERRUPT_RISING_EDGE;

	handlerEXTIEndStopY.pGPIOHandler	=&handlerEndStopY;
	handlerEXTIEndStopY.edgeType		=EXTERNAL_INTERRUPT_RISING_EDGE;

	//Cargamos las configuraciones
	ExtInt_Config(&handlerEXTIEndStopX);
	ExtInt_Config(&handlerEXTIEndStopY);
}

void Home(void){

	/*Primero paramos todo por seguridad*/
	disableOutput(&handlerPwmM1);
	disableOutput(&handlerPwmM2);

	/*Activamos los enable de los drivers*/
	GPIO_WritePin(&handlerEnableM1, 1);
	GPIO_WritePin(&handlerEnableM2, 1);

	/*Bajamos la bandera en X para
	que inicie el movimiento en esta direccion.*/
	endStopXFlag =0;

	/*Hacemos que se mueva a la izquierda*/
	GPIO_WritePin(&handlerDireccM1, 1); //CW
	GPIO_WritePin(&handlerDireccM2, 1); //CW

	enableOutput(&handlerPwmM1);
	enableOutput(&handlerPwmM2);

	//Mientras no cambie la bandera, el se mantiene en un bucle
	while(!endStopXFlag){
		__NOP();
	}

//	/*Desactivamos el movimiento en X*/
//	disableOutput(&handlerPwmM1);
//	disableOutput(&handlerPwmM2);


	/*Ahora, bajamos la bandera en Y para
	que inicie el movimiento en esta direccion.*/
	endStopYFlag =0;

	/*Hacemos que se mueva hacia abajo*/
	GPIO_WritePin(&handlerDireccM1, 1); //CW
	GPIO_WritePin(&handlerDireccM2, 0); //CCW

//	enableOutput(&handlerPwmM1);
//	enableOutput(&handlerPwmM2);

	//Mientras no cambie la bandera, el se mantiene en un bucle
	while(!endStopYFlag){
		__NOP();
	}

	/*Desactivamos el movimiento en Y*/
	disableOutput(&handlerPwmM1);
	disableOutput(&handlerPwmM2);

	/*desactivamos los enable de los drivers*/
	GPIO_WritePin(&handlerEnableM1, 0);
	GPIO_WritePin(&handlerEnableM2, 0);


}

void BasicMove (void) {

	//Ahora, hacemos que se mueva hacia arriba
	//y a partir de esta posicion, va a estar nuestro cero

	/*Activamos los enable de los drivers*/
	GPIO_WritePin(&handlerEnableM1, 1);
	GPIO_WritePin(&handlerEnableM2, 1);

	//Ajustamos la direccion
	GPIO_WritePin(&handlerDireccM1, 0); //CCW
	GPIO_WritePin(&handlerDireccM2, 1); //CW

	//Habilitamos las salidas de los PWM
	enableOutput(&handlerPwmM1);
	enableOutput(&handlerPwmM2);

	//Inicializamos el contador de pasos
	contadorPasosMotores =0;

	//Hacemos que se quede en unn bucle hasta que se
	//complete el numero de pasos
	while(!(contadorPasosMotores > 385)){
		__NOP();
	}

	//Detenmos todo
	disableOutput(&handlerPwmM1);
	disableOutput(&handlerPwmM2);

//	/*desactivamos los enable de los drivers*/
//	GPIO_WritePin(&handlerEnableM1, 1);
//	GPIO_WritePin(&handlerEnableM2, 1);
}


/*Callbacks*/
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}

void BasicTimer3_Callback(void){
	contadorPasosMotores++;

}

void BasicTimer4_Callback(void){
	contadorPasosServo++;

}

void usart2Rx_Callback(void){
	//Cada que se lanze la interrupcion, recibimos datos
	USARTDataRecieved =getRxData();
}

void callback_extInt1(void){
	endStopXFlag =1;
}

void callback_extInt4(void){
	endStopYFlag =1;
}








