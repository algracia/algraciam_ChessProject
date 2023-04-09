/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************
*Desarrollo de la Tarea 3
 ******************************************************************************
 */

#include <stdint.h>
#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"

/*Macros utiles*/
#define DISPUNI		0
#define DISPDEC		1
#define ALL_OFF		12

/*Handlers de los GPIO*/
GPIO_Handler_t handlerOnBoardLed 			={0};
GPIO_Handler_t handlerPuertoA0				={0};
GPIO_Handler_t handlerPuertoA1				={0};
GPIO_Handler_t handlerPuertoA4				={0};
GPIO_Handler_t handlerPuertoC11				={0};
GPIO_Handler_t handlerPuertoC10				={0};
GPIO_Handler_t handlerPuertoC12				={0};
GPIO_Handler_t handlerPuertoA15				={0};
GPIO_Handler_t handlerPuertoC0				={0};
GPIO_Handler_t handlerPuertoC1				={0};
GPIO_Handler_t handlerPuertoC2				={0};
GPIO_Handler_t handlerPuertoC3				={0};
GPIO_Handler_t handlerPuertoB0				={0};

/*Handlers de los timers*/
BasicTimer_Handler_t handlerTimerBlinky 	={0};
BasicTimer_Handler_t handlerTimer7Seg		={0};

/*Handlers de los EXTI*/
EXTI_Config_t handlerEXTIEncoder			={0};
EXTI_Config_t handlerEXTISW					={0};

/*Flags*/
//Estas banderas iran dentro de los callback que correpondan
//y permitiran ejecutar en la funcion main el codigo relacionado
//a su interrupcion correspondiente

uint8_t Exti4Flag	=0;
uint8_t Timer3Flag 	=0;
uint8_t ModeFlag	=0;  //Indica si se esta en modo "Conteo" o "Culebrita"


/*Variables*/
uint8_t data 				=0;
uint8_t clk					=0;
uint8_t displaySelect		=0;
int8_t unitCounter  		=0;
int8_t decimalCounter  		=0;
int8_t culebritaCounter 	=0;


/*Funciones*/
void InitHardware (void);
void Numero7Segmentos (uint8_t numero);
void CulebritaConfig (uint8_t pos);
uint8_t DisplaySelect (int8_t pos_disp);



int main(void) {

	/*Cargamos toda la configuracion del hardware*/
	InitHardware ();


	/* Loop forever*/
	while (1) {

		//Se genera un switch-case que discrimine ambos modos
		switch (ModeFlag){

		case 0:{
			/*Este caso corresponde al modo conteo*/

			if(Timer3Flag){
				//Se intercambian los valores de los pines
				//que controlan a los transistores, esto
				//genera el switcheo entre los dos displays
				GPIOxTooglePin(&handlerPuertoC11);
				GPIOxTooglePin(&handlerPuertoC10);

				/*Configuramos como se van a mostrar los numeros en el 7 segmentos*/

				if(GPIO_ReadPin(&handlerPuertoC11)){
					//Si el puerto C11(el de las unidades) esta con un 1 logico
					//entonces el display de las unidades esta apagdo y el
					//de las decimas esta encendido, con lo cual, se configura
					Numero7Segmentos(decimalCounter);

				}else{
					//Por otro lado, si el puerto C11(el de las unidades)
					//esta con un 0 logico entonces el display de las
					//unidades encendido, con lo cual, este se configura
					Numero7Segmentos(unitCounter);

				}
			Timer3Flag =0; //Se baja la bandera del Timer 3

			}//Fin del 'if' del Timer3


			/*Configurando el contador de unidades
			 * y decimas segun el sentido de giro del encoder
			 */
			if(Exti4Flag){

				if (data > clk){
					//Sentido CW
					unitCounter++;

					if(unitCounter > 9){
						unitCounter =0;
						decimalCounter++;
						//Cada que las unidades pasen de 9,
						//la decimas aumentan en 1 y las unidades vuelven a 0

						if (decimalCounter > 9){
							unitCounter 	=9;
							decimalCounter 	=9;

							//Ademas, si tanto unidadades como decimas pasan de 9,
							//su valor se mantendra estatico e igual a 9,
							//de este modo, los displays no muestran un numero mayor a 99
						}
					}


				}else{
					//Sentido CCW
					unitCounter--;

					if(unitCounter < 0){
						unitCounter =9;
						decimalCounter--;
						//Cada que las unidades bajen de 0,
						//la decimas disminuyen en 1 y las unidades vuelven a 9

						if (decimalCounter < 0){
							unitCounter 	=0;
							decimalCounter  =0;
							//Ademas, si tanto unidadades como decimas bajan de 0,
							//su valor se mantendra estatico e igual a 0,
							//de este modo, los displays no muestran un numero menor a 00
						}
					}
				}
				Exti4Flag =0;// Se baja el Flag del EXTI4

			}//Fin del 'if' del Exti del encoder

			break;
		}//Fin del case 0


		case 1:{
			/*Este caso corresponde al modo "Culebrita"*/

			if(Timer3Flag){
				GPIOxTooglePin(&handlerPuertoC10);
				GPIOxTooglePin(&handlerPuertoC11);

				//Revisamos en que display se deberia estar visualizando el segmento
				displaySelect = DisplaySelect(culebritaCounter);

				if(displaySelect ==DISPUNI){

					//Si se debe visualizar en las unidades, entonces apagamos el display de las decenas
					//y dejamos encendido el de las unidades con su respectiva configuracion.

					if(GPIO_ReadPin(&handlerPuertoC11)){
						//Si el puerto C11(el de las unidades) esta con un 1 logico
						//entonces el display de las unidades esta apagdo y el
						//de las decimas esta encendido, con lo cual, se configura
						CulebritaConfig(ALL_OFF);

					}else{
						//Por otro lado, si el puerto C11(el de las unidades)
						//esta con un 0 logico entonces el display de las
						//unidades encendido, con lo cual, este se configura
						CulebritaConfig(culebritaCounter);
					}

				}else{

					//Si se debe visualizar en las decenas, entonces apagamos el display de las unidades
					//y dejamos encendido el de las decenas con su respectiva configuracion.

					if(GPIO_ReadPin(&handlerPuertoC11)){
						//Si el puerto C11(el de las unidades) esta con un 1 logico
						//entonces el display de las unidades esta apagdo y el
						//de las decimas esta encendido, con lo cual, se configura
						CulebritaConfig(culebritaCounter);

					}else{
						//Por otro lado, si el puerto C11(el de las unidades)
						//esta con un 0 logico entonces el display de las
						//unidades encendido, con lo cual, este se configura
						CulebritaConfig(ALL_OFF);
					}
				}

				Timer3Flag =0; // Se baja la bandela del TIMER3
			}

			if(Exti4Flag){

				if (data > clk){

					//Sentido CW
					culebritaCounter++;

						if (culebritaCounter > 11){
							//Cada que se supera la posicion final,
							//la culebrita se reinicia y vuelve a su posicion incial
							culebritaCounter 	=0;
						}
					}

				else{

					//Sentido CCW
					culebritaCounter--;

						if (culebritaCounter < 0){
							culebritaCounter =11;
							//Cada que se llega mas abajo que la posicion inicial,
							//la culebrita vuelve a empezar desde su posicion final
						}
					}
				Exti4Flag =0; //Se baja la bandera del EXTI4
				}

				break;
			}

		default:{
			break;
		}

		}
	}
}


void InitHardware (void){

		/*Configuracion del Blinky*/
		handlerOnBoardLed.pGPIOx 								= GPIOA;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
		handlerOnBoardLed.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		handlerTimerBlinky.ptrTIMx								=TIM2;
		handlerTimerBlinky.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
		handlerTimerBlinky.TIMx_Config.TIMx_speed				=BTIMER_SPEED_1ms;
		handlerTimerBlinky.TIMx_Config.TIMx_period				=250;
		handlerTimerBlinky.TIMx_Config.TIMx_interruptEnable		=BTIMER_INTERRUPT_ENABLE;

		//Cargamos las configuraciones
		GPIO_Config(&handlerOnBoardLed);
		BasicTimer_Config(&handlerTimerBlinky);


		/*Configuracion del encoder*/
		handlerPuertoA1.pGPIOx								= GPIOA;
		handlerPuertoA1.GPIO_PinConfig.GPIO_PinNumber		= PIN_1;
		handlerPuertoA1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
		handlerPuertoA1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

		handlerPuertoA4.pGPIOx								= GPIOA;
		handlerPuertoA4.GPIO_PinConfig.GPIO_PinNumber		= PIN_4;
		handlerPuertoA4.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
		handlerPuertoA4.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

		//Cargamos la configuracion del pin de DATA del encoder
		GPIO_Config(&handlerPuertoA1);

		//Configuracion del EXTI del Encoder para el Pin del Clock
		handlerEXTIEncoder.pGPIOHandler						=&handlerPuertoA4;
		handlerEXTIEncoder.edgeType							=EXTERNAL_INTERRUPT_FALLING_EDGE;

		//Cargamos la configuracion del EXTI para el Pin del Clock
		ExtInt_Config(&handlerEXTIEncoder);



		/*Configuracion del boton del encoder*/
		handlerPuertoA0.pGPIOx								= GPIOA;
		handlerPuertoA0.GPIO_PinConfig.GPIO_PinNumber		= PIN_0;
		handlerPuertoA0.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
		handlerPuertoA0.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;


		//Configuracion del EXTI del boton del encoder
		handlerEXTISW.pGPIOHandler						=&handlerPuertoA0;
		handlerEXTISW.edgeType							=EXTERNAL_INTERRUPT_FALLING_EDGE;

		//Cargamos la configuracion del EXTI
		ExtInt_Config(&handlerEXTISW);



		/*Configuracion del 7 segmentos*/

		//Configuracion del swicheo de los 7 segmentos
		handlerPuertoC10.pGPIOx 								= GPIOC;
		handlerPuertoC10.GPIO_PinConfig.GPIO_PinNumber			= PIN_10;
		handlerPuertoC10.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerPuertoC10.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC10.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC10.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC10);
		GPIO_WritePin(&handlerPuertoC10, 0); //Hacemos que este pin sea un cero logico a la salida (Switchea decenas)

		handlerPuertoC11.pGPIOx 								= GPIOC;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinNumber			= PIN_11;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC11);
		GPIO_WritePin(&handlerPuertoC11, 1); //Hacemos que este pin sea un 1 logico a la salida (Switchea unidades)

		//De este modo garantizamos que las salidas de ambos pines seran opuestas
		//y asi, cuando uno este encendido, el otro estará apagado

		//Configuracion del timer asociado
		handlerTimer7Seg.ptrTIMx							=TIM3;
		handlerTimer7Seg.TIMx_Config.TIMx_mode				=BTIMER_MODE_UP;
		handlerTimer7Seg.TIMx_Config.TIMx_speed				=BTIMER_SPEED_1ms;
		handlerTimer7Seg.TIMx_Config.TIMx_period			=5;
		handlerTimer7Seg.TIMx_Config.TIMx_interruptEnable	=BTIMER_INTERRUPT_ENABLE;

		BasicTimer_Config(&handlerTimer7Seg);

		//Configuracion del conteo del 7 segmentos

		/*
		 * A -> PC12
		 * B -> PA15
		 * C -> PC2
		 * D -> PC3
		 * E -> PC1
		 * F -> PB0
		 * G -> PC0
		 *
		 * Ánodos: PC10 y PC11
		 */
		handlerPuertoC12.pGPIOx 								= GPIOC;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinNumber			= PIN_12;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC12);
		GPIO_WritePin(&handlerPuertoC12, 0); //Enviamos todos los pines a un valor conocido

		handlerPuertoA15.pGPIOx 								= GPIOA;
		handlerPuertoA15.GPIO_PinConfig.GPIO_PinNumber			= PIN_15;
		handlerPuertoA15.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerPuertoA15.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoA15.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoA15.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoA15);
		GPIO_WritePin(&handlerPuertoA15, 0);

		handlerPuertoC0.pGPIOx 									= GPIOC;
		handlerPuertoC0.GPIO_PinConfig.GPIO_PinNumber			= PIN_0;
		handlerPuertoC0.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
		handlerPuertoC0.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC0.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC0.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC0);
		GPIO_WritePin(&handlerPuertoC0, 0);

		handlerPuertoC1.pGPIOx 									= GPIOC;
		handlerPuertoC1.GPIO_PinConfig.GPIO_PinNumber			= PIN_1;
		handlerPuertoC1.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
		handlerPuertoC1.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC1.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC1.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC1);
		GPIO_WritePin(&handlerPuertoC1, 0);

		handlerPuertoC2.pGPIOx 									= GPIOC;
		handlerPuertoC2.GPIO_PinConfig.GPIO_PinNumber			= PIN_2;
		handlerPuertoC2.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
		handlerPuertoC2.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC2.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC2.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC2);
		GPIO_WritePin(&handlerPuertoC2, 0);

		handlerPuertoC3.pGPIOx 									= GPIOC;
		handlerPuertoC3.GPIO_PinConfig.GPIO_PinNumber			= PIN_3;
		handlerPuertoC3.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
		handlerPuertoC3.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC3.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC3.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC3);
		GPIO_WritePin(&handlerPuertoC3, 0);

		handlerPuertoB0.pGPIOx 									= GPIOB;
		handlerPuertoB0.GPIO_PinConfig.GPIO_PinNumber			= PIN_0;
		handlerPuertoB0.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
		handlerPuertoB0.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoB0.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoB0.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoB0);
		GPIO_WritePin(&handlerPuertoB0, 0);

}//Termina el initHardware


/*Funcion para configurar los numeros que se van a visualizar*/
void Numero7Segmentos (uint8_t numero){
	switch (numero){
		case 0:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 0);	//E
			GPIO_WritePin(&handlerPuertoB0, 0);	//F
			GPIO_WritePin(&handlerPuertoC0, 1);	//G
			break;

		}
		case 1:{
			GPIO_WritePin(&handlerPuertoC12,1); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 1);	//D
			GPIO_WritePin(&handlerPuertoC1, 1);	//E
			GPIO_WritePin(&handlerPuertoB0, 1);	//F
			GPIO_WritePin(&handlerPuertoC0, 1);	//G
			break;

		}
		case 2:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 1);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 0);	//E
			GPIO_WritePin(&handlerPuertoB0, 1);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;

		}
		case 3:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 1);	//E
			GPIO_WritePin(&handlerPuertoB0, 1);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;

		}
		case 4:{
			GPIO_WritePin(&handlerPuertoC12,1); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 1);	//D
			GPIO_WritePin(&handlerPuertoC1, 1);	//E
			GPIO_WritePin(&handlerPuertoB0, 0);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;

		}
		case 5:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,1);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 1);	//E
			GPIO_WritePin(&handlerPuertoB0, 0);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;

		}
		case 6:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,1);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 0);	//E
			GPIO_WritePin(&handlerPuertoB0, 0);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;


		}
		case 7:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 1);	//D
			GPIO_WritePin(&handlerPuertoC1, 1);	//E
			GPIO_WritePin(&handlerPuertoB0, 1);	//F
			GPIO_WritePin(&handlerPuertoC0, 1);	//G
			break;

		}
		case 8:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 0);	//E
			GPIO_WritePin(&handlerPuertoB0, 0);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;

		}
		case 9:{
			GPIO_WritePin(&handlerPuertoC12,0); //A
			GPIO_WritePin(&handlerPuertoA15,0);	//B
			GPIO_WritePin(&handlerPuertoC2, 0);	//C
			GPIO_WritePin(&handlerPuertoC3, 0);	//D
			GPIO_WritePin(&handlerPuertoC1, 1);	//E
			GPIO_WritePin(&handlerPuertoB0, 0);	//F
			GPIO_WritePin(&handlerPuertoC0, 0);	//G
			break;

		}

		default:{
			__NOP();
			break;
		}

	}
}//Fin de la funcion Numero7Segmentos


/* Funcion para configurar la visualizacion
 * del segmento en cada posicion del modo "Culebrita"
 */
void CulebritaConfig (uint8_t pos){

	switch (pos){

	case 0:{
		GPIO_WritePin(&handlerPuertoC12,0); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 1:{
		GPIO_WritePin(&handlerPuertoC12,0); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 2:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 0);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 3:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 0);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 4:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 0);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 5:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 0);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 6:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 0);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 7:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,0);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 8:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 0);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 9:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 0);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 10:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 0);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case 11:{
		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,0);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	case ALL_OFF:{
		//Este caso es para cuando el segmento no este
		//en uno de los displays, este display se encuentre
		//totalmente apagado.

		GPIO_WritePin(&handlerPuertoC12,1); //A
		GPIO_WritePin(&handlerPuertoA15,1);	//B
		GPIO_WritePin(&handlerPuertoC2, 1);	//C
		GPIO_WritePin(&handlerPuertoC3, 1);	//D
		GPIO_WritePin(&handlerPuertoC1, 1);	//E
		GPIO_WritePin(&handlerPuertoB0, 1);	//F
		GPIO_WritePin(&handlerPuertoC0, 1);	//G

		break;
	}

	default:{
		break;
	}
	}
}//Fin de la funcion CulebritaConfig


/*Funcion para indicar que display esta activo
 * en cada posicion del modo "Culebrita"
 */
uint8_t DisplaySelect (int8_t pos_disp){

	uint8_t display =0;
	switch (pos_disp){

	case 0:	{display =DISPUNI;break;}
	case 1:	{display =DISPDEC;break;}
	case 2:	{display =DISPDEC;break;}
	case 3:	{display =DISPDEC;break;}
	case 4:	{display =DISPDEC;break;}
	case 5:	{display =DISPUNI;break;}
	case 6:	{display =DISPUNI;break;}
	case 7:	{display =DISPDEC;break;}
	case 8:	{display =DISPDEC;break;}
	case 9:	{display =DISPUNI;break;}
	case 10:{display =DISPUNI;break;}
	case 11:{display =DISPUNI;break;}
	default:{break;}
	}
	return display;

}//Fin de la funcion DisplaySelect


/*Callbacks de los timers*/
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);
}

void BasicTimer3_Callback(void){
	//Interrupcion del Timer que controla el switcheo del 7 segmentos
	Timer3Flag =1;
}


/*Callbacks de los EXTI*/
void callback_extInt0(void){
	//Interrupcion que controla el "Modo" del 7 segmentos,
	//ya sea conteo o culebrita (asociado al boton SW del encoder)
	ModeFlag ^=1;
}

void callback_extInt4(void){
	//Interrupcion ocacionada por la rotacion del encoder
	Exti4Flag =1;
	data = GPIO_ReadPin(&handlerPuertoA1); //Lee el estado de DT
	clk = GPIO_ReadPin(&handlerPuertoA4);  //Lee el estado del Clock

}



