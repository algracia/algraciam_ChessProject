/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************

 ******************************************************************************
 */

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

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

#define ACCEL_DATASIZE			1024
#define FRECUENCIA_MUESTREO_FFT	200

/*Handlers*/
GPIO_Handler_t handlerOnBoardLed			={0};
GPIO_Handler_t handlerPinTX					={0};
GPIO_Handler_t handlerPinRX					={0};
GPIO_Handler_t handlerMCO1					={0};
GPIO_Handler_t handlerAccelSDA				={0};
GPIO_Handler_t handlerAccelSCL				={0};
GPIO_Handler_t handlerLCDSDA				={0};
GPIO_Handler_t handlerLCDSCL				={0};

BasicTimer_Handler_t handlerTimerBlinky		={0};
BasicTimer_Handler_t handlerTimerMuestreo	={0};
BasicTimer_Handler_t handlerTimerRefresco	={0};

USART_Handler_t handlerUSART				={0};

RTC_Handler_t handlerRTC					={0};

ADC_Config_t handlerADC						={0};

PWM_Handler_t handlerPWM					={0};

I2C_Handler_t handlerAcelerometro			={0};
I2C_Handler_t handlerLCD					={0};


/*Variables*/
//USART
char bufferMsg[100]				={0};
char recievedMsg[64]			={0};
uint8_t USARTDataRecieved 		=0;
uint8_t counterRecieved			=0;
uint8_t commandComplete			=0;
char cmd[64]					={0};
char string[64]					={0};
unsigned int firstParameter 	={0};
unsigned int secondParameter 	={0};
unsigned int thirdParameter 	={0};

//RTC
uint8_t hora 				=0;
uint8_t minutos				=0;
uint8_t segundos 			=0;
uint8_t am_pm 				=0;
uint8_t mes					=0;
uint8_t año					=0;
uint8_t dia					=0;
uint8_t diaSemana			=0;
char bufferDiaSemana[30] 	={0};

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
uint16_t periodoMuestreo =0;
float auxPeriodoMuestreo =0;

//ACCEL
uint8_t flagMuestreo						=0;
uint16_t numDato							=0;
uint8_t accelDataRDY						=0;
int16_t dataX								=0;
int16_t dataY								=0;
int16_t dataZ								=0;
float32_t aceleracionesX[ACCEL_DATASIZE] 	={0};
float32_t aceleracionesY[ACCEL_DATASIZE] 	={0};
float32_t aceleracionesZ[ACCEL_DATASIZE] 	={0};

//FFT
arm_rfft_fast_instance_f32 config_Rfft_fast_32;
arm_status status = ARM_MATH_ARGUMENT_ERROR;
arm_status statusInitFFT = ARM_MATH_ARGUMENT_ERROR;
uint16_t fftSize = 1024;
float32_t transformedSignal[ACCEL_DATASIZE] 		={0};
float32_t abslotueTransformedSignal[ACCEL_DATASIZE] ={0};
float32_t amplitudMax			=0;
uint16_t indiceMax				=0;
float frecuenciaSeñal			=0;

//LCD
uint8_t flagRefresco	=0;


/*Headers de funciones*/
void InitHardware(void);
void MCO1clock(uint8_t reloj);
void MCO1prescaler(uint8_t division);
void RecibirComando(void);
void parseCommands(char *ptrRecievedMsg);
void CambiarPeriodoMuestreo(uint16_t periodoMuestreo);
void ADCchannelData(uint8_t channel);
void DatosAcelerometro(char ejeAceleracion);
void HacerFFT(float32_t *ptrDatosFFT);
void ConfigurarRTC_formato(uint8_t rtcFormato);
void ConfigurarRTC_hora(uint8_t rtcHora, uint8_t rtcMinutos, uint8_t rtcSegundos, uint8_t rtcAm_pm);
void ConfigurarRTC_fecha(uint8_t rtcDia, uint8_t rtcMes, uint8_t rtcAño, uint8_t rtcDiaSemana);
void StringDiaSemana(uint8_t rtcDiaSemana);

int main(void) {

	InitHardware();
	configPLL(HSI_100MHz_PLLN, HSI_100MHz_PLLP);
	ChangeUSART_BRR(&handlerUSART, 100);
	ChangeClockI2C(&handlerAcelerometro, 50);
	ChangeClockI2C(&handlerLCD, 50);

	/* Loop forever*/
	while (1) {

		/*Ejecutamos la funcion para recibir comandos
		 * e internamente ella ejecuta todas las demas funciones
		 */
		RecibirComando();

		/*Medimos los datos del RTC*/
		hora		=getRTChours();
		minutos		=getRTCminutes();
		segundos 	=getRTCseconds();
		am_pm		=getRTCAmPm();
		mes			=getRTCmonth();
		año			=getRTCyear();
		dia 		=getRTCdate();
		diaSemana 	=getRTCweekDay();

//		/*Ahora, enviamos dichos datos a la LCD*/
//		if(flagRefresco){
//			//los valores se actualizan solo cada vez que el timer lo indica
//
//			/*Primero mostramos la hora*/
//			//Ubicamos el cursor
//			MoveLCDCursor(&handlerLCD, 1, 0);
//
//			if(handlerRTC.formatoHora == RTC_FORMATO_12HORAS){
//				/*Enviamos los datos de la hora*/
//				if (handlerRTC.am_pm == 0) {
//					sprintf(bufferMsg, "Hora -> %u:%u:%u %s", hora, minutos, segundos, "am");
//					WriteLCDMsg(&handlerLCD, bufferMsg);
//
//				} else {
//					sprintf(bufferMsg, "Hora -> %u:%u:%u %s", hora, minutos, segundos, "pm");
//					WriteLCDMsg(&handlerLCD, bufferMsg);
//				}
//			}
//			else{
//				sprintf(bufferMsg, "Hora -> %u:%u:%u", hora, minutos, segundos);
//				WriteLCDMsg(&handlerLCD, bufferMsg);
//			}
//
//			/*Ahora, enviamos los datos de la fecha*/
//			//Ubicamos el cursor
//			MoveLCDCursor(&handlerLCD, 2, 0);
//
//			StringDiaSemana(diaSemana);
//
//			sprintf(bufferMsg, "Fecha -> %u %u %u", dia, mes, año);
//			WriteLCDMsg(&handlerLCD, bufferMsg);
//
//			//Ubicamos el cursor
//			MoveLCDCursor(&handlerLCD, 3, 0);
//
//			sprintf(bufferMsg, "Dia -> %s",bufferDiaSemana);
//			WriteLCDMsg(&handlerLCD, bufferMsg);
//
//			/*Enviamos cual es el formato del RTC*/
//			//Ubicamos el cursor
//			MoveLCDCursor(&handlerLCD, 4, 0);
//
//			if(handlerRTC.formatoHora == RTC_FORMATO_12HORAS){
//				WriteLCDMsg(&handlerLCD, "Formato 12hrs");
//			}
//			else{
//				WriteLCDMsg(&handlerLCD, "Formato 24hrs");
//			}
//
//			//Bajamos la bandera de refresco
//			flagRefresco =0;
//		}

	}
}

void InitHardware(void){

	/*Configuramos el Blinky*/
	handlerOnBoardLed.pGPIOx 								= GPIOH;
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_1;
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
	handlerPinTX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_11;
	handlerPinTX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinTX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF8;

	handlerPinRX.pGPIOx 								= GPIOA;
	handlerPinRX.GPIO_PinConfig.GPIO_PinNumber 			= PIN_12;
	handlerPinRX.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinRX.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF8;

	//Cargamos las configuraciones
	GPIO_Config(&handlerPinTX);
	GPIO_Config(&handlerPinRX);

	//Configuramos el USART
	handlerUSART.ptrUSARTx 							= USART6;
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
	handlerADC.channel[0]			=ADC_CHANNEL_0;
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

	//Configuramos el timer de muestreo del Acelerometro
	handlerTimerMuestreo.ptrTIMx							=TIM4;
	handlerTimerMuestreo.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
	handlerTimerMuestreo.TIMx_Config.TIMx_speed				=BTIMER_100MHz_100us;
	handlerTimerMuestreo.TIMx_Config.TIMx_period			=50; //Con esto, el timer va a 5ms (200Hz)
	handlerTimerMuestreo.TIMx_Config.TIMx_interruptEnable	=BTIMER_INTERRUPT_ENABLE;

	BasicTimer_Config(&handlerTimerMuestreo);

	/*Configuramos el I2C de la pantalla LCD*/
	//Configuramos los pines
	handlerLCDSCL.pGPIOx 								= GPIOB;
	handlerLCDSCL.GPIO_PinConfig.GPIO_PinNumber			= PIN_10;
	handlerLCDSCL.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerLCDSCL.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_OPENDRAIN;
	handlerLCDSCL.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerLCDSCL.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerLCDSCL.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF4;

	handlerLCDSDA.pGPIOx 								= GPIOB;
	handlerLCDSDA.GPIO_PinConfig.GPIO_PinNumber			= PIN_3;
	handlerLCDSDA.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerLCDSDA.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_OPENDRAIN;
	handlerLCDSDA.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerLCDSDA.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerLCDSDA.GPIO_PinConfig.GPIO_PinAltFunMode 	= AF9;

	GPIO_Config(&handlerLCDSCL);
	GPIO_Config(&handlerLCDSDA);

	//Configuramos el I2C
	handlerLCD.ptrI2Cx			=I2C2;
	handlerLCD.modeI2C 			=I2C_MODE_SM;
	handlerLCD.slaveAddress		=LCD_ADDRESS_A1JUMPER;

	i2c_config(&handlerLCD);

	//Configuramos la LCD
	LCD_Config(&handlerLCD);

	//Configuramos un timer para el refresco de la LCD
	handlerTimerRefresco.ptrTIMx							=TIM5;
	handlerTimerRefresco.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
	handlerTimerRefresco.TIMx_Config.TIMx_speed				=BTIMER_100MHz_100us;
	handlerTimerRefresco.TIMx_Config.TIMx_period			=10000; //Con esto, el timer va a 1s
	handlerTimerRefresco.TIMx_Config.TIMx_interruptEnable	=BTIMER_INTERRUPT_ENABLE;

	BasicTimer_Config(&handlerTimerRefresco);

	/*Vamos a activar la unidad de punto flotante para ciertos calculos*/
	SCB->CPACR |= (0XF << 20);

}

/*Funcion para configurar el reloj del MCO1*/
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

/*Funcion para configurar el prescaler del MCO1*/
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

/*Funcion para recibir los comandos por USART*/
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

/*Funcion que analiza y ejecuta los comandos*/
void parseCommands(char *ptrRecievedMsg){

	//Partimos el string y lo revisamos
	sscanf(ptrRecievedMsg,"%s %u %u %u %s",cmd, &firstParameter, &secondParameter, &thirdParameter, string);

	/*Este primer comando imprime una lista con los otros comandos que tiene el equipo*/
	if(strcmp(cmd, "help") == 0){
		writeMsg(&handlerUSART, "Menu de comandos:\n");
		writeMsg(&handlerUSART, "1) help  --Imprime este menu.\n\n");
		writeMsg(&handlerUSART, "2) muestreoADC #valorMuestreo  --Se le ingresa la frecuencia de muestreo en Hz.\n\n");
		writeMsg(&handlerUSART, "3) datosADC #canal --Pedimos los datos de un canal en especifico del ADC.\n\n");
		writeMsg(&handlerUSART, "4) MCO1pre #factor del prescaler --Se configura el prescaler del MCO1 segun el valor que se ingrese.\n\n");
		writeMsg(&handlerUSART, "5) MCO1clock 0 0 0 reloj --Se configura el reloj del MCO1 segun el nombre de reloj que se ingrese.\n\n");
		writeMsg(&handlerUSART, "6) datosACCEL 0 0 0 eje  --Obtenemos e imprimimos los datos del acelerometro en el eje indicado.\n\n");
		writeMsg(&handlerUSART, "7) FFT 0 0 0 eje --Hacemos la transformada de fourier de los datos en el eje que queramos\n\n");
		writeMsg(&handlerUSART, "8) RTC_formato #formato --Cargamos en numero si el formato de las horas va a ser de 12 o 24 (es de 12hrs por defecto)\n\n");
		writeMsg(&handlerUSART, "9) RTC_hora #hora #minutos #segundos am_pm --Cargamos la hora en el RTC(En el formato 24hrs el am_pm no afecta)\n\n");
		writeMsg(&handlerUSART, "10) RTC_fecha #dia #mes #año diaSemana --Cargamos la fecha en el RTC\n\n");
		writeMsg(&handlerUSART, "11) RTC_enConsola --Imprime en consola todos los datos instantaneos del RTC \n\n");
	}

	/*Procesamos el comando de muestreo del ADC*/
	else if(strcmp(cmd, "muestreoADC") == 0){
		CambiarPeriodoMuestreo(firstParameter);

	}

	/*Procesamos el comando de datosADC*/
	else if(strcmp(cmd, "datosADC") == 0){

		//Habilitamos el PWM
		enableOutput(&handlerPWM);

		sprintf(bufferMsg, "Se estan tomando los datos del canal: %u\n", firstParameter);
		writeMsg(&handlerUSART, bufferMsg);

		ADCchannelData(firstParameter);
	}

	/*Procesamos el comando de MCO1pre*/
	else if (strcmp(cmd, "MCO1pre") == 0){

		//Modificamos el prescaler del MCO
		switch (firstParameter) {

		case 0: {
			//El prescaler es de cero
			MCO1prescaler(MCO_PRE_NODIV);
			sprintf(bufferMsg, "El MCO no tiene prescaler\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		case 2: {
			//El prescaler es de 2
			MCO1prescaler(MCO_PRE_2);
			sprintf(bufferMsg, "El MCO tiene prescaler de 2\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		case 3: {
			//El prescaler es de 3
			MCO1prescaler(MCO_PRE_3);
			sprintf(bufferMsg, "El MCO tiene prescaler de 3\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		case 4: {
			//El prescaler es de 4
			MCO1prescaler(MCO_PRE_4);
			sprintf(bufferMsg, "El MCO tiene prescaler de 4\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		case 5: {
			//El prescaler es de 5
			MCO1prescaler(MCO_PRE_5);
			sprintf(bufferMsg, "El MCO tiene prescaler de 5\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}

		default: {
			sprintf(bufferMsg, "Ingrese un valor valido de prescaler\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}
		}
	}


	/*Procesamos el comando de MCO1clock*/
	else if (strcmp(cmd, "MCO1clock") == 0){

		//Modificamos el reloj del MCO
		if (strcmp(string, "HSI") == 0) {
			//Estamos en el reloj HSI
			MCO1clock(MCO_HSI);
			sprintf(bufferMsg, "El reloj es el HSI\n");
			writeMsg(&handlerUSART, bufferMsg);
		}

		else if (strcmp(string, "LSE") == 0) {
			//Estamos en el reloj LSE
			MCO1clock(MCO_LSE);
			sprintf(bufferMsg, "El reloj es el LSE\n");
			writeMsg(&handlerUSART, bufferMsg);
		}

		else if (strcmp(string, "PLL") == 0) {
			//Estamos en el reloj PLL
			MCO1clock(MCO_PLL);
			sprintf(bufferMsg, "El reloj es el PLL\n");
			writeMsg(&handlerUSART, bufferMsg);
		}

		else {
			sprintf(bufferMsg, "Ingrese un nombre de reloj valido\n");
			writeMsg(&handlerUSART, bufferMsg);
		}

	}


	/*Procesamos el comando de datosACCEL*/
	else if (strcmp(cmd, "datosACCEL") == 0){
		//Ejecuatamos la funcion para tomar datos del accel
		//en el eje indicado en el string
		DatosAcelerometro(string[0]);
	}

	/*Procesamos el comando de FFT*/
	else if (strcmp(cmd, "FFT") == 0){

		if (strcmp(string, "x") == 0){
			//Se aplica la FFT a los datos del eje x
			writeMsg(&handlerUSART, "Se le va a hacer la FFT a los datos en el eje X\n");
			HacerFFT(aceleracionesX);
		}

		else if (strcmp(string, "y") == 0) {
			//Se aplica la FFT a los datos del eje y
			writeMsg(&handlerUSART, "Se le va a hacer la FFT a los datos en el eje Y\n");
			HacerFFT(aceleracionesY);
		}

		else if (strcmp(string, "z") == 0){
			//Se aplica la FFT a los datos del eje z
			writeMsg(&handlerUSART, "Se le va a hacer la FFT a los datos en el eje Z\n");
			HacerFFT(aceleracionesZ);
		}

	}

	/*Procesamos el comando de RTC_formato*/
	else if (strcmp(cmd, "RTC_formato") == 0){

		//Hacemos un switch que revisa cual es el formato
		//Indicado por el usuario
		switch(firstParameter){

		case 12:{
			//Formato 12hrs
			 ConfigurarRTC_formato(RTC_FORMATO_12HORAS);
			 writeMsg(&handlerUSART, "El RTC esta en formato 12hrs\n");
			 break;
		}
		case 24:{
			//Formato 24hrs
			 ConfigurarRTC_formato(RTC_FORMATO_24HORAS);
			 writeMsg(&handlerUSART, "El RTC esta en formato 24hrs\n");
			 break;
		}
		default:{
			writeMsg(&handlerUSART, "Ingrese un formato valido\n");
			break;
		}
		}
	}

	/*Procesamos el comando de RTC_hora*/
	else if (strcmp(cmd, "RTC_hora") == 0){

//		/*Limpiamos la LCD*/
//		WriteLCDInstruction(&handlerLCD, LCD_INSTRUCTION_DISPLAY_CLEAN);

		if(handlerRTC.formatoHora == RTC_FORMATO_12HORAS){
			if (strcmp(string, "am") == 0) {

				ConfigurarRTC_hora(firstParameter, secondParameter,thirdParameter, RTC_AM);
				writeMsg(&handlerUSART, "La hora se configuró adecuadamente\n");
			}

			else if (strcmp(string, "pm") == 0) {

				ConfigurarRTC_hora(firstParameter, secondParameter,thirdParameter, RTC_PM);
				writeMsg(&handlerUSART, "La hora se configuró adecuadamente\n");
			}

			else {
				writeMsg(&handlerUSART, "Ingrese un valor valido de am o pm\n");
			}
		}
		else{
			ConfigurarRTC_hora(firstParameter, secondParameter,thirdParameter, RTC_AM);
			writeMsg(&handlerUSART, "La hora se configuró adecuadamente\n");
		}
	}

	/*Procesamos el comando de RTC_fecha*/
	else if (strcmp(cmd, "RTC_fecha") == 0){

//		/*Limpiamos la LCD*/
//		WriteLCDInstruction(&handlerLCD, LCD_INSTRUCTION_DISPLAY_CLEAN);

		if (strcmp(string, "lunes") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, LUNES);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else if (strcmp(string, "martes") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, MARTES);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else if (strcmp(string, "miercoles") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, MIERCOLES);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else if (strcmp(string, "jueves") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, JUEVES);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else if (strcmp(string, "viernes") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, VIERNES);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else if (strcmp(string, "sabado") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, SABADO);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else if (strcmp(string, "domingo") == 0){
			ConfigurarRTC_fecha(firstParameter, secondParameter, thirdParameter, DOMINGO);
			writeMsg(&handlerUSART, "La fecha se configuró adecuadamente\n");
		}

		else{
			writeMsg(&handlerUSART, "Ingrese un dia de la semana valido\n");
		}
	}

	/*Procesamos el comando de RTC_enConsola*/
	else if (strcmp(cmd, "RTC_enConsola") == 0){

		if(handlerRTC.formatoHora == RTC_FORMATO_12HORAS){
			/*Enviamos los datos de la hora*/
			if (handlerRTC.am_pm == 0) {
				sprintf(bufferMsg, "Hora -> %u:%u:%u %s\n", hora, minutos, segundos, "am");
				writeMsg(&handlerUSART, bufferMsg);
			} else {
				sprintf(bufferMsg, "Hora -> %u:%u:%u %s\n", hora, minutos, segundos, "pm");
				writeMsg(&handlerUSART, bufferMsg);
			}
		}
		else{
			sprintf(bufferMsg, "Hora -> %u:%u:%u\n", hora, minutos, segundos);
			writeMsg(&handlerUSART, bufferMsg);
		}

		/*Ahora, enviamos los datos de la fecha*/
		StringDiaSemana(diaSemana);

		sprintf(bufferMsg,"Fecha -> %u %u %u %s\n",dia,mes,año,bufferDiaSemana);
		writeMsg(&handlerUSART, bufferMsg);

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

	//Apagamos el PWM
	disableOutput(&handlerPWM);
}

/*Funcion para cambiar la frecuencia de muestreo del ADC*/
void CambiarPeriodoMuestreo(uint16_t frecuenciaMuestreo){

	//Simplemente cambiamos el periodo del PWM relacionado
	//al muestreo del ADC

	//Pero antes, revisamos que dicho periodo no sea mayor
	//a al valor necesario para muestrear la frecuencia minima
	//Siguiendo que  la frecuencia de muestreo debe ser almenos
	//10 veces mayor que la frecuencia de la señal de entrada


	auxPeriodoMuestreo = (1000000/frecuenciaMuestreo) ;

	periodoMuestreo = (uint16_t) auxPeriodoMuestreo;

	if(periodoMuestreo > 125){

		periodoMuestreo = 125;
	}
	else if(periodoMuestreo < 10){

		periodoMuestreo =10;
	}

	updatePeriod(&handlerPWM, periodoMuestreo);
	updatePulseWidth(&handlerPWM, periodoMuestreo/2);

	sprintf(bufferMsg, "La frecuencia de muestreo del ADC será: %u Hz\n ",frecuenciaMuestreo);
	writeMsg(&handlerUSART, bufferMsg);

	sprintf(bufferMsg, "que equivale a un periodo de muestreo aproximado de: %u us\n",periodoMuestreo);
	writeMsg(&handlerUSART, bufferMsg);
}

/*Funcion para extraer datos del acelerometro*/
void DatosAcelerometro(char ejeAceleracion){

	/*Primero hacemos que el acelerometro entre en modo Measure*/
	ChangeAccelMode(&handlerAcelerometro, ACCEL_MODE_MEASURE);

	/*Ahora, tomamos los datos*/
		/*Creamos un ciclo While que se va a ejecutar hasta que se
		 * guarde toda la cantidad de datos requeria
		 */
		//Reiniciamos la variable
		numDato =0;
		sprintf(bufferMsg,"Se estan capturando los datos\n");
		writeMsg(&handlerUSART, bufferMsg);

		while(numDato < ACCEL_DATASIZE){

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
			while(numDato < ACCEL_DATASIZE){

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
			while(numDato < ACCEL_DATASIZE){

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
			while(numDato < ACCEL_DATASIZE){

				sprintf(bufferMsg,"%u %.2f\n",(numDato+1), aceleracionesZ[numDato]);
				writeMsg(&handlerUSART, bufferMsg);
				numDato++;
			}

			sprintf(bufferMsg, "Los datos se imprimieron exitosamente\n");
			writeMsg(&handlerUSART, bufferMsg);
			break;
		}
		default:{
			writeMsg(&handlerUSART, "Ingrese un caracter valido\n");
			break;
		}
		}

		//Subimos esta bandera indicando que hay datos para hacer la FFT
		accelDataRDY =1;

}

/*Funcion para ejecutar la FFT*/
void HacerFFT(float32_t *ptrDatosFFT){

	/*Primero verificamos si hay datos del acelerometro para aplicarle la transformada*/
	if(accelDataRDY){

		//Ahora inicializamos la transformada
		statusInitFFT = arm_rfft_fast_init_f32(&config_Rfft_fast_32, fftSize);

		if(statusInitFFT == ARM_MATH_SUCCESS){
			writeMsg(&handlerUSART, "Initialization ... SUCESS!\n");
		}

		//Revisamos si la FFT se inicializo correctamente
		if(statusInitFFT == ARM_MATH_SUCCESS){
			//Calculamos la transformada
			arm_rfft_fast_f32(&config_Rfft_fast_32, ptrDatosFFT, transformedSignal, 0);

			//Aplicamos valor absoluto para extraer el modulo de las partes imaginarias
			arm_abs_f32(transformedSignal, abslotueTransformedSignal, fftSize);

			uint16_t j =1;

			for(uint16_t i = 1; i < fftSize; i++){

				if(i % 2){
					//Tomamos solo los valores impares por que solo nos interesa la mitad de estos
					sprintf(bufferMsg, "%#.6f\n", 2*abslotueTransformedSignal[i]);
					writeMsg(&handlerUSART, bufferMsg);

					//Ahora, hallamos la frecuencia fundamental de la señal
					if(abslotueTransformedSignal[i] > amplitudMax ){
						amplitudMax = abslotueTransformedSignal[i];
						indiceMax = j;
					}
					i++;
					j++;
				}
			}

			//La resolucion en la frecuencia esta dada por FRECUENCIA_MUESTREO_FFT / fftSize
			//Es decir, este valor, es el minimo cambio en la frecuencia que se puede distinguir
			//Por ende, al multiplicarlo por el indice con la amplitud maxima, obtenemos la frecuencia de la señal
			frecuenciaSeñal = (FRECUENCIA_MUESTREO_FFT / (float) fftSize) * (float) indiceMax;
			sprintf(bufferMsg, "La frecuencia fundamental de la señal es: %.2f Hz\n",frecuenciaSeñal);
			writeMsg(&handlerUSART, bufferMsg);
		}
		else{
			writeMsg(&handlerUSART, "Transformada no inicializada correctamente\n");
		}
	}
	else{
		writeMsg(&handlerUSART, "No hay datos para hacer la FFT\n");
	}
}

/*Funcion para actualizar el formato de horas del RTC*/
void ConfigurarRTC_formato(uint8_t rtcFormato){

	//Actualizamos loa valores del RTC
	handlerRTC.formatoHora	 	=rtcFormato;

	configRTC(&handlerRTC);
}

/*Funcion para actualizar la hora del RTC*/
void ConfigurarRTC_hora(uint8_t rtcHora, uint8_t rtcMinutos, uint8_t rtcSegundos, uint8_t rtcAm_pm){

	//Actualizamos loa valores del RTC
	handlerRTC.hora 	= rtcHora;
	handlerRTC.minutos 	= rtcMinutos;
	handlerRTC.segundos = rtcSegundos;
	handlerRTC.am_pm 	= rtcAm_pm;

	ChangeRTChour(&handlerRTC);
}

/*Funcion para actualizar la fecha del RTC*/
void ConfigurarRTC_fecha(uint8_t rtcDia, uint8_t rtcMes, uint8_t rtcAño, uint8_t rtcDiaSemana){

	//Actualizamos loa valores del RTC
	handlerRTC.fecha	 	= rtcDia;
	handlerRTC.mes 			= rtcMes;
	handlerRTC.año 			= rtcAño;
	handlerRTC.diaSemana 	= rtcDiaSemana;

	ChangeRTCdate(&handlerRTC);
}

/*Funcion para tomar el valor binario de los dias de la semana
 * y convertirlos en strings para imprimir por USART
 */
void StringDiaSemana(uint8_t rtcDiaSemana){

	if(rtcDiaSemana == LUNES){
		sprintf(bufferDiaSemana,"lunes");
	}

	else if(rtcDiaSemana == MARTES){
		sprintf(bufferDiaSemana,"martes");
	}

	else if(rtcDiaSemana == MIERCOLES){
		sprintf(bufferDiaSemana,"miercoles");
	}

	else if(rtcDiaSemana == JUEVES){
		sprintf(bufferDiaSemana,"jueves");
	}

	else if(rtcDiaSemana == VIERNES){
		sprintf(bufferDiaSemana,"viernes");
	}

	else if(rtcDiaSemana == SABADO){
		sprintf(bufferDiaSemana,"sabado");
	}

	else if(rtcDiaSemana == DOMINGO){
		sprintf(bufferDiaSemana,"domingo");
	}

}

/*Callbacks*/

void usart6Rx_Callback(void){
	USARTDataRecieved =getRxData();
}

void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}

void BasicTimer4_Callback(void){
	flagMuestreo =1;

}

void BasicTimer5_Callback(void){
	flagRefresco =1;

}

void adcComplete_Callback(void){
	adcData[adcCounter] = getADC();
	adcCounter++;
	if(adcCounter > 1){
		adcCounter =0;

		adcComplete = 1;
	}
}


