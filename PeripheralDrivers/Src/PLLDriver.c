/*
 * PLLDriver.c
 *
 *  Created on: May 22, 2023
 *      Author: algraciam
 */

#include <stm32f4xx.h>
#include "PLLDriver.h"
#include "USARTxDriver.h"

void configPLL(uint8_t PLLN, uint8_t PLLP){

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
	RCC->PLLCFGR |= (PLLN << RCC_PLLCFGR_PLLN_Pos);


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

	//Realizamos un switch para cada caso del PLLP
	switch(PLLP){

	case 2:{
		//Para este caso, PLLP = 0b00
		RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
		break;
	}
	case 4:{
		//Para este caso, PLLP = 0b01
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLP_0;
		break;
	}
	case 6:{
		//Para este caso, PLLP = 0b10
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLP_1;
		break;
	}
	case 8:{
		//Para este caso, PLLP = 0b11
		RCC->PLLCFGR |= RCC_PLLCFGR_PLLP;
		break;
	}
	default:{
		__NOP();
		break;
	}
	}

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
	RCC->CFGR |= (0b110 << RCC_CFGR_MCO1PRE_Pos);


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

	//Por si las moscas, configuramos el preescaler del AHB1 y del APB2
	//De modo que su frecuencia quede 1:1

	//Para el AHB1
	RCC->CFGR &= ~RCC_CFGR_HPRE;

	//Para el APB2
	RCC->CFGR &= ~RCC_CFGR_PPRE2;

	//Limpiamos los bits del PPRE1
	RCC->CFGR &= ~RCC_CFGR_PPRE1;

	//Cargamos la configuracion
	RCC->CFGR |= RCC_CFGR_PPRE1_2;


}//Fin funcion PLLConfig

uint8_t getPLLFrequency(uint8_t PLLN, uint8_t PLLP){
	/*Creamos una funcion que calcule la frecuencia a la que esta el PLL
	 * segun los valores de PLLN y PLLP que este.
	 *
	 * Solo sera con estos valores ya que como siempre se va a usar el HSI
	 * PLLM tendra un valor fijo y por ende la VCOinput siempre sera de 2MHz
	 *
	 * En general, la formula completa cambiar la frecuencia del micro es:
	 * PLL output clock frequency = (HSIFrequency * PLLN)/(PLLM/PLLP)
	 *
	 * pero como HSIFrequency/PLLM = VCO input frequency = 2MHz
	 *
	 * -> PLL output clock frequency = 2*(PLLN/PLLP)
	 */
	uint8_t PLLOutFrequency = 0;

	PLLOutFrequency = 2*(PLLN/PLLP);

	return PLLOutFrequency;


}//Fin funcion getPLLFrequency


/*Creamos una funcion que va a calcular y cambiar los valores del BRR del USART
  segun la frecuencia que  se le ingrese
 */
void ChangeUSART_BRR(USART_Handler_t *ptrUsartHandler,uint8_t PLLFreqMHz){
	/*Primero debemos activar la unidad de punto flotante para esta operacion*/
	SCB->CPACR |= (0XF << 20);

	//Vamo a aplicar la  ecuacion para hallar el valor a cargar en el BRR
	//Para cada Baudrate configurado (Con OVER8 =0)
	uint32_t auxMantiza =0;
	float auxFraccion =0;

	uint16_t mantiza =0;
	uint8_t fraccion =0;

	switch(ptrUsartHandler ->USART_Config.USART_baudrate){

	case USART_BAUDRATE_9600:{

		/*Calculamos la mantiza*/
		auxMantiza =(PLLFreqMHz * 1000000)/(16*9600);

		/*Calculamos la fraccion*/
		auxFraccion = ((((float)PLLFreqMHz * 1000000)/(16*9600)) - (float)auxMantiza)*16;

		/*definimos la mantiza y la fraccion*/
		mantiza = (uint16_t) auxMantiza;
		fraccion = (uint8_t) auxFraccion;


		/*Cargamos la mantiza*/
		//Limpiamos esa parte del registro
		ptrUsartHandler->ptrUSARTx->BRR &= ~(USART_BRR_DIV_Mantissa);

		//Escribimos en el registro
		ptrUsartHandler->ptrUSARTx->BRR |= (mantiza << USART_BRR_DIV_Mantissa_Pos);

		/*Cargamos la fraccion*/
		//Limpiamos esa parte del registro
		ptrUsartHandler->ptrUSARTx->BRR &= ~(USART_BRR_DIV_Fraction);

		//Escribimos en el registro
		ptrUsartHandler->ptrUSARTx->BRR |= (fraccion << USART_BRR_DIV_Fraction_Pos);

		break;

	}
	case USART_BAUDRATE_19200:{

		/*Calculamos la mantiza*/
		auxMantiza =(PLLFreqMHz * 1000000)/(16*19200);

		/*Calculamos la fraccion*/
		auxFraccion = ((((float)PLLFreqMHz * 1000000)/(16*19200)) - (float)auxMantiza)*16;

		/*definimos la mantiza y la fraccion*/
		mantiza = (uint16_t) auxMantiza;
		fraccion = (uint8_t) auxFraccion;


		/*Cargamos la mantiza*/
		//Limpiamos esa parte del registro
		ptrUsartHandler->ptrUSARTx->BRR &= ~(USART_BRR_DIV_Mantissa);

		//Escribimos en el registro
		ptrUsartHandler->ptrUSARTx->BRR |= (mantiza << USART_BRR_DIV_Mantissa_Pos);

		/*Cargamos la fraccion*/
		//Limpiamos esa parte del registro
		ptrUsartHandler->ptrUSARTx->BRR &= ~(USART_BRR_DIV_Fraction);

		//Escribimos en el registro
		ptrUsartHandler->ptrUSARTx->BRR |= (fraccion << USART_BRR_DIV_Fraction_Pos);

		break;

	}

	case USART_BAUDRATE_115200:{

		/*Calculamos la mantiza*/
		auxMantiza =(PLLFreqMHz * 1000000)/(16*115200);

		/*Calculamos la fraccion*/
		auxFraccion = ((((float)PLLFreqMHz * 1000000)/(16*115200)) - (float)auxMantiza)*16;

		/*definimos la mantiza y la fraccion*/
		mantiza = (uint16_t) auxMantiza;
		fraccion = (uint8_t) auxFraccion;


		/*Cargamos la mantiza*/
		//Limpiamos esa parte del registro
		ptrUsartHandler->ptrUSARTx->BRR &= ~(USART_BRR_DIV_Mantissa);

		//Escribimos en el registro
		ptrUsartHandler->ptrUSARTx->BRR |= (mantiza << USART_BRR_DIV_Mantissa_Pos);

		/*Cargamos la fraccion*/
		//Limpiamos esa parte del registro
		ptrUsartHandler->ptrUSARTx->BRR &= ~(USART_BRR_DIV_Fraction);

		//Escribimos en el registro
		ptrUsartHandler->ptrUSARTx->BRR |= (fraccion << USART_BRR_DIV_Fraction_Pos);


		break;
	}

	default:{
		__NOP();
		break;
	}
	}
}//FIn funcion ChangeUSART
