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

#define LUNES
#define MARTES
#define	MIERCOLES

typedef struct
{

uint8_t formatoHora;
uint8_t hora;
uint8_t minutos;
uint8_t segundos;
uint8_t am_pm;
uint8_t año;
uint8_t fecha;
uint8_t diaSemana;

}RTC_Handler_t;

#endif /* RTCDRIVER_H_ */
