/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************
SOLUCIONARIO DE LA TAREA 2

1R\
a.El error consiste en que la funcion, tal y como esta:
pinValue = (pPinHandler->pGPIOx->IDR >> pPinHandler->GPIO_PinConfig.GPIO_PinNumber);

me devuelve todos los valores de todos los bits del IDR corridos o desplazados
hacia la derecha un numero de veces igual al numero del PIN que nos interesa
leer (el que hayamos configurado en el pPinHandler).

b.Mi propuesta para correir el codigo fue crear una mascara que me permitiera
seleccionar exactamente el bit con la informacion del pin que necesito.

para ello tome un uno y lo desplaze a la izquierda tantas veces como el numero
del pin y luego utilize una operacion AND bitwise y compare todos los bits del
registro con la mascara que habia creado, de modo que solo me quedaria el valor
del bit del IDR asociado al pin que necesito. Esto lo almacene en una variable
auxiliar.

Luego, movi el valor de lectura del estado del pin hacia la posicion 0 y lo
almacene en la variable "pinValue" de modo que el resultado me diera 1 o 0
(ya sea en binario o en decimal).

El codigo usado es el siguiente:

uint32_t GPIO_ReadPin(GPIO_Handler_t*pPinHandler){

	//Creamos una variable auxiliar y una variable con el valor del estado del pin
	//la cual retornaremos

	uint32_t auxPinValue =0;
	uint32_t pinValue =0;

	//Creamos una mascara, extraemos el valor del estado del pin y lo almacenamos en la variable auxiliar
	 auxPinValue= pPinHandler->pGPIOx->IDR & (SET << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);

	//Movemos el valor del estado del pin hacia la posicion cero
	 pinValue = auxPinValue >> pPinHandler->GPIO_PinConfig.GPIO_PinNumber;

	return pinValue;
}

C. se hizo la respectiva prueba en el debugger y el resultado dio correcto.


2R/
La funcion GPIOxTooglePin se ecuentra en el archivo GPIOxDriver.c
esta basicamente usa una mascara y un XOR para reescribir el ODR.

Esta funcion cambia el estado del pin gracias a que el XOR
toma el valor en el que este el PIN(sea 0 o 1 ) y lo compara
con el valor que este en la mascara ( un 1 en la posicion del
bit a modificar y un cero en las demas posiciones) y como esta
operacion logica solo da 1 cuando ambas entradas son mutuamente
excluyentes, si hay un 0 en el bit del pin claramente saldra
un 1 y si hay un 1 en el bit claramente saldra un cero.
 ******************************************************************************
 */

#include <stdint.h>

#include"stm32f411xx_hal.h"
#include"GPIOxDriver.h"

//Esta funcion la explico luego del main
uint8_t binaryControl (uint8_t decimalNumber,uint8_t bitPosition);

/*Funcion principal del programa. Es aca donde se ejecuta todo*/
int main(void) {

	//Definimos los handles para cada uno de los pines conectados a los LED
	GPIO_Handler_t handlerPC9 ={0};
	GPIO_Handler_t handlerPC6 ={0};
	GPIO_Handler_t handlerPB8 ={0};
	GPIO_Handler_t handlerPA6 ={0};
	GPIO_Handler_t handlerPC7 ={0};
	GPIO_Handler_t handlerPC8 ={0};
	GPIO_Handler_t handlerPA7 ={0};

	//Definimos el handler para el pin del boton de usuario
	GPIO_Handler_t handlerPC13 ={0};

	/*Configuramos cada uno de los pines*/

	//Configuracion pin PC9
	handlerPC9.pGPIOx 								= GPIOC;
	handlerPC9.GPIO_PinConfig.GPIO_PinNumber		= PIN_9;
	handlerPC9.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPC9.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPC9.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPC9.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PC6
	handlerPC6.pGPIOx 								= GPIOC;
	handlerPC6.GPIO_PinConfig.GPIO_PinNumber		= PIN_6;
	handlerPC6.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPC6.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPC6.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPC6.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PB8
	handlerPB8.pGPIOx 								= GPIOB;
	handlerPB8.GPIO_PinConfig.GPIO_PinNumber		= PIN_8;
	handlerPB8.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPB8.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPB8.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPB8.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PA6
	handlerPA6.pGPIOx 								= GPIOA;
	handlerPA6.GPIO_PinConfig.GPIO_PinNumber		= PIN_6;
	handlerPA6.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPA6.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPA6.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPA6.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PC7
	handlerPC7.pGPIOx 								= GPIOC;
	handlerPC7.GPIO_PinConfig.GPIO_PinNumber		= PIN_7;
	handlerPC7.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPC7.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPC7.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPC7.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PC8
	handlerPC8.pGPIOx 								= GPIOC;
	handlerPC8.GPIO_PinConfig.GPIO_PinNumber		= PIN_8;
	handlerPC8.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPC8.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPC8.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPC8.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PA7
	handlerPA7.pGPIOx 								= GPIOA;
	handlerPA7.GPIO_PinConfig.GPIO_PinNumber		= PIN_7;
	handlerPA7.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerPA7.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerPA7.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerPA7.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

	//Configuracion pin PC13
	handlerPC13.pGPIOx 								= GPIOC;
	handlerPC13.GPIO_PinConfig.GPIO_PinNumber		= PIN_13;
	handlerPC13.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerPC13.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;


	// Cargamos la configuracion a cada uno de los pines
	GPIO_Config(&handlerPC9);
	GPIO_Config(&handlerPC6);
	GPIO_Config(&handlerPB8);
	GPIO_Config(&handlerPA6);
	GPIO_Config(&handlerPC7);
	GPIO_Config(&handlerPC8);
	GPIO_Config(&handlerPA7);
	GPIO_Config(&handlerPC13);

	//definimos unas variables que sean el valor de cada bit (0 o 1)
	//con los que voy a representar los numeros decimales
	uint8_t valueBit0 = 0;
	uint8_t valueBit1 = 0;
	uint8_t valueBit2 = 0;
	uint8_t valueBit3 = 0;
	uint8_t valueBit4 = 0;
	uint8_t valueBit5 = 0;
	uint8_t valueBit6 = 0;

	uint8_t valorPinUserButton = 0;
	uint8_t i =1;
	uint32_t x =0;

	while (1) {

		/*Voy a hacer un swicht case que discrimine el caso
		 en el que el boton este presionado (case 0) o no (case 1)
		 */

		valorPinUserButton = GPIO_ReadPin(&handlerPC13); //reviso si el boton esta presionado

		switch(valorPinUserButton){
			case 1:{
				//hago un while infinito para que el conteo se mantenga
				while(1){

					//reviso si el contador llego a 60, para que empiece
					//en 1 nuevamente
					if(i>60){ i=1;}

					//defino estas variables para ser mas organizado
					//y claro con el uso de la funcion 'binaryControl'
					valueBit0 = binaryControl(i,0);
					valueBit1 = binaryControl(i,1);
					valueBit2 = binaryControl(i,2);
					valueBit3 = binaryControl(i,3);
					valueBit4 = binaryControl(i,4);
					valueBit5 = binaryControl(i,5);
					valueBit6 = binaryControl(i,6);

					GPIO_WritePin(&handlerPA7, valueBit0);
					GPIO_WritePin(&handlerPC8, valueBit1);
					GPIO_WritePin(&handlerPC7, valueBit2);
					GPIO_WritePin(&handlerPA6, valueBit3);
					GPIO_WritePin(&handlerPB8, valueBit4);
					GPIO_WritePin(&handlerPC6, valueBit5);
					GPIO_WritePin(&handlerPC9, valueBit6);

					i++;

					//reviso nuevamente si el boton esta presionado o no
					valorPinUserButton = GPIO_ReadPin(&handlerPC13);

					/*en caso de NOestar presionado, luego de la ultima iteracion,
					que se salga del while y de paso, tambien del switch case
					para que este ultimo vuelva a ejecutarse y se llegue al caso
					correspondiente */
					if(valorPinUserButton == 0){
						break;
					}

					//hago un ciclo con 1600000 iteraciones para que el tiempo que tarde
					//en realizarlas, demore aproximadamente un segundo
					for(x=1; x<=1600000; x++){
						continue;}
				}
				break;
			}

			case 0:{
				//hago un while infinito para que el conteo se mantenga
				while(1){

					/* decremento al principio, de modo que si se salio del case 1,
					 el conteo hacia atras se de desde el ultimo numero decimal
					 ingresado ya que, por ejemplo, si presiono el boton en el numero 5,
					 una vez se salga del case 1 'i' valdra 6 y me interesa contar hacia
					  atras es desde el 5.
					 */
					i--;

					/*aqui reviso si el numero decimal bajo de 1 para asi volver a iniciar
					el conteo hacia atras desde el 60*/

					if(i<1){i=60;}

					valueBit0 = binaryControl(i,0);
					valueBit1 = binaryControl(i,1);
					valueBit2 = binaryControl(i,2);
					valueBit3 = binaryControl(i,3);
					valueBit4 = binaryControl(i,4);
					valueBit5 = binaryControl(i,5);
					valueBit6 = binaryControl(i,6);

					GPIO_WritePin(&handlerPA7, valueBit0);
					GPIO_WritePin(&handlerPC8, valueBit1);
					GPIO_WritePin(&handlerPC7, valueBit2);
					GPIO_WritePin(&handlerPA6, valueBit3);
					GPIO_WritePin(&handlerPB8, valueBit4);
					GPIO_WritePin(&handlerPC6, valueBit5);
					GPIO_WritePin(&handlerPC9, valueBit6);

					//reviso nuevamente si el boton esta presionado o no
					valorPinUserButton = GPIO_ReadPin(&handlerPC13);

					/*en caso de NOestar presionado, luego de la ultima iteracion,
					 que se salga del while y de paso, tambien del switch case
				     para que este ultimo vuelva a ejecutarse y se llegue al caso
				     correspondiente */
					if(valorPinUserButton == 1){
						break;
					}

					for(x=1; x<= 1600000; x++){
						continue;}
				}
				break;
			}
			default:
				break;
			}
		}
	}


/*Voy a crear una funcion que reciba como parametros el numero
decimal que quiero mostrar y la posicion del bit que quiero controlar.

Esta funcion basicamente devuelve el valor (0 o 1) que debe tener el bit
ubicado en la posicion ingresada para asi poder representar el numero
decimal tambien ingresado.
*/

uint8_t binaryControl (uint8_t decimalNumber,uint8_t bitPosition){
	uint8_t bitValue = 0;

	//creo una mascara que me tome el valor solo del bit en la posicion que le ingresé
	bitValue = decimalNumber & (SET << bitPosition);

	//ubico el valor hacia la derecha para que me represente un 0 o un 1
	//tanto en binario como en decimal
	bitValue >>= bitPosition;
	return bitValue;
}
