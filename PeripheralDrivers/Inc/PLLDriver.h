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

#define HSI_PLLM		8
#define HSI_80MHz_PLLN	80
#define HSI_80MHz_PLLP	2

void configPLL(uint8_t PLLN, uint8_t PLLP);
uint8_t getPLLFrequency(uint8_t PLLN, uint8_t PLLP);
void ChangeUSART_BRR(USART_Handler_t *ptrUsartHandler,uint8_t PLLFreqMHz);


#endif /* PLLDRIVER_H_ */
