/*
 * SOLUCIÓN TAREA #2
 * Created on: Mar 13, 2023
 * Author: Isabel Puerta Alvarez
 */


/* PRIMER PUNTO:
 *
 * El error corresponde a que, en la función GPIO_ReadPin() sólo se está trasladando hacia la posicion menos
 * significativa del registro IDR el bit particular que se quiere leer.
 *
 * Sólo se está moviendo, pero lo que haya a la izquiera de ese bit, sigue estando, y al retornar PinValue no
 * se tendrá únicamente el dato de 1 bit, sino otros que no me interesan (Cuando quiero leer un pin,
 * sólo necesito ese pin, no el resto)
 *
 * Este error se soluciona con una máscara con una operación AND, poniendo un 1 en la posición menos
 * significativa y el resto con ceros. Con esto estamos limpiando la información que no necesito y dejando sólo
 * lo que sí necesito.
 *
 * Esto se hace añadiendo la siguiente línea de código en el GPIOxDRIVER.c
 *
 * pinValue &= (1<<0);
 *
 * Esta corrección se comprobó configurando el user button para leer, mediante la función GPIO_ReadPin() su estado,
 * el cual corresponde a un 1 si está sin presionar y a un 0 estando presionado.
 *
 * Esta misma configuración del botón también es necesaria para el punto 3.
 *
 *
 */

/* SEGUNDO PUNTO:
 *
 * Esta función es creada en el GPIOxDriver.c, es aquí donde se definen funciones de manera global para poder,
 * desde el archivo main, llamar a esas funciones.
 *
 * Adicionalmente, se debe añadir la cabecera de la función al archivo GPIOxDriver.h, que es aquí donde se
 * definen diferentes estructuras útiles para toda la configuración de los GPIOx en general.
 *
 * La función GPIOxTooglePin, de manera intuitiva se puede ver como un blinking en un LED, por lo tanto, es una
 * forma de escribir sobre el registro ODR después de configurar el pin respectivo como salida.
 *
 * La máscara utilizada corresponde a una operación XOR que sólo tiene 1 en el bit correspondiente al pin que
 * se está configurando, y el resto está lleno de ceros. Esto para garantizar que sólo se modifique una única
 * posición de registro sin afectar al resto.
 *
 * Esta función es utilizada como un extra del punto 3, haciendo que el LED LD2 se encienda y apague a un tiempo
 * determinado
 */


//Se incluyen las librerías y otros archivos necesarios para la compilación del código.

#include <stdint.h>
#include "stm32f411xx_hal.h"
#include "GPIOxDriver.h"



//Definición de variables para el punto 3

uint8_t numeroBinario = 0;
uint32_t buttonValue;

/*
 * -Se definen los handlers para los pines que se van a configurar. Estos se ubican afuera de la función main,
 * 	porque la función controlLeds está definida por fuera del main y necesita utilizar esos handlers, por lo que
 * 	deben ser globales.
 *
 * -Se definen cabeceras de funciones.
 */

void controlLeds(uint8_t numeroBinario);		//Cabecera de la función controlLeds.

GPIO_Handler_t handlerOnBoardLed = {0};			//Handler para el led LD2
GPIO_Handler_t handlerUserButton = {0};			//Handler para el user button
GPIO_Handler_t bit6 = {0};						//Handler para el pin PC9 que corresponde el Bit-6
GPIO_Handler_t bit5 = {0};						//Handler para el pin PC6 que corresponde el Bit-5
GPIO_Handler_t bit4 = {0};						//Handler para el pin PB8 que corresponde el Bit-4
GPIO_Handler_t bit3 = {0};						//Handler para el pin PA6 que corresponde el Bit-3
GPIO_Handler_t bit2 = {0};						//Handler para el pin PC7 que corresponde el Bit-2
GPIO_Handler_t bit1 = {0};						//Handler para el pin PC8 que corresponde el Bit-1
GPIO_Handler_t bit0 = {0};						//Handler para el pin PA7 que corresponde el Bit-0


int main(void){

	/*
	 * Paso seguido, se utilizan los handlers respectivos para configurar los valores respecitvos con los que
	 * se van a modificar los registros que controlan el comportamiento del pin.
	 *
	 * Mediante las macros definidas en el archivo SMT32F411xx_hal.h se puede asociar palabras (algo fácil de
	 * recordar) a los valores (1/0) que se deben escribir en cada registro para poder que cumpla la función
	 * que deseo. Por ejemplo GPIO_MODE_OUT corresponde a un 1, lo cual corresponde, en el Mode Register
	 * a configurar el pin como una salida.
	 *
	 * Luego, la función GPIO_Config toma todos los datos configurados en el handler, para traducirlos en 1 y 0
	 * en los registros correspondientes, para así finalmente, configurar el pin.
	 */

	//Configuración del pin asociado al led LD2: PA5

	handlerOnBoardLed.pGPIOx = GPIOA;												//Configura el puerto del GPIO
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;				//Configura el número del pin
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;		//Configura si es IN/OUT
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;	//Configura el tipo de salida
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;		//Configura la velocidad
	handlerOnBoardLed.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;					//Ninguna función

	GPIO_Config(&handlerOnBoardLed);			//Se carga la configuración del LD2.

	//COnfiguración del pin asociado al user button: PC13

	handlerUserButton.pGPIOx = GPIOC;
	handlerUserButton.GPIO_PinConfig.GPIO_PinNumber			= PIN_13;
	handlerUserButton.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserButton.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	handlerUserButton.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&handlerUserButton);			//Se carga la configuración del botón

	//Configuración del pin PC9 que corresponde el Bit-6

	bit6.pGPIOx = GPIOC;
	bit6.GPIO_PinConfig.GPIO_PinNumber			= PIN_9;
	bit6.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit6.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit6.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit6.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit6.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit6);							//Se carga la configuración del PC9 (bit 6)

	//Configuración del pin PC6 que corresponde el Bit-5

	bit5.pGPIOx = GPIOC;
	bit5.GPIO_PinConfig.GPIO_PinNumber			= PIN_6;
	bit5.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit5.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit5.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit5.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit5.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit5);							//Se carga la configuración del PC6 (bit 5)


	//Configuración del pin PB8 que corresponde el Bit-4

	bit4.pGPIOx = GPIOB;
	bit4.GPIO_PinConfig.GPIO_PinNumber			= PIN_8;
	bit4.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit4.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit4.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit4.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit4.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit4);							//Se carga la configuración del PB8 (bit 4)

	//Configuración del pin PA6 que corresponde el Bit-3

	bit3.pGPIOx = GPIOA;
	bit3.GPIO_PinConfig.GPIO_PinNumber			= PIN_6;
	bit3.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit3.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit3.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit3.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit3.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit3);							//Se carga la configuración del PA6 (bit 3)

	//Configuración del pin PC7 que corresponde el Bit-2

	bit2.pGPIOx = GPIOC;
	bit2.GPIO_PinConfig.GPIO_PinNumber			= PIN_7;
	bit2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit2.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit2.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit2.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit2.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit2);							//Se carga la configuración del PC7 (bit 2)

	//Configuración del pin PC8 que corresponde el Bit-1

	bit1.pGPIOx = GPIOC;
	bit1.GPIO_PinConfig.GPIO_PinNumber			= PIN_8;
	bit1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit1.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit1.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit1.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit1.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit1);							//Se carga la configuración del PC8 (bit 1)

	//Configuración del pin PA7 que corresponde el Bit-0

	bit0.pGPIOx = GPIOA;
	bit0.GPIO_PinConfig.GPIO_PinNumber			= PIN_7;
	bit0.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	bit0.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	bit0.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
	bit0.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	bit0.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;

	GPIO_Config(&bit0);							//Se carga la configuración del PA7 (bit 0)


	while(1){									//ciclo principal, donde se ejecuta el programa

		/*
		 * Mediante el siguiente for se garantiza un delay de aproximadamente 1 segundo, para poder apreciar
		 * el parpadeo del led
		 * Llamando a la función Toogle, se modifica el registro ODR correspondiente para modificar la salida
		 * del pin que controla al LED.
		 */

		for(uint32_t contador = 0; contador <= 1000000; contador ++){

		NOP();
		}
		GPIOxTooglePin(&handlerOnBoardLed);


		// La variable buttonValue, guarda el estado del user button, para evaluar si está presionado o no.

		buttonValue = GPIO_ReadPin(&handlerUserButton);

		/*
		 * Mediante un condicional if-else, se indica de qué manera va a contar el programa: va a incrementar
		 * de a 1 si el botón NO está siendo presionado (su estado es SET) y de lo contrario (está presionado)
		 * va a disminuir de a 1
		 */

		if (buttonValue == SET){

			numeroBinario ++;						//Se realiza el incremento

			//La función controlLeds enciende y apaga los LEDS dependiendo del valor de numeroBinario.

			controlLeds(numeroBinario);

			//Con el siguiente if se garantiza que el número sólo incremente hasta 60 y que luego vuelva a ser 1.

			if (numeroBinario == 61){
				numeroBinario = 1;

				controlLeds(numeroBinario);			//Encendido y apagado de lEDS

			} //Fin if límite superior

			//Con el delay se logra que el cambio del número se dé aproximadamente cada segundo.

//			for(uint32_t contador = 0; contador <= 1000000; contador ++){
//
//			NOP();
//			} //Fin delay

		} //Fin if: button SET

		else{
			numeroBinario --;						//Se realiza la disminución

			controlLeds(numeroBinario);				//Encendido y apagado de LEDS

			if (numeroBinario == 0){				//Cuando el número llegue a 1, pasa de nuevo a 60.
				numeroBinario = 60;

				controlLeds(numeroBinario);			//Encendido y apagado de LEDS.

			}  //Fin else límite inferior

//			for(uint32_t contador = 0; contador <= 1000000; contador ++){		//delay
//
//			NOP();
//			}	//Fin delay

		} //Fin else: button CLEAR
	} //Fin while

} //Fin main


/*
 * DEFINICIÓN DE FUNCIONES DEL PROGRAMA:
 *
 * El numeroBinario corresponde a la variable que va aumentando o disminuyendo de a 1.
 * Necesito que el número binario que esa variable representa, se visualice en los leds.
 * Por lo que, cada bit del número (0,1,2,3,4,5,6) corresponde a un pin (y por ende a un LED)
 * determinado.
 *
 * La función controlLeds es la encargada de encender y apagar los leds dependiendo del estado
 * (1/0) de cada bit de numeroBinario.
 *
 * Cada if corresponde a un bit. Por lo que, la condición, corresponde a una máscara para volver
 * todo cero excepto el bit que voy a evaluar.
 *
 * Si el estado del bit es cero, se llama a la función GPIO_WritePin para que se configure la posición
 * de registro BSRR de modo que se haga un RESET y por ende el LED se apague. (1 en la parte alta
 * del registro y 0 en la parte baja)
 *
 * Si el estado del pin es 1, se llama a la función GPIO_WritePin para que configure el registro BSRR
 * de modo que se haga un SET y se encienda el LED respectivo. (0 en la parte alta del registro y 1 en
 * la baja)
 *
 */

void controlLeds(uint8_t numeroBinario){

	if ((numeroBinario &(1<<0)) == 0){
		GPIO_WritePin(&bit0, CLEAR);
	}
	else {
		GPIO_WritePin(&bit0, SET);
	}


	if ((numeroBinario &(1<<1)) == 0){
		GPIO_WritePin(&bit1, CLEAR);
	}
	else {
		GPIO_WritePin(&bit1, SET);
	}


	if ((numeroBinario &(1<<2)) == 0){
		GPIO_WritePin(&bit2, CLEAR);
	}
	else {
		GPIO_WritePin(&bit2, SET);
	}


	if ((numeroBinario &(1<<3)) == 0){
		GPIO_WritePin(&bit3, CLEAR);
	}
	else {
		GPIO_WritePin(&bit3, SET);
	}


	if ((numeroBinario &(1<<4)) == 0){
		GPIO_WritePin(&bit4, CLEAR);
	}
	else {
		GPIO_WritePin(&bit4, SET);
	}


	if ((numeroBinario &(1<<5)) == 0){
		GPIO_WritePin(&bit5, CLEAR);
	}
	else {
		GPIO_WritePin(&bit5, SET);
	}


	if ((numeroBinario &(1<<6)) == 0){
		GPIO_WritePin(&bit6, CLEAR);
	}
	else {
		GPIO_WritePin(&bit6, SET);
	}

}	//Fin controlLeds

