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
#include <string.h>

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

#include "arm_math.h"

#include "ADXL345Driver.h"
#include "HD44780LCDDriver.h"

/*Macros utiles*/
#define MCO_HSI			00
#define MCO_LSE			01
#define MCO_PLL			11

#define MCO_PRE_NODIV	0
#define MCO_PRE_2		100
#define MCO_PRE_3		101
#define MCO_PRE_4		110
#define MCO_PRE_5		111

/*Handlers*/
GPIO_Handler_t handlerOnBoardLed			={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};
GPIO_Handler_t handlerMCO1					={0};
GPIO_Handler_t handlerAccelSDA				={0};
GPIO_Handler_t handlerAccelSCL				={0};

BasicTimer_Handler_t handlerTimerBlinky		={0};
BasicTimer_Handler_t handlerTimerMuestreo	={0};

USART_Handler_t handlerUSART				={0};

RTC_Handler_t handlerRTC					={0};

ADC_Config_t handlerADC						={0};

PWM_Handler_t handlerPWM					={0};

I2C_Handler_t handlerAcelerometro			={0};


/*Variables*/
//USART
char bufferMsg[100]			={0};
char recievedMsg[64]		={0};
uint8_t USARTDataRecieved 	=0;
uint8_t counterRecieved		=0;
uint8_t commandComplete		=0;
char cmd[64]				={0};
char string[64]				={0};
unsigned int firstParameter 	={0};
unsigned int secondParameter 	={0};
unsigned int thirdParameter 	={0};


//RTC
uint8_t hora 		=0;
uint8_t minutos		=0;
uint8_t segundos 	=0;
uint8_t am_pm 		=0;
uint8_t mes			=0;
uint8_t año			=0;
uint8_t fecha		=0;
uint8_t diaSemana	=0;

//MCO
uint8_t MCOclock 		=0;
uint8_t MCOpres 		=0;
uint8_t MCOpresMode		=0;
uint8_t MCOclockMode	=0;

//ADC
uint8_t  adcComplete		=0;
uint16_t adcData[2]			={0};
uint8_t  adcCounter			=0;
uint16_t dataCounter		=0;
uint16_t adcDataC1[256]		={0};
uint16_t adcDataC2[256]		={0};

//ACCEL
uint8_t flagMuestreo		=0;
uint16_t numDato			=0;
uint8_t accelDataRDY		=0;
int16_t dataX				=0;
int16_t dataY				=0;
int16_t dataZ				=0;
float aceleracionesX[1024] 	={0};
float aceleracionesY[1024] 	={0};
float aceleracionesZ[1024] 	={0};


/*Headers de funciones*/
void InitHardware(void);
void MCO1clock(uint8_t reloj);
void MCO1prescaler(uint8_t division);
void RecibirComando(void);
void parseCommands(char *ptrRecievedMsg);
void CambiarPeriodoMuestreo(uint16_t periodoMuestreo);
void ADCchannelData(uint8_t channel);
void DatosAcelerometro(char ejeAceleracion);

int main(void) {

	InitHardware();
	configPLL(HSI_100MHz_PLLN, HSI_100MHz_PLLP);
	ChangeUSART_BRR(&handlerUSART, 50);
	ChangeClockI2C(&handlerAcelerometro, 50);

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

		RecibirComando();



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
	handlerTimerBlinky.TIMx_Config.TIMx_speed				=BTIMER_100MHz_100us;
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

	/*Configuramos el ADC*/
	handlerADC.channel[0]			=ADC_CHANNEL_1;
	handlerADC.channel[1]			=ADC_CHANNEL_8;
	handlerADC.dataAlignment		=ADC_ALIGNMENT_RIGHT;
	handlerADC.samplingPeriod[0]	=ADC_SAMPLING_PERIOD_84_CYCLES;
	handlerADC.samplingPeriod[1]	=ADC_SAMPLING_PERIOD_84_CYCLES;
	handlerADC.resolution			=ADC_RESOLUTION_12_BIT;
	handlerADC.edgeType				=ADC_EDGETYPE_RISING;
	handlerADC.extSelect			=ADC_EXTSEL_TIMER3_CC1;

	ADC_ConfigMultichannel(&handlerADC, 2);

	//Configuramos el PWM del ADC
	handlerPWM.ptrTIMx							=TIM3;
	handlerPWM.config.channel 					=PWM_CHANNEL_1;
	handlerPWM.config.polarity 					=PWM_POLARITY_ACTIVE_HIGH;
	handlerPWM.config.prescaler 				=BTIMER_100MHz_1us;
	handlerPWM.config.periodo 					=60;
	handlerPWM.config.pulseWidth 				=30;

	pwm_Config(&handlerPWM);
	startPwmSignal(&handlerPWM);

	//Dejamos las salidas habilitadas en un principio
	enableOutput(&handlerPWM);

	/*Configuramos el I2C del acelerometro*/
	//Configuramos los pines
	handlerAccelSCL.pGPIOx 								= GPIOB;
	handlerAccelSCL.GPIO_PinConfig.GPIO_PinNumber		= PIN_8;
	handlerAccelSCL.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerAccelSCL.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_OPENDRAIN;
	handlerAccelSCL.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerAccelSCL.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;
	handlerAccelSCL.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF4;

	handlerAccelSDA.pGPIOx 								= GPIOB;
	handlerAccelSDA.GPIO_PinConfig.GPIO_PinNumber		= PIN_9;
	handlerAccelSDA.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerAccelSDA.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_OPENDRAIN;
	handlerAccelSDA.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerAccelSDA.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;
	handlerAccelSDA.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF4;

	GPIO_Config(&handlerAccelSCL);
	GPIO_Config(&handlerAccelSDA);

	//Configuramos el I2C
	handlerAcelerometro.ptrI2Cx				=I2C1;
	handlerAcelerometro.modeI2C 			=I2C_MODE_FM;
	handlerAcelerometro.slaveAddress		=ACCEL_ADDRESS_SDO_HIGH;

	i2c_config(&handlerAcelerometro);

	/*Configuramos el timer de muestreo del Acelerometro*/
	handlerTimerMuestreo.ptrTIMx							=TIM4;
	handlerTimerMuestreo.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
	handlerTimerMuestreo.TIMx_Config.TIMx_speed				=BTIMER_100MHz_100us;
	handlerTimerMuestreo.TIMx_Config.TIMx_period			=50; //Con esto, el timer va a 5ms (200Hz)
	handlerTimerMuestreo.TIMx_Config.TIMx_interruptEnable	=BTIMER_INTERRUPT_ENABLE;

	BasicTimer_Config(&handlerTimerMuestreo);

	/*Vamos a activar la unidad de punto flotante para ciertos calculos*/
	SCB->CPACR |= (0XF << 20);

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

	/*Cargamos la configuracion del reloj*/
	RCC->CFGR |= (reloj << RCC_CFGR_MCO1_Pos);

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

	/*Cargamos la configuracion*/
	RCC->CFGR |= (division << RCC_CFGR_MCO1PRE_Pos);

	/*Encendemos el PLL y el LSE*/
	//Encendemos el PLL
	RCC->CR |= RCC_CR_PLLON;

	//Encendemos el LSE
	RCC->BDCR |= RCC_BDCR_LSEON;
}

void RecibirComando(void){

	/*Primero revisamos si se ingresó un caracter diferente al nulo*/
	if(USARTDataRecieved != '\0'){
		//Si se cumple, almacenamos el caracter en un array
		recievedMsg[counterRecieved] = USARTDataRecieved;
		counterRecieved++;

		//Ahora, revisamos nuestro caracter que indica que el comando
		//se termino de mandar
		if(USARTDataRecieved == '$'){
			commandComplete = 1;

			//Ahora, agregamos un null para que el programa interprete el array
			//como un string
			recievedMsg[counterRecieved] = '\0';

			//Reiniciamos el counter
			counterRecieved =0;
		}

		USARTDataRecieved = '\0';
	}

	//Hacemos un analisis de la cadena de datos obtenida
	if(commandComplete){
		parseCommands(recievedMsg);

		commandComplete = 0;
	}
}

void parseCommands(char *ptrRecievedMsg){

	//Partimos el string y lo revisamos
	sscanf(ptrRecievedMsg,"%s %u %u %u %s",cmd, &firstParameter, &secondParameter, &thirdParameter, string);

	//Este primer comando imprime una lista con los otros comandos que tiene el equipo
	if(strcmp(cmd, "help") == 0){
		writeMsg(&handlerUSART, "Menu de comandos:\n");
		writeMsg(&handlerUSART, "1) help  --Imprime este menu.\n\n");
		writeMsg(&handlerUSART, "2) muestreoADC #valorMuestreo  --Se le ingresa el periodo de muestreo en micro segundos.\n\n");
		writeMsg(&handlerUSART, "3) datosADC #canal  --Pedimos los datos de un canal en especifico del ADC.\n\n");
		writeMsg(&handlerUSART, "4) MCO1preMode --Entramos en un modo que configura el prescaler del MCO1\npresionando la 'p'. Si se vuelve a enviar este comando, este modo finaliza.\n\n");
		writeMsg(&handlerUSART, "5) MCO1clockMode --Entramos en un modo que configura el reloj del MCO1\npresionando la 'c'. Si se vuelve a enviar este comando, este modo finaliza.\n\n");
		writeMsg(&handlerUSART, "6) datosACCEL 0 0 0 eje  --Obtenemos e imprimimos los datos del acelerometro en el eje indicado.\n\n");
	}

	//Procesamos el comando de muestreo del ADC
	else if(strcmp(cmd, "muestreoADC") == 0){
		CambiarPeriodoMuestreo(firstParameter);

		sprintf(bufferMsg, "El nuevo periodo de muestreo es: %u\n", firstParameter);
		writeMsg(&handlerUSART, bufferMsg);
	}

	//Procesamos el comando de datosADC
	else if(strcmp(cmd, "datosADC") == 0){

		sprintf(bufferMsg, "Se estan tomando los datos del canal: %u\n", firstParameter);
		writeMsg(&handlerUSART, bufferMsg);

		ADCchannelData(firstParameter);
	}

	//Procesamos el comando de MCO1pre
	else if (strcmp(cmd, "MCO1preMode") == 0){

		MCOpresMode ^=1;

		if(MCOpresMode){
			sprintf(bufferMsg, "Estamos en modo MCOpres\n");
			writeMsg(&handlerUSART, bufferMsg);
		}
		else{
			sprintf(bufferMsg, "Salimos del modo MCOpres\n");
			writeMsg(&handlerUSART, bufferMsg);
		}
		while(MCOpresMode == 1){

			/*Aqui comienza el codigo con los comandos por USART*/
			if (USARTDataRecieved != '\0') {

				if (USARTDataRecieved == 'p') {

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
				}

				else {
					sprintf(bufferMsg, "Tecla incorrecta. Presione la 'p'\n");
				}

				USARTDataRecieved = '\0';

				writeMsg(&handlerUSART, bufferMsg);
			}
		}
	}

	//Procesamos el comando de MCO1clock
	else if (strcmp(cmd, "MCO1clockMode") == 0){

		MCOclockMode ^=1;

		if(MCOclockMode){
			sprintf(bufferMsg, "Estamos en modo MCOclock\n");
			writeMsg(&handlerUSART, bufferMsg);
		}
		else{
			sprintf(bufferMsg, "Salimos del modo MCOclock\n");
			writeMsg(&handlerUSART, bufferMsg);
		}
		while(MCOclockMode == 1){

			/*Aqui comienza el codigo con los comandos por USART*/
			if (USARTDataRecieved != '\0') {

				if (USARTDataRecieved == 'c') {

					//Modificamos el reloj del MCO
					switch (MCOclock) {

					case 0: {
						//Estamos en el reloj HSI
						MCO1clock(MCO_HSI);
						sprintf(bufferMsg, "El reloj es el HSI\n");
						break;
					}

					case 1: {
						//Estamos en el reloj LSE
						MCO1clock(MCO_LSE);
						sprintf(bufferMsg, "El reloj es el LSE\n");
						break;
					}

					case 2: {
						//Estamos en el reloj PLL
						MCO1clock(MCO_PLL);
						sprintf(bufferMsg, "El reloj es el PLL\n");
						break;
					}

					default: {
						__NOP();
						break;
					}
					}

					MCOclock++;

					if (MCOclock > 2) {
						MCOclock = 0;
					}
				} else {
					sprintf(bufferMsg, "Tecla incorrecta. Presione la 'c'\n");
				}

				USARTDataRecieved = '\0';

				writeMsg(&handlerUSART, bufferMsg);
			}
		}
	}

	else if (strcmp(cmd, "datosACCEL") == 0){

		DatosAcelerometro(string[0]);

	}

}

/*Funcion para sacar la informacion de los canales*/
void ADCchannelData(uint8_t channel){

	switch(channel){
	case 1:{
		dataCounter =0;
		//Primero, almacenamos los datos en un array
		while(dataCounter < 256){

			if(adcComplete){
				//Si se completa la conversion, se almacena el canal respectivo
				adcDataC1[dataCounter] = adcData[0];
				dataCounter++;
				adcComplete =0;
			}
		}

		//Ahora, recorremos los datos y los mostramos en la terminal
		dataCounter =0;
		while(dataCounter < 256){

			sprintf(bufferMsg,"%u\n",adcDataC1[dataCounter]);
			writeMsg(&handlerUSART, bufferMsg);
			dataCounter++;
		}

		sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
		writeMsg(&handlerUSART, bufferMsg);

		break;
	}

	case 2:{
		dataCounter =0;
		while(dataCounter < 256){

			if(adcComplete){
				//Si se completa la conversion, se almacena el canal respectivo
				adcDataC2[dataCounter] = adcData[1];
				dataCounter++;
				adcComplete =0;
			}
		}

		//Ahora, recorremos los datos y los mostramos en la terminal
		dataCounter =0;
		while(dataCounter < 256){

			sprintf(bufferMsg,"%u\n", adcDataC2[dataCounter]);
			writeMsg(&handlerUSART, bufferMsg);
			dataCounter++;
		}

		sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
		writeMsg(&handlerUSART, bufferMsg);

		break;
	}
	default:{
		sprintf(bufferMsg, "Ingrese un valor de canal valido\n");
		writeMsg(&handlerUSART, bufferMsg);
		break;
	}
	}
}

/*Funcion para cambiar la frecuencia de muestreo del ADC*/
void CambiarPeriodoMuestreo(uint16_t periodoMuestreo){

	//Simplemente cambiamos el periodo del PWM relacionado
	//al muestreo del ADC

	//Pero antes, revisamos que dicho periodo no sea mayor
	//a al valor necesario para muestrear la frecuencia minima
	//Siguiendo que  la frecuencia de muestreo debe ser almenos
	//10 veces mayor que la frecuencia de la señal de entrada

	if(periodoMuestreo > 125){

		periodoMuestreo = 125;
	}
	else if(periodoMuestreo < 10){

		periodoMuestreo =10;
	}

	updatePeriod(&handlerPWM, periodoMuestreo);
	updatePulseWidth(&handlerPWM, periodoMuestreo/2);
}

void DatosAcelerometro(char ejeAceleracion){

	/*Primero hacemos que el acelerometro entre en modo Measure*/
	ChangeAccelMode(&handlerAcelerometro, ACCEL_MODE_MEASURE);

	/*Ahora, tomamos los datos*/
		/*Creamos un ciclo While que se va a ejecutar hasta que se
		 * guarde toda la cantidad de datos requeria que en este caso son
		 * 1024 por eje
		 */
		//Reiniciamos la variable
		numDato =0;
		sprintf(bufferMsg,"Se estan capturando los datos\n");
		writeMsg(&handlerUSART, bufferMsg);

		while(numDato < 1024){

			//Este if me permite almacenar Si y solo si el timer da la señal
			if(flagMuestreo){

				//Medimos las aceleraciones en cada eje y las almacenamos
				//en su respectivo array

				//Recibimos los datos del acelerometro
				dataX = GetAccelXDATA(&handlerAcelerometro);
				dataY = GetAccelYDATA(&handlerAcelerometro);
				dataZ = GetAccelZDATA(&handlerAcelerometro);

				//Almacenamos los datos en los arrays
				aceleracionesX[numDato] = dataX;
				aceleracionesY[numDato] = dataY;
				aceleracionesZ[numDato] = dataZ;
				numDato++;

				flagMuestreo =0;
			}
		}

		//Luego de almacenar los datos, los imprimimos en el puerto serial
		//Pero solo imprimimos los del eje que se indicó

		switch(ejeAceleracion){

		case 'x':{

			numDato =0;
			while(numDato < 1024){

				sprintf(bufferMsg,"%u %.2f\n",(numDato+1), aceleracionesX[numDato]);
				writeMsg(&handlerUSART, bufferMsg);
				numDato++;
			}

			sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		case 'y':{

			numDato =0;
			while(numDato < 1024){

				sprintf(bufferMsg,"%u %.2f\n",(numDato+1), aceleracionesY[numDato]);
				writeMsg(&handlerUSART, bufferMsg);
				numDato++;
			}

			sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		case 'z':{

			numDato =0;
			while(numDato < 1024){

				sprintf(bufferMsg,"%u %.2f\n",(numDato+1), aceleracionesZ[numDato]);
				writeMsg(&handlerUSART, bufferMsg);
				numDato++;
			}

			sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}
		default:{
			writeMsg(&handlerUSART, "Ingrese un caracter valido");
			break;
		}
		}

		//Subimos esta bandera indicando que hay datos para hacer la FFT
		accelDataRDY =1;

}

/*Callbacks*/

void usart2Rx_Callback(void){
	USARTDataRecieved =getRxData();
}

void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}

void BasicTimer4_Callback(void){
	flagMuestreo =1;

}

void adcComplete_Callback(void){
	adcData[adcCounter] = getADC();
	adcCounter++;
	if(adcCounter > 1){
		adcCounter =0;

		adcComplete = 1;
	}
}


