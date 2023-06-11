/*
 * RTCDriver.c
 *
 *  Created on: Jun 4, 2023
 *      Author: algraciam
 */

#include <stm32f4xx.h>
#include "RTCDriver.h"

uint8_t decenas =0;
uint8_t unidades =0;


void configRTC(RTC_Handler_t *ptrRTCHandler){

	/*0 Verificamos que los datos ingresados en el main tiene sentido
	 * y en caso de no tenerlo, los ajustamos
	 */
	/*Revisamos la hora*/
	if(ptrRTCHandler->formatoHora == RTC_FORMATO_12HORAS){
		//Si se seleccionó el formato 12 horas

		if(ptrRTCHandler->hora > 12){
			ptrRTCHandler->hora = 12;
			//Con esto no permitimos que, en este formato,
			//la no hora no se pase de las 12
		}
		else if(ptrRTCHandler->hora < 1){
			ptrRTCHandler->hora = 1;
			//Por otro lado, tampoco permitimos que el
			//valor baje de 1
		}
	}
	else{
		//En este caso, estamos en el formato 24 horas
		//Solo nos interesa si las horas se pasan de 23
		if(ptrRTCHandler->hora > 23){
			ptrRTCHandler->hora = 23;
		}
	}

	/*Revisamos los minutos*/
	if(ptrRTCHandler->minutos > 60){
		ptrRTCHandler->minutos = 60;
	}

	/*Revisamos los segundos*/
	if(ptrRTCHandler->segundos > 60){
		ptrRTCHandler->segundos = 60;
	}

	/*Revisamos los meses*/
	if(ptrRTCHandler->mes > 12){
		ptrRTCHandler->mes = 12;
	}
	else if(ptrRTCHandler->mes < 1){
		ptrRTCHandler->mes = 1;
	}

	/*Revisamos los dias*/
	if(ptrRTCHandler->fecha > 31){
		ptrRTCHandler->fecha = 31;
	}
	else if(ptrRTCHandler->fecha < 1){
		ptrRTCHandler->fecha = 1;
	}


	/*1. Hay que desactivar las protecciones de escritura
	 * en los registros de este periferico
	 */
	/*1a. Debemos habilitar el periferico del Power Control*/
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	/*1b. habilitamos la escritura en el RTC*/
	PWR->CR |= PWR_CR_DBP;

	/*1c. Encendemos el reloj LSE*/
	RCC->BDCR |= RCC_BDCR_LSEON;

	/*1d. Revisamos que la señal del reloj sea estable*/
	while(!(RCC->BDCR & RCC_BDCR_LSERDY)){
		__NOP();
	}

	/*1e. Seleccionamos la fuente de reloj del RTC*/
	//Limpiamos la parte del registro que nos interesa
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;

	//La fuente de reloj será el LSE de 32 768 Hz
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;

	/*1f. Activamos la señal de reloj hacia el RTC*/
	RCC->BDCR |= RCC_BDCR_RTCEN;

	/*1g. Escribimos la claves de seguridad en el WPR*/

	//Escribimos la primera clave
	RTC->WPR = 0xCA;
	//Escribimos la segunda clave
	RTC->WPR = 0x53;

	/*1h. Activamos el bit de bypass de los shadow registers*/
	RTC->CR |= RTC_CR_BYPSHAD;

	/*2. Configuramos la hora*/
	/*2a. Indicamos que estamos en modo de inicializacion*/
	RTC->ISR |= RTC_ISR_INIT;

	//Revisamos la bandera del modo de inicializacion
	while(!(RTC->ISR & RTC_ISR_INITF)){
		__NOP();
	}

	//En este punto se podrian cambiar los valores del RTC_PRE
	//pero con los que viene por defecto, ya el RTC queda con la
	//frecuencia necesitada (ojo, esto usando como reloj el LSE)

	RTC->PRER =0;

	RTC->PRER |= (0x7F << RTC_PRER_PREDIV_A_Pos);
	RTC->PRER |= (0xFF << RTC_PRER_PREDIV_S_Pos);

	/*2b. Fijamos el formato de la hora*/
	if(ptrRTCHandler->formatoHora == RTC_FORMATO_12HORAS){
		//Estamos en formato de 12 horas
		RTC->CR |= RTC_CR_FMT;

		//Ahora, indicamos si va a ser PM o AM
		if(ptrRTCHandler->am_pm == RTC_PM){
			//Es PM
			RTC->TR |= RTC_TR_PM;
		}
		else{
			//Es AM
			RTC->TR &= ~RTC_TR_PM;
		}

	}else{
		//Estamos en formato de 24 horas
		RTC->CR &=  ~RTC_CR_FMT;
		RTC->TR &= ~RTC_TR_PM;
	}

	/*2c. Cargamos la hora*/
	//Primero dividimos las decenas de las unidades
	DividirDecenasYunidades(ptrRTCHandler->hora);

	//Cargamos la decenas de la hora
	RTC->TR &= ~RTC_TR_HT; //Limpiamos

	RTC->TR |= (decenas << RTC_TR_HT_Pos);

	//Cargamos las unidades de la hora
	RTC->TR &= ~RTC_TR_HU; //Limpiamos

	RTC->TR |= (unidades << RTC_TR_HU_Pos);

	/*2d. Cargamos los minutos*/
	//Nuevamente dividimos las decenas de las unidades
	DividirDecenasYunidades(ptrRTCHandler->minutos);

	//Cargamos la decenas de los minutos
	RTC->TR &= ~RTC_TR_MNT; //Limpiamos

	RTC->TR |= (decenas << RTC_TR_MNT_Pos);

	//Cargamos las unidades de los minutos
	RTC->TR &= ~RTC_TR_MNU; //Limpiamos

	RTC->TR |= (unidades << RTC_TR_MNU_Pos);

	/*2e. Cargamos los segundos*/
	//Nuevamente dividimos las decenas de las unidades
	DividirDecenasYunidades(ptrRTCHandler->segundos);

	//Cargamos la decenas de los segundos
	RTC->TR &= ~RTC_TR_ST; //Limpiamos

	RTC->TR |= (decenas << RTC_TR_ST_Pos);

	//Cargamos las unidades de los segundos
	RTC->TR &= ~RTC_TR_SU; //Limpiamos

	RTC->TR |= (unidades << RTC_TR_SU_Pos);

	/*3. Configuramos el calendario*/
	//Limpiamos el registro encargado de esto
	RTC->DR =0;

	/*3a. Cargamos el año*/
	//Nuevamente dividimos las decenas de las unidades
	DividirDecenasYunidades(ptrRTCHandler->año);

	//Cargamos la decenas del año
	RTC->DR |= (decenas << RTC_DR_YT_Pos);

	//CarDamos las unidades del año
	RTC->DR |= (unidades << RTC_DR_YU_Pos);

	/*3b. Cargamos la fecha*/
	//Nuevamente dividimos las decenas de las unidades
	DividirDecenasYunidades(ptrRTCHandler->fecha);

	//Cargamos la decenas de la fecha
	RTC->DR |= (decenas << RTC_DR_DT_Pos);

	//CarDamos las unidades de la fecha
	RTC->DR |= (unidades << RTC_DR_DU_Pos);

	/*3c. Cargamos el mes*/
	//Nuevamente dividimos las decenas de las unidades
	DividirDecenasYunidades(ptrRTCHandler->mes);

	//Cargamos la decenas del mes
	RTC->DR |= (decenas << RTC_DR_MT_Pos);

	//Cargamos las unidades del mes
	RTC->DR |= (unidades << RTC_DR_MU_Pos);


	/*3d. Cargamos el dia de la semana*/
	RTC->DR |= (ptrRTCHandler->diaSemana << RTC_DR_WDU_Pos);

	/*4. Desactivamos el modo de inicializacion*/
	RTC->ISR &= ~RTC_ISR_INIT;

	/*5. Colocamos la proteccion de escritura*/
	//Esto se puede logar basicamente cargando una clave incorrecta
	//al WPR
	//RTC->WPR = 0;
}

/*Funcion para leer las horas del RTC*/
uint8_t getRTChours(void){

	/*Primero revisamos la flag de la sincronizacion de
	 * los shadow registers
	 */
//	while(!(RTC->ISR & RTC_ISR_RSF)){
//		__NOP();
//	}

	/*Leemos las decenas de la hora*/
	uint8_t Htens = (RTC->TR & RTC_TR_HT) >> RTC_TR_HT_Pos;


	/*Leemos las unidades de la hora*/
	uint8_t Hunits = (RTC->TR & RTC_TR_HU) >> RTC_TR_HU_Pos;

	/*Ahora, operamos para que de la hora correcta*/
	uint8_t hora = (Htens*10) + Hunits;


	return hora;

}

/*Funcion para leer los minutos del RTC*/
uint8_t getRTCminutes(void){


	/*Leemos las decenas de los minutos*/
	uint8_t Mtens = (RTC->TR & RTC_TR_MNT) >> RTC_TR_MNT_Pos;

	/*Leemos las unidades de los minutos*/
	uint8_t Munits = (RTC->TR & RTC_TR_MNU) >> RTC_TR_MNU_Pos;

	/*Ahora, operamos para que de la hora correcta*/
	uint8_t minutos = (Mtens*10) + Munits;


	return minutos;

}

/*Funcion para leer los segundos del RTC*/
uint8_t getRTCseconds(void){

	/*Leemos las decenas de los segundos*/
	uint8_t Stens = (RTC->TR & RTC_TR_ST) >> RTC_TR_ST_Pos;


	/*Leemos las unidades de los minutos*/
	uint8_t Sunits = (RTC->TR & RTC_TR_SU) >> RTC_TR_SU_Pos;

	/*Ahora, operamos para que de la hora correcta*/
	uint8_t segundos = (Stens*10) + Sunits;


	return segundos;

}

/*Funcion para saber si la hora es AM o PM*/
uint8_t getRTCAmPm(void){

	/*Leemos si es PM o AM*/
	uint8_t indicador = (RTC->TR & RTC_TR_PM) >> RTC_TR_PM_Pos;

	return indicador;

}

/*Funcion para leer la fecha del RTC*/
uint8_t getRTCdate(void){

	/*Leemos las decenas de la fecha*/
	uint8_t Dtens = (RTC->DR & RTC_DR_DT) >> RTC_DR_DT_Pos;

	/*Leemos las unidades de la fecha*/
	uint8_t Dunits = (RTC->DR & RTC_DR_DU) >> RTC_DR_DU_Pos;

	/*Ahora, operamos para que de la hora correcta*/
	uint8_t fecha = (Dtens*10) + Dunits;

	return fecha;

}

/*Funcion para leer el mes del RTC*/
uint8_t getRTCmonth(void){

	/*Leemos las decenas del mes*/
	uint8_t MonthTens = (RTC->DR & RTC_DR_MT) >> RTC_DR_MT_Pos;

	/*Leemos las unidades del mes*/
	uint8_t MonthUnits = (RTC->DR & RTC_DR_MU) >> RTC_DR_MU_Pos;

	/*Ahora, operamos para que de la hora correcta*/
	uint8_t mes = (MonthTens*10) + MonthUnits;

	return mes;

}

/*Funcion para leer el año del RTC*/
uint8_t getRTCyear(void){


	/*Leemos las decenas del año*/
	uint8_t Ytens = (RTC->DR & RTC_DR_YT) >> RTC_DR_YT_Pos;

	/*Leemos las unidades de la fecha*/
	uint8_t Yunits = (RTC->DR & RTC_DR_YU) >> RTC_DR_YU_Pos;

	/*Ahora, operamos para que de la hora correcta*/
	uint8_t año = (Ytens*10) + Yunits;

	return año;

}

/*Funcion para saber el dia de la semana*/
uint8_t getRTCweekDay(void){

	/*Leemos el dia de la semana*/
	uint8_t diaSemana = (RTC->DR & RTC_DR_WDU) >> RTC_DR_WDU_Pos;

	return diaSemana;
}

void DividirDecenasYunidades(uint8_t numero){

	decenas =0;
	unidades =0;

	/*Primero almacenamos las decenas del numero*/
	decenas = numero/10;

	//Notese que pese a esa operacion ser un decimal, al almacenarla en
	//una variable entera, solo va a quedar la parte entera del numero

	/*Ahora almacenamos las unidades del numero*/
	unidades = numero - (decenas*10);

	/*Aca se tomo la parte entera, se multiplico por 10 y al restar eso,
	*solo va a quedar la unidad, por ejemplo, numero = 32
	* decenas  = 32/10 = 3.2 = 3 (por guardarse en una variable entera)
	* unidades = 32 - (3*10) = 32 - 30 = 2
	*/
}
