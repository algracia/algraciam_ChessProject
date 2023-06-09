/*
 * PLLDriver.h
 *
 *  Created on: May 22, 2023
 *      Author: algraciam
 */

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

#include <stm32f4xx.h>
#include "USARTxDriver.h"
#include "I2CDriver.h"

#define HSI_PLLM		8
#define HSITRIM			12

#define HSI_80MHz_PLLN	80
#define HSI_80MHz_PLLP	2

#define HSI_100MHz_PLLN	100
#define HSI_100MHz_PLLP	2


void configPLL(uint8_t PLLN, uint8_t PLLP);
uint8_t getPLLFrequency(uint8_t PLLN, uint8_t PLLP);
void ChangeUSART_BRR(USART_Handler_t *ptrUsartHandler,uint8_t BusFreqMHz);
void ChangeClockI2C(I2C_Handler_t *ptrHandlerI2C,uint8_t BusFreqMHz);

#endif /* PLLDRIVER_H_ */
