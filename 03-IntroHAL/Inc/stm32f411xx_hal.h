/*************************************************************************************************************************
 * stm32f411xx_hal.h
 *
 *  Created on: Mar 3, 2023
 *      Author: algraciam
 *  Este archivo contiene la informacion mas basica del micro:
 *  	-Valores de reloj principal
 *  	-Distribucion basica de la memoria (descrito en la figura 14 de la hoja de datos del micro)
 *  	-Posiciones de memoria de los perifericos disponibles en el micro descrito en la tabal(Memory Map)
 *  	-Incluye los demas registros de los perifericos
 *  	-definiciones de las constantes mas basicas
 * ***********************************************************************************************************************
 */

#ifndef STM32F411XX_HAL_H_
#define STM32F411XX_HAL_H_

#include <stdint.h>
#include <stddef.h>

#define HSI_CLOCK_SPEED 16000000  //Value for the main clock signal (HSI -> High Speed Internal)
#define HSE_CLOCK_SPEED 4000000   //Value for the main clock signal (HSE -> High Speed External)


#define NOP()		asm("NOP")
#define __weak		__attribute__((weak))

/*
 * Base addresses of Flash and SRAM memories
 * Datasheet, Memory Map, Figure 14
 */
#define FLASH_BASE_ADDR 	0x08000000U	//Memoria del programa, 512KB
#define SRAM_BASE_ADDR 		0x20000000U	//Memoria RAM, 128KB

/**
 * AHBx and APBx Bus Peripherals base addresses
 */
#define APB1_BASE_ADDR		0x40000000U
#define APB2_BASE_ADDR		0x40010000U
#define AHB1_BASE_ADDR		0x40020000U
#define AHB2_BASE_ADDR		0x50000000U

/* Posiciones de memoria para perifericos del AHB2*/
#define USB_OTG_FS_BASE_ADDR	(AHB2_BASE_ADDR + 0x0000U)

/*Posiciones de memoria para perifericos del AHB1
 * Observar que NO esta completa*/
#define RCC_BASE_ADDR 		(AHB1_BASE_ADDR + 0x3800U)
#define GPIOH_BASE_ADDR		(AHB1_BASE_ADDR + 0x1C00U)
#define GPIOE_BASE_ADDR		(AHB1_BASE_ADDR + 0x1000U)
#define GPIOD_BASE_ADDR		(AHB1_BASE_ADDR + 0x0C00U)
#define GPIOC_BASE_ADDR		(AHB1_BASE_ADDR + 0x0800U)
#define GPIOB_BASE_ADDR		(AHB1_BASE_ADDR + 0x0400U)
#define GPIOA_BASE_ADDR		(AHB1_BASE_ADDR + 0x0000U)

/*
 * Macros Genericos
 */
#define ENABLE 				1
#define DISABLE 			0
#define SET 				ENABLE
#define CLEAR				DISABLE
#define RESET				DISABLE
#define GPIO_PIN_SET		SET
#define GPIO_PIN_RESET		RESET
#define FLAG_SET			SET
#define FLAG_RESET			RESET
#define I2C_WRITE			0
#define I2C_READ			1

/*Definicion de la estructura de datos que representa a cada uno de los registros que componen el periferico RCC*/
typedef struct
{
	volatile uint32_t CR;			//Clock Control Register					ADDR_OFFSET:	0x00
	volatile uint32_t PLLCFGR;		//PLL Configuration register				ADDR_OFFSET:	0x04
	volatile uint32_t CFGR;			//Clock Configuration register				ADDR_OFFSET:	0x08
	volatile uint32_t CIR;			//Clock Interrupt Register					ADDR_OFFSET:	0x0C
	volatile uint32_t AHB1RSTR;		//AHB1 Peripheral reset register			ADDR_OFFSET:	0x10
	volatile uint32_t AHB2RSTR;		//AHB2 Peripheral reset register			ADDR_OFFSET:	0x14
	volatile uint32_t reserved0;	//reserved									ADDR_OFFSET:	0x18
	volatile uint32_t reserved1;	//reserved									ADDR_OFFSET:	0x1C
	volatile uint32_t APB1RSTR;		//APB1 Peripheral reset register			ADDR_OFFSET:	0x20
	volatile uint32_t APB2RSTR;		//APB2 Peripheral reset register			ADDR_OFFSET:	0x24
	volatile uint32_t reserved2;	//reserved									ADDR_OFFSET:	0x28
	volatile uint32_t reserved3;	//reserved									ADDR_OFFSET:	0x2C
	volatile uint32_t AHB1ENR;		//AHB1 Peripheral clock enable register		ADDR_OFFSET:	0x30
	volatile uint32_t AHB2ENR;		//AHB2 Peripheral clock enable register		ADDR_OFFSET:	0x34
	volatile uint32_t reserved4;	//reserved									ADDR_OFFSET:	0x38
	volatile uint32_t reserved5;	//reserved									ADDR_OFFSET:	0x3C
};

#endif /* STM32F411XX_HAL_H_ */
