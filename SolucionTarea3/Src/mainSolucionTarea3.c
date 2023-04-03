/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Main program body
 ******************************************************************************
*COnfiguracion base del ambiente de desarrollo
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
//y permitiran ejecutar en el main el codigo relacionado
//a su interrupcion correspondiente

uint8_t Exti4Flag	=0;
uint8_t Timer3Flag 	=0;
uint8_t ModeFlag	=0;


/*Variables*/
uint8_t data 				=0;
uint8_t clk					=0;
int8_t unitCounter  		=0;
int8_t decimalCounter  		=0;
int8_t culebritaCounter 	=0;

/*Funciones*/
void initHardware (void);
void Numero7Segmentos (uint8_t numero);
void CulebritaConfig (uint8_t pos);
uint8_t DisplaySelect (int8_t pos_disp);

uint8_t var1 =0;

int main(void) {

	/*Cargamos toda la configuracion del hardware*/
	initHardware ();


	/* Loop forever*/
	while (1) {

		switch (ModeFlag){

		case 0:{

			if(Timer3Flag){
				GPIOxTooglePin(&handlerPuertoC11);
				GPIOxTooglePin(&handlerPuertoC10);

				/*Configuramos como se van a mostrar los numeros en el 7 segmentos*/
				if(GPIO_ReadPin(&handlerPuertoC11)){
					Numero7Segmentos(decimalCounter);
					//Si el puerto C11(el de las decimas) esta apagado (con 1 logico)
					//el de las unidades es el que esta encendido y se configura

				}else{
					Numero7Segmentos(unitCounter);
					//Si el puerto C11(el de las decimas) esta encendido (con 0 logico)
					//se confura ese mismo
				}
			Timer3Flag =0;
			}//Fin del 'if' del Timer3


			/*Configurando el contador de unidades
			 * y decimas segun el sentido de giro del encoder
			 */
			if(Exti4Flag){

				if (data > clk){
					unitCounter++;

					if(unitCounter > 9){
						unitCounter =0;
						decimalCounter++;

						if (decimalCounter > 9){
							unitCounter 	=9;
							decimalCounter 	=9;
						}
					}


				}else{
					unitCounter--;

					if(unitCounter < 0){
						unitCounter =9;
						decimalCounter--;

						if (decimalCounter < 0){
							unitCounter 	=0;
							decimalCounter  =0;
						}
					}
				}
				Exti4Flag =0;
			}//Fin del 'if' del Exti del encoder

			break;
		}//Fin del case 0


		case 1:{

			if(Timer3Flag){
				GPIOxTooglePin(&handlerPuertoC10);
				GPIOxTooglePin(&handlerPuertoC11);

				var1 = DisplaySelect(culebritaCounter);

//				switch (DisplaySelect(culebritaCounter)){
//				case DISPUNI:{
				if(var1 ==0){

					if(GPIO_ReadPin(&handlerPuertoC11)){
						//Si el puerto C11(el de las unidades) esta apagado (con 1 logico)
						//el de las decimas es el que esta encendido y se configura
						CulebritaConfig(ALL_OFF);

					}else{
						CulebritaConfig(culebritaCounter);
					}

				}else{
					if(GPIO_ReadPin(&handlerPuertoC11)){
						//Si el puerto C11(el de las unidades) esta apagado (con 1 logico)
						//el de las decimas es el que esta encendido y se configura
						CulebritaConfig(culebritaCounter);

					}else{
						CulebritaConfig(ALL_OFF);
					}
				}

//				case DISPDEC:{

//					if(GPIO_ReadPin(&handlerPuertoC11)){
//						//Si el puerto C11(el de las unidades) esta apagado (con 1 logico)
//						//el de las decimas es el que esta encendido y se configura
//						CulebritaConfig(culebritaCounter);
//
//					}else{
//						CulebritaConfig(ALL_OFF);
//					}
//				//}
//				//}
				Timer3Flag =0;
			}

			if(Exti4Flag){

				if (data > clk){
					culebritaCounter++;

						if (culebritaCounter > 11){
							culebritaCounter 	=0;
						}
					}

				else{
					culebritaCounter--;

						if (culebritaCounter < 0){
							culebritaCounter =11;
						}
					}
				Exti4Flag =0;
				}

				break;
			}

		default:{
			break;
		}

		}
	}
}


void initHardware (void){

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
		GPIO_WritePin(&handlerPuertoC10, 0); //Hacemos que este pin sea un cero logico a la salida (Switchea unidades)

		handlerPuertoC11.pGPIOx 								= GPIOC;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinNumber			= PIN_11;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC11.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC11);
		GPIO_WritePin(&handlerPuertoC11, 1); //Hacemos que este pin sea un 1 logico a la salida (Switchea decenas)

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
		 * Anodos: PC10 y PC11
		 */
		handlerPuertoC12.pGPIOx 								= GPIOC;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinNumber			= PIN_12;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;
		handlerPuertoC12.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;

		GPIO_Config(&handlerPuertoC12);
		GPIO_WritePin(&handlerPuertoC12, 0);

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
		GPIO_WritePin(&handlerPuertoC1, 1);

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
		GPIO_WritePin(&handlerPuertoB0, 1);

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
}

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
}

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
	//ya sea contador o culebrita (asociado al boton SW del encoder)
	ModeFlag ^=1;
}

void callback_extInt4(void){
	//Interrupcion ocacionada por la rotacion del encoder
	Exti4Flag =1;
	data = GPIO_ReadPin(&handlerPuertoA1);
	clk = GPIO_ReadPin(&handlerPuertoA4);

}



