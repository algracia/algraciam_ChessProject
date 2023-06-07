/*
 * RTCDriver.h
 *
 *  Created on: Jun 4, 2023
 *      Author: algraciam
 */

#ifndef RTCDRIVER_H_
#define RTCDRIVER_H_

#include "stm32f4xx.h"

#define RTC_FORMATO_24HORAS		0
#define	RTC_FORMATO_12HORAS		1

#define RTC_AM	0
#define RTC_PM	1

#define LUNES		1
#define MARTES		2
#define	MIERCOLES	3
#define	JUEVES		4
#define	VIERNES		5
#define	SABADO		6
#define	DOMINGO		7

#define ENERO 		1
#define FEBRERO 	2
#define MARZO	 	3
#define ABRIL	 	4
#define MAYO	 	5
#define JUNIO	 	6
#define JULIO	 	7
#define AGOSTO		8
#define SEPTIEMBRE 	9
#define OCTUBRE 	10
#define NOVIEMBRE 	11
#define DICIEMBRE 	12

typedef struct
{

uint8_t formatoHora;
uint8_t hora;
uint8_t minutos;
uint8_t segundos;
uint8_t am_pm;
uint8_t año;
uint8_t mes;
uint8_t fecha;
uint8_t diaSemana;

}RTC_Handler_t;

/*Headers de las funciones*/
void configRTC(RTC_Handler_t *ptrRTCHandler);

uint8_t getRTChours(void);
uint8_t getRTCminutes(void);
uint8_t getRTCseconds(void);
uint8_t getRTCAmPm(void);
uint8_t getRTCdate(void);
uint8_t getRTCmonth(void);
uint8_t getRTCyear(void);
uint8_t getRTCweekDay(void);

void DividirDecenasYunidades(uint8_t numero);

#endif /* RTCDRIVER_H_ */
