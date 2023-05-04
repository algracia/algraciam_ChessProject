/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Alberto Gracia Martelo
 * @brief          : Este el main del proyecto final de tallerV
 ******************************************************************************

 ******************************************************************************
 */

#include <stdint.h>
#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "SysTickDriver.h"
#include "PwmDriver.h"


/*Configuramos los handlers*/
GPIO_Handler_t handlerOnBoardLed 			={0};
GPIO_Handler_t handlerSeñalM1				={0};
GPIO_Handler_t handlerSeñalM2				={0};
GPIO_Handler_t handlerDireccM1				={0};
GPIO_Handler_t handlerDireccM2				={0};
GPIO_Handler_t handlerSeñalServo			={0};
GPIO_Handler_t handlerUserButton			={0};

BasicTimer_Handler_t handlerTimerBlinky 	={0};

PWM_Handler_t handlerPwmM1					={0};
PWM_Handler_t handlerPwmM2					={0};
PWM_Handler_t handlerPwmServo				={0};

EXTI_Config_t handlerEXTIUserButton			={0};


/*Variables*/
uint8_t extiCount =0;
uint16_t pulseWidth = 10;

/*Headers de funciones*/
void InitHardware (void);

int main(void) {

	InitHardware();
	disableOutput(&handlerPwmM1);
	disableOutput(&handlerPwmM2);
	disableOutput(&handlerPwmServo);

	/* Loop forever*/
	while (1) {


	}
}

void InitHardware (void){
	/*Configuramos el Blinky*/
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

	GPIO_Config(&handlerOnBoardLed);
	BasicTimer_Config(&handlerTimerBlinky);


	/*Configuramos la Señal de los motores*/
	handlerSeñalM1.pGPIOx 								= GPIOC;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_7;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalM1.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	handlerSeñalM2.pGPIOx 								= GPIOC;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_6;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalM2.GPIO_PinConfig.GPIO_PinAltFunMode	= AF2;

	GPIO_Config(&handlerSeñalM1);
	GPIO_Config(&handlerSeñalM2);

	//PWM de los motores
	handlerPwmM1.ptrTIMx				=TIM3;
	handlerPwmM1.config.channel 		=PWM_CHANNEL_2;
	handlerPwmM1.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmM1.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmM1.config.periodo 		=10; //Equivale a un periodo de 1ms
	handlerPwmM1.config.pulseWidth 		=5; //Equivale a un PW de 0.5 ms o un DutyCicle de 50%

	handlerPwmM2.ptrTIMx				=TIM3;
	handlerPwmM2.config.channel 		=PWM_CHANNEL_1;
	handlerPwmM2.config.polarity 		=PWM_POLARITY_ACTIVE_LOW;
	handlerPwmM2.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmM2.config.periodo 		=10; //Equivale a un periodo de 1ms
	handlerPwmM2.config.pulseWidth 		=5; //Equivale a un PW de 0.5 ms o un DutyCicle de 50%

	//Cargamos las configuraciones
	pwm_Config(&handlerPwmM1);
	pwm_Config(&handlerPwmM2);

	//Activamos las señales,como para ambos motores es un mismo timer
	//basta con hacer esto para uno solo de los handlers
	startPwmSignal(&handlerPwmM1);

	//Direccion motores
	handlerDireccM1.pGPIOx 								= GPIOB;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinNumber		= PIN_2;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerDireccM1.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	handlerDireccM2.pGPIOx 								= GPIOC;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinNumber		= PIN_8;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerDireccM2.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerDireccM1);
	GPIO_Config(&handlerDireccM2);


	/*Configuramos la señal del Servomotor*/
	handlerSeñalServo.pGPIOx 								= GPIOB;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinNumber			= PIN_6;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_ALTFN;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	handlerSeñalServo.GPIO_PinConfig.GPIO_PinAltFunMode		= AF2;

	GPIO_Config(&handlerSeñalServo);

	//PWM del Servo
	handlerPwmServo.ptrTIMx					=TIM4;
	handlerPwmServo.config.channel 			=PWM_CHANNEL_1;
	handlerPwmServo.config.polarity 		=PWM_POLARITY_ACTIVE_HIGH;
	handlerPwmServo.config.prescaler 		=PWM_PRESCALER_100us;
	handlerPwmServo.config.periodo 			=200; //Equivale a un periodo de 20ms
	handlerPwmServo.config.pulseWidth 		=pulseWidth;

	//Cargamos la configuracion
	pwm_Config(&handlerPwmServo);

	//Activamos la señal
	startPwmSignal(&handlerPwmServo);

	/*Configuramos el UserButton*/
	handlerUserButton.pGPIOx								= GPIOC;
	handlerUserButton.GPIO_PinConfig.GPIO_PinNumber			= PIN_13;
	handlerUserButton.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	GPIO_Config(&handlerSeñalServo);

	//Configuracion del EXTI del UserButton
	handlerEXTIUserButton.pGPIOHandler					=&handlerUserButton;
	handlerEXTIUserButton.edgeType						=EXTERNAL_INTERRUPT_FALLING_EDGE;

	//Cargamos la configuracion del EXTI
	ExtInt_Config(&handlerEXTIUserButton);



}


/*Callbacks Timers*/
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerOnBoardLed);

}


/*Callback EXTI*/
void callback_extInt13(void){
	extiCount = 1;
}

