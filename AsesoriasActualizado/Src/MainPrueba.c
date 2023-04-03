/*
 * MainPrueba.c
 *
 *  Created on: Apr 3, 2023
 *      Author: algraciam
 */
#include <stdint.h>
#include <stm32f4xx.h>

#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "ExtiDriver.h"

// Se define y se inicializa el LED de estado.
GPIO_Handler_t handlerBlinkyPin = {0};
// Se definen y se inicializan los elementos del encoder.
GPIO_Handler_t handlerPinDT = {0};
GPIO_Handler_t handlerCLK = {0};
GPIO_Handler_t handlerButtonSW = {0};

BasicTimer_Handler_t handlerTim2 = {0};

EXTI_Config_t handlerExtiCLK = {0};
EXTI_Config_t handlerExtiButtonSW = {0};
EXTI_Config_t handlerExtiDT = {0};
// Se declaran las variables necesarias para las banderas
char flagClock = 0;
char flagButtonSW = 0;
char flagPinDT = 0;

uint8_t x = 0;
// Definición del contador
uint8_t counter = 0;
// Función que inicializa el sistema
void init_Hardware(void);
void BasicTimer2_Callback(void);
void callback_extInt2(void);
void callback_extInt1(void);


/* Se ajustan los parámetros
 * de la función central del programa
 */
int main(void){

//Se inician todos los elementos necesarios del sistema.
init_Hardware();

while(1){
/* Se definen las condiciones generales para la rotación del ENCODER */
// Se considera que cuando el encoder gire en sentido horario,
// hay incremento en el valor.
/* SI pinData está en alto y se detecta un flanco de bajada */

if(flagClock){
if(GPIO_ReadPin(&handlerPinDT) == 1){
counter++;
}


flagClock = 0;

if(counter <= 0){
counter = 0;
}
else if (counter >= 99) {
      counter = 99;
}

flagClock = 0;

}

}
}

void init_Hardware (void){

 /* Configuración del LED de estado que indica el funcionamiento del programa */
handlerBlinkyPin.pGPIOx = GPIOA;
handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5;
handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl    = GPIO_PUPDR_NOTHING;
handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OSPEED_FAST;

// Se selecciona el TIMER que se elegió trabajar en el programa.
handlerTim2.ptrTIMx = TIM2;

// Configuración general en que se va a manejar el timer.
handlerTim2.TIMx_Config.TIMx_mode = BTIMER_MODE_UP;
handlerTim2.TIMx_Config.TIMx_period = 250;
handlerTim2.TIMx_Config.TIMx_speed = BTIMER_SPEED_1ms;

// Se carga lo que se hizo sobre el pin A5
GPIO_Config(&handlerBlinkyPin);

/* Se carga ahora la configuración del TIMER */
BasicTimer_Config(&handlerTim2);

/* Se configuran las salidas del encoder */
// Configuración para la salida del clock (CLK) del encoder.
handlerCLK.pGPIOx = GPIOA;
handlerCLK.GPIO_PinConfig.GPIO_PinNumber = PIN_4;
handlerCLK.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
handlerCLK.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;


   // Se carga la configuración.
GPIO_Config(&handlerCLK);

handlerExtiCLK.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE;
handlerExtiCLK.pGPIOHandler = &handlerCLK;

   // Cargando la configuracion del EXTI
   ExtInt_Config(&handlerExtiCLK);

/* Se configuran las salidas del encoder */
// Configuración para la salida del DATA (DT) del encoder.
handlerPinDT.pGPIOx = GPIOA;
handlerPinDT.GPIO_PinConfig.GPIO_PinNumber = PIN_1;
handlerPinDT.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
handlerPinDT.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;

   // Se carga la configuración.
GPIO_Config(&handlerPinDT);

handlerExtiDT.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE;
handlerExtiDT.pGPIOHandler = &handlerPinDT;

   // Cargando la configuracion del EXTI
  ExtInt_Config(&handlerExtiDT);

/* Se configuran las salidas del encoder */
// Configuración para el botón (sw) del encoder.
handlerButtonSW.pGPIOx = GPIOA;
handlerButtonSW.GPIO_PinConfig.GPIO_PinNumber = PIN_0;
handlerButtonSW.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
handlerButtonSW.GPIO_PinConfig.GPIO_PinOPType = GPIO_OTYPE_PUSHPULL;
handlerButtonSW.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
handlerButtonSW.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OSPEED_FAST;

   // Se carga la configuración.
GPIO_Config(&handlerButtonSW);

handlerExtiButtonSW.edgeType = EXTERNAL_INTERRUPT_FALLING_EDGE;
handlerExtiButtonSW.pGPIOHandler = &handlerCLK;

   // Cargando la configuracion del EXTI
   ExtInt_Config(&handlerExtiButtonSW);
}

void BasicTimer2_Callback(void){
GPIOxTooglePin(&handlerBlinkyPin);
}

void callback_extInt3(void){
flagPinDT = 1;
}

void callback_extInt2(void){
flagClock = 1;
}

void callback_extInt1(void){
flagButtonSW = 1;
}

