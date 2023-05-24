/*
 * PLLDriver.h
 *
 *  Created on: May 22, 2023
 *      Author: algraciam
 */

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

#include <stm32f4xx.h>

#define HSI_PLLM		8
#define HSI_80MHz_PLLN	80
#define HSI_80MHz_PLLQ	4

void configPLL(void);
uint8_t getConfigPLL(void);

#endif /* PLLDRIVER_H_ */
