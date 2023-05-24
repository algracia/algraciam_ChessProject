/*
 * PLLDriver.c
 *
 *  Created on: May 22, 2023
 *      Author: algraciam
 */

#include <stm32f4xx.h>
#include "PLLDriver.h"

void configPLL(void){

	/*1. Deshabilitamos el PLL para poderlo configurar*/
	RCC->CR &= ~RCC_CR_PLLON;

	/*2. Seleccionamos la entrada de reloj al PLL*/
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;

	/*3. Cargamos el valor del PLLM siguiendo la formula:
	 * VCO input frequency = PLL input clock frequency / PLLM with 2 ≤ PLLM ≤ 63
	 * donde PLL input clock frequency = HSI = 16MHz
	 *
	 * Recomiendan que VCO input = 2MHz para lo cual, PLLM = 8
	 */
	//Limpiamos primero esa parte del registro
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;

	//Cargamos la configuracion
	RCC->PLLCFGR |= (HSI_PLLM << RCC_PLLCFGR_PLLM_Pos);


	/*4. Cargamos el valor del PLLN siguiendo la formula:
	 * VCO output frequency = VCO input frequency × PLLN with 50 ≤ PLLN ≤ 432
	 *
	 * Procurando que 100MHz < VCO output frequency < 432Mhz
	 *
	 * En este caso PLLN = 80 y VCO output frequency = 160MHz
	 */
	//Limpiamos primero esta parte del registro
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;

	//Cargamos la configuracion
	RCC->PLLCFGR |= (HSI_80MHz_PLLN << RCC_PLLCFGR_PLLN_Pos);


	/*5. Cargamos el valor del PLLP siguiendo la formula:
	 * PLL output clock frequency = VCO frequency / PLLP with PLLP = 2, 4, 6, or 8
	 *
	 * Para este caso tenemos que VCO frequency = 160MHz
	 * y necesitamos que PLL output clock frequency = 80MHz
	 *
	 * con lo cual, PLLP = 2
	 */
	//Limpiamos primero esa posicion del registro
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;

	/*6. Ahora, configuramos la latencia de la memoria FLASH para que todo funcione*/
	//Primero limpiamos la parte del registro
	FLASH->ACR &= ~(0b1111 << FLASH_ACR_LATENCY_Pos);

	//Cargamos la configuracion
	FLASH->ACR |= (0b10 << FLASH_ACR_LATENCY_Pos);

	/*7a. Configuramos la salida del MCO1 antes de habilitar el PLL*/

	//Limpiamos esta parte del registro
	RCC->CFGR &= ~RCC_CFGR_MCO1;

	//Cargamos la configuracion requerida
	RCC->CFGR |= RCC_CFGR_MCO1_0;
	RCC->CFGR |= RCC_CFGR_MCO1_1;

	/*7b. Ahora, le aplicamos un preescaler al MCO1 para poder medir la señal
	 * en un osciloscopio
	 */
	 //Limpiamos esa parte del registro
	RCC->CFGR &= ~RCC_CFGR_MCO1PRE;

	//Configuramos esa parte del registro
	RCC->CFGR |= (0b111 << RCC_CFGR_MCO1PRE_Pos);


	/*8. Activamos el PLL*/
	RCC->CR |= RCC_CR_PLLON;

	//Hacemos un bucle hasta que se avise que el PLL esta listo
	while(!(RCC->CR & RCC_CR_PLLRDY)){
		__NOP();
	}

	/*9. Cambiamos cual va a ser la fuente de reloj del sistema*/

	//Limpiamos esta parte del registro
	RCC->CFGR &= ~RCC_CFGR_SW;

	//Cargamos la configuracion
	RCC->CFGR |= RCC_CFGR_SW_1;

	/*10. Cambiamos el preescaler de los buses para que se ajusten a la nueva velocidad
	 * En este caso, solo habria que cambiar la del APB1 que puede maximo hasta 50MHz
	 *
	 * En ese sentido basta con tomar la frecuencia del AHB1 que es 1:1 con la del sistema
	 * y dividirla entre dos
	 *
	 * Esto implica: PPRE1 = 0b100
	 */

	//Por si las moscas, configuramos el preescaler del AHB1
	RCC->CFGR &= ~RCC_CFGR_HPRE;

	//Limpiamos los bits del PPRE1
	RCC->CFGR &= ~RCC_CFGR_PPRE1;

	//Cargamos la configuracion
	RCC->CFGR |= RCC_CFGR_PPRE1_2;


}

//uint8_t getConfigPLL(void){
//
//
//}
