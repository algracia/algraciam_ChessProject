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

#include "ADXL345Driver.h"

/*Macros utiles*/



/*Handlers*/
GPIO_Handler_t handlerOnBoardLed			={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};
GPIO_Handler_t handlerMCO1					={0};
GPIO_Handler_t handlerI2cSDA				={0};
GPIO_Handler_t handlerI2cSCL				={0};
GPIO_Handler_t handlerPinPWM_X				={0};
GPIO_Handler_t handlerPinPWM_Y				={0};
GPIO_Handler_t handlerPinPWM_Z				={0};

BasicTimer_Handler_t handlerTimerBlinky		={0};
BasicTimer_Handler_t handlerTimerMuestreo	={0};

USART_Handler_t handlerUSART6				={0};

I2C_Handler_t handlerAcelerometro			={0};

PWM_Handler_t handlerSeñalPWM_X				={0};
PWM_Handler_t handlerSeñalPWM_Y				={0};
PWM_Handler_t handlerSeñalPWM_Z				={0};

/*Variables*/
//USART
char bufferMsg[100] 		={0};
uint8_t USARTDataRecieved 	= 0;

//I2C
uint8_t i2cBuffer			=0;
uint16_t numDato			=0;
float aceleracionesX[2000] 	={0};
float aceleracionesY[2000] 	={0};
float aceleracionesZ[2000] 	={0};

//Acelerometro
uint8_t mode 		=0;
float accelX		=0;
float accelY		=0;
float accelZ		=0;


/*Banderas*/
uint8_t measureRDY 		=0;
uint8_t flagMuestreo 	=0;


/*Headers de funciones*/
void InitHardware(void);

int main(void) {

	InitHardware();
	//configPLL(HSI_80MHz_PLLN, HSI_80MHz_PLLP);
	//ChangeUSART_BRR(&handlerUSART6, 40);


	/* Loop forever*/
	while (1) {

		/*Al comienzo de cada ciclo se toman medidas de la
		 * aceleracion en X Y Z a 1kHz para asi, irlas representando
		 * en la LCD
		 *
		 * ademas, en esta misma etapa se hara la actualizacion constante
		 * de los PWM
		 */
		if(flagMuestreo){

		accelX = GetAccelXDATA(&handlerAcelerometro);
		accelY = GetAccelYDATA(&handlerAcelerometro);
		accelZ = GetAccelZDATA(&handlerAcelerometro);



		flagMuestreo =0;
		}

		if (USARTDataRecieved != '\0'){

			//Hacemos un switch para cada mensaje de informacion
			switch(USARTDataRecieved){

			case 'i':{
				//En este caso se revisa el ID del acelerometro conectado

				i2cBuffer = GetAccelID(&handlerAcelerometro);

				sprintf(bufferMsg,"\nEl ID del acelerometro conectado es: 0x%x",i2cBuffer);

				break;
			}//Fin del Case 'i'

			case 'm':{
				//En este caso se configura el modo del acelerometro
				//Si en StandBy o Measure
				mode ^= 1; //Esto va alternando de un modo a otro

				//Hacemos un switch que configure cada caso
				switch(mode){

				case 0:{
					//Estamos en el caso StandBy
					ChangeAccelMode(&handlerAcelerometro, ACCEL_MODE_STANDBY);
					measureRDY =0; //Bajamos la bandera del measureRDY
					break;
				}

				case 1:{
					//Estamos en el caso StandBy
					ChangeAccelMode(&handlerAcelerometro, ACCEL_MODE_MEASURE);
					break;
				}
				default:{
					__NOP();
					break;
				}
				}

				/*Ahora, leemos la configuracion en el acelerometro para comprobar
				 * que si haya quedado bien configurado el modo
				 */
				switch(GetAccelMode(&handlerAcelerometro)){

				case ACCEL_MODE_STANDBY:{

					sprintf(bufferMsg, "El acelerometro esta en modo StandBy\n");
					break;
				}
				case ACCEL_MODE_MEASURE:{

					sprintf(bufferMsg, "El acelerometro esta en modo Measure\n");
					measureRDY =1; //Confirmamos que el acelerometro si esta midiendo
					break;
				}
				default:{
					__NOP();
					break;
				}
			}

				break;
			}//Fin del Case 'm'

			case 'x':{

				//Revisamos si esta o no activo el modo de medicion
				if(measureRDY){
					//El modo de medicion esta activo

					accelX = GetAccelXDATA(&handlerAcelerometro);
					sprintf(bufferMsg,"La aceleración en X en este instante es: %.2f m/s^2\n",accelX);
				}
				else{
					//El modo de medicion esta desactivo
					sprintf(bufferMsg, "Por favor active el modo de medicion antes de realizar esta accion\n");
				}

				break;
			}//Fin del Case 'x'

			case 'y':{

				//Revisamos si esta o no activo el modo de medicion
				if(measureRDY){
					//El modo de medicion esta activo

					accelY = GetAccelYDATA(&handlerAcelerometro);
					sprintf(bufferMsg,"La aceleración en Y en este instante es: %.2f m/s^2\n",accelY);
				}
				else{
					//El modo de medicion esta desactivo
					sprintf(bufferMsg, "Por favor active el modo de medicion antes de realizar esta accion\n");
				}

				break;
			}//Fin del Case 'y'

			case 'z':{

				//Revisamos si esta o no activo el modo de medicion
				if(measureRDY){
					//El modo de medicion esta activo

					accelZ = GetAccelZDATA(&handlerAcelerometro);
					sprintf(bufferMsg,"La aceleración en Z en este instante es: %.2f m/s^2\n",accelZ);
				}
				else{
					//El modo de medicion esta desactivo
					sprintf(bufferMsg, "Por favor active el modo de medicion antes de realizar esta accion\n");
				}

				break;

			}//Fin del Case 'z'

			case 't':{

				//Revisamos si esta o no activo el modo de medicion
				if(measureRDY){
					//El modo de medicion esta activo

					/*Creamos un ciclo While que se va a ejecutar hasta que se
					 * guarde toda la cantidad de datos requeria que en este caso son
					 * 2000 por eje (2s de mediciones a 1kHz de muestreo)
					 */
					//Reiniciamos la variable
					numDato =0;
					sprintf(bufferMsg,"Se estan capturando los datos\n");
					writeMsg(&handlerUSART6, bufferMsg);

					while(numDato < 2000){

						//Este if me permite almacenar Si y solo si el timer da la señal
						if(flagMuestreo){

							//Medimos las aceleraciones en cada eje y las almacenamos
							//en su respectivo array

							accelX = GetAccelXDATA(&handlerAcelerometro);
							accelY = GetAccelYDATA(&handlerAcelerometro);
							accelZ = GetAccelZDATA(&handlerAcelerometro);

							aceleracionesX[numDato] = accelX;
							aceleracionesY[numDato] = accelY;
							aceleracionesZ[numDato] = accelZ;
							numDato++;

							flagMuestreo =0;
						}
					}

					/*Luego de almacenar los datos, los imprimimos en el puerto serial*/
					sprintf(bufferMsg,"Dato # ; X ; Y ; Z\n");
					writeMsg(&handlerUSART6, bufferMsg);

					numDato =0;
					while(numDato < 2000){

						sprintf(bufferMsg,"%u ; %.2f m/s^2 ; %.2f m/s^2 ; %.2f m/s^2\n",(numDato+1), aceleracionesX[numDato],aceleracionesY[numDato],aceleracionesZ[numDato]);
						writeMsg(&handlerUSART6, bufferMsg);
						numDato++;
					}

					sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
				}
				else{
					//El modo de medicion esta desactivo
					sprintf(bufferMsg, "Por favor active el modo de medicion antes de realizar esta accion\n");
				}

				break;

			}//Fin del Case 'z'

			default:{
				sprintf(bufferMsg,"Por favor ingrese un caracter valido\n");
				break;
			}

			}//Fin del switch case principal

			writeMsg(&handlerUSART6, bufferMsg);
			USARTDataRecieved = '\0';
		}//Fin del if principal

	}//Fin del while(1)

}//Fin de la funcion main

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
	handlerTimerBlinky.TIMx_Config.TIMx_speed				=BTIMER_SPEED_100us;
	handlerTimerBlinky.TIMx_Config.TIMx_period				=2500;   //Con esto, el blinky va a 250ms
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

	/*Configuramos el I2C del acelerometro*/
	//Configuramos los pines
	handlerI2cSCL.pGPIOx 								= GPIOB;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinNumber			= PIN_8;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_OPENDRAIN;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerI2cSCL.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF4;

	handlerI2cSDA.pGPIOx 								= GPIOB;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinNumber			= PIN_9;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_OPENDRAIN;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerI2cSDA.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF4;

	GPIO_Config(&handlerI2cSCL);
	GPIO_Config(&handlerI2cSDA);

	//Configuramos el I2C
	handlerAcelerometro.ptrI2Cx				=I2C1;
	handlerAcelerometro.modeI2C 			=I2C_MODE_FM;
	handlerAcelerometro.slaveAddress		=ACCEL_ADDRESS_SDO_HIGH;

	i2c_config(&handlerAcelerometro);

	//Configuramos el acelerometro
	Accel_Config(&handlerAcelerometro);

	/*Configuramos el timer de muestreo del Acelerometro*/
	handlerTimerMuestreo.ptrTIMx							=TIM3;
	handlerTimerMuestreo.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
	handlerTimerMuestreo.TIMx_Config.TIMx_speed				=BTIMER_SPEED_100us;
	handlerTimerMuestreo.TIMx_Config.TIMx_period			=10; //Con esto, el timer va a 1ms (1kHz)
	handlerTimerMuestreo.TIMx_Config.TIMx_interruptEnable	=BTIMER_INTERRUPT_ENABLE;

	BasicTimer_Config(&handlerTimerMuestreo);

	/*Configuramos los PWM*/
	//PWM_X
	handlerPinPWM_X.pGPIOx 								= GPIOA;
	handlerPinPWM_X.GPIO_PinConfig.GPIO_PinNumber		= PIN_0;
	handlerPinPWM_X.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerPinPWM_X.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPinPWM_X.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPinPWM_X.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;
	handlerPinPWM_X.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	handlerSeñalPWM_X.ptrTIMx							=TIM5;
	handlerSeñalPWM_X.config.channel 					=PWM_CHANNEL_1;
	handlerSeñalPWM_X.config.polarity 					=PWM_POLARITY_ACTIVE_HIGH;
	handlerSeñalPWM_X.config.prescaler 					=PWM_PRESCALER_1ms;
	handlerSeñalPWM_X.config.periodo 					=20; //Equivale a un periodo de 10ms
	handlerSeñalPWM_X.config.pulseWidth 				=10; //Equivale a un PW de 10 ms o un DutyCicle de 50%

	GPIO_Config(&handlerPinPWM_X);
	pwm_Config(&handlerSeñalPWM_X);

	//PWM_Y
	handlerPinPWM_Y.pGPIOx 								= GPIOA;
	handlerPinPWM_Y.GPIO_PinConfig.GPIO_PinNumber		= PIN_1;
	handlerPinPWM_Y.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerPinPWM_Y.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPinPWM_Y.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPinPWM_Y.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;
	handlerPinPWM_Y.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	handlerSeñalPWM_Y.ptrTIMx							=TIM5;
	handlerSeñalPWM_Y.config.channel 					=PWM_CHANNEL_2;
	handlerSeñalPWM_Y.config.polarity 					=PWM_POLARITY_ACTIVE_HIGH;
	handlerSeñalPWM_Y.config.prescaler 					=PWM_PRESCALER_1ms;
	handlerSeñalPWM_Y.config.periodo 					=20; //Equivale a un periodo de 10ms
	handlerSeñalPWM_Y.config.pulseWidth 				=10; //Equivale a un PW de 10 ms o un DutyCicle de 50%

	GPIO_Config(&handlerPinPWM_Y);
	pwm_Config(&handlerSeñalPWM_Y);

	//PWM_Z
	handlerPinPWM_Z.pGPIOx 								= GPIOA;
	handlerPinPWM_Z.GPIO_PinConfig.GPIO_PinNumber		= PIN_1;
	handlerPinPWM_Z.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerPinPWM_Z.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPinPWM_Z.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPinPWM_Z.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;
	handlerPinPWM_Z.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	handlerSeñalPWM_Z.ptrTIMx							=TIM5;
	handlerSeñalPWM_Z.config.channel 					=PWM_CHANNEL_3;
	handlerSeñalPWM_Z.config.polarity 					=PWM_POLARITY_ACTIVE_HIGH;
	handlerSeñalPWM_Z.config.prescaler 					=PWM_PRESCALER_1ms;
	handlerSeñalPWM_Z.config.periodo 					=20; //Equivale a un periodo de 10ms
	handlerSeñalPWM_Z.config.pulseWidth 				=10; //Equivale a un PW de 10 ms o un DutyCicle de 50%

	GPIO_Config(&handlerPinPWM_Z);
	pwm_Config(&handlerSeñalPWM_Z);

	//Activamos las señales y como estas comparten un mismo timer,
	//basta con hacer esto una vez
	startPwmSignal(&handlerSeñalPWM_X);

	//Dejamos las salidas deshabilitadas en un principio
	disableOutput(&handlerSeñalPWM_X);
	disableOutput(&handlerSeñalPWM_Y);
	disableOutput(&handlerSeñalPWM_Z);
}


/*Callbacks*/
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}

void BasicTimer3_Callback(void){
	flagMuestreo =1;

}

void usart2Rx_Callback(void){
	USARTDataRecieved =getRxData();
}
