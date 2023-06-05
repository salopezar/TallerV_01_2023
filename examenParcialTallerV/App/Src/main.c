/**
 ******************************************************************************
 * @file           : main.c
 * @author         : SANTIAGO LÓPEZ ARANZAZU - CC. 1007429871
 * @e-mail         : salopezar@unal.edu.co
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/*************************** EXAMEN PARCIAL - 2023 ******************************/
/*
 * Este programa busca unificar algunas ideas y conceptos trabajados en el
 * curso de taller 5 anteriormente, a través de la integración de algunos drivers
 * que muestran la introducción a algunos protocolos de comunicación como USART e
 * I2C y la generación de señales PWM para el control de algunos dispositivos
 * externos al microcontrolador mediante señales cuadradas con características de
 * frecuencia y anchos de pulso determinados por lecturas de sensores externos o
 * de acuerdo al criterio que se requiera.
 *
 * Además, se implementa un nuevo driver que permite variar la frecuencia natural
 * de funcionamiento de 16 MHz del microcontrolador (PLL), que a su vez, obliga a
 * cambiar algunas frecuencias de transmisión en los protocolos de comunicación con
 * los que se ha trabajado en clase. Dado que en la tarea anterior no se incluyó
 * trabajo sobre el módulo de conversión análogo-digital ADC, en el presente se
 * muestra cómo lograr un driver para conseguir varias conversiones consecutivas
 * y el uso del reloj RTC interno del microcontrolador.
 *
 */

/*Librerias*/
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f4xx.h"
#include "ExtiDriver.h"
#include "GPIOxDriver.h"
#include "BasicTimer.h"
#include "USARTxDriver.h"
#include "adcDriver.h"

//Definición de los handlers necesarios
GPIO_Handler_t handlerStateLED				= {0};
GPIO_Handler_t handlerPinTx 				= {0};
GPIO_Handler_t handlerPinRx 				= {0};

BasicTimer_Handler_t handlerBlinkyTimer 	= {0};
BasicTimer_Handler_t handlerTimer5			= {0};

ADC_Config_t adcConfig = {0};

USART_Handler_t USART2Comm = {0};

uint8_t adcIsComplete = 0;
uint16_t dataADC[1] = {0};
char buffer[64] = {0};
uint8_t cont = 0;
uint8_t numberOfConversion = 2;

//Definición de las cabeceras de las funciones del main
void initSystem(void);

/* Función principal del programa */
int main(void){

	/* inicialización de todos los elementos del sistema */
	initSystem();
	writeMsg(&USART2Comm, "Hello i'm working");

	/* Loop infinito */
	while(1){
		if(adcIsComplete == 1){
			sprintf(buffer, "Data Canal 0 y 1: %u, %u\n", dataADC[0], dataADC[1]);
			writeMsg(&USART2Comm, buffer);
			adcIsComplete = 0;
			cont = 0;
		}

	}		// Fin while infinito
	return 0;
}			// Fin main

/* Función en la que se configuran los pines a usar, los EXTI y los timers */
void initSystem(void){

	/* Configuración del LED2 - PA5 */
	handlerStateLED.pGPIOx 									= GPIOA;
	handlerStateLED.GPIO_PinConfig.GPIO_PinNumber 			= PIN_5;
	handlerStateLED.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_OUT;
	handlerStateLED.GPIO_PinConfig.GPIO_PinOPType 			= GPIO_OTYPE_PUSHPULL;
	handlerStateLED.GPIO_PinConfig.GPIO_PinSpeed 			= GPIO_OSPEED_FAST;
	handlerStateLED.GPIO_PinConfig.GPIO_PinPuPdControl 		= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerStateLED);

	/* Configuración del TIM2 para que haga un blinky cada 250 ms */
	handlerBlinkyTimer.ptrTIMx 								= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode 				= BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed				= BTIMER_SPEED_100us;
	handlerBlinkyTimer.TIMx_Config.TIMx_period 				= 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable 	= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerBlinkyTimer);


	/* Configuración del TIM5*/
	handlerTimer5.ptrTIMx 								= TIM5;
	handlerTimer5.TIMx_Config.TIMx_mode 				= BTIMER_MODE_UP;
	handlerTimer5.TIMx_Config.TIMx_speed 				= BTIMER_SPEED_100us;
	handlerTimer5.TIMx_Config.TIMx_period 				= 10000;
	handlerTimer5.TIMx_Config.TIMx_interruptEnable 		= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerTimer5);

	/* Configuracion de la comunicacion serial */
	/* Pin TX USART6 */
	handlerPinTx.pGPIOx 								= GPIOA;
	handlerPinTx.GPIO_PinConfig.GPIO_PinNumber 			= PIN_2;
	handlerPinTx.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinTx.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF7;
	GPIO_Config(&handlerPinTx);

	/* Pin RX USART6 */
	handlerPinRx.pGPIOx 								= GPIOA;
	handlerPinRx.GPIO_PinConfig.GPIO_PinNumber 			= PIN_3;
	handlerPinRx.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinRx.GPIO_PinConfig.GPIO_PinAltFunMode		= AF7;
	GPIO_Config(&handlerPinRx);

	/* Configuración USART2 */
	USART2Comm.ptrUSARTx 						= USART2;
	USART2Comm.USART_Config.USART_baudrate 		= USART_BAUDRATE_115200;
	USART2Comm.USART_Config.USART_datasize 		= USART_DATASIZE_8BIT;
	USART2Comm.USART_Config.USART_parity 		= USART_PARITY_NONE;
	USART2Comm.USART_Config.USART_stopbits 		= USART_STOPBIT_1;
	USART2Comm.USART_Config.USART_mode 			= USART_MODE_RXTX;
	USART2Comm.USART_Config.USART_enableIntRX   = USART_RX_INTERRUP_ENABLE;
	USART2Comm.USART_Config.USART_enableIntTX   = USART_TX_INTERRUP_DISABLE;
	USART_Config(&USART2Comm);

	/* Configuración ADC */

	// Cargando la configuración para la conversación ADC
	adcConfig.dataAlignment			= ADC_ALIGNMENT_RIGHT;
	adcConfig.resolution			= ADC_RESOLUTION_12_BIT;
	adcConfig.samplingPeriod		= ADC_SAMPLING_PERIOD_28_CYCLES;
	adcConfig.AdcEventType			= TIMER_ADC_EVENT;
	adcConfig.AdcChannelEvent		= TIM5_CH3;
	adcConfig.adcMultiChannel[0] 	= ADC_CHANNEL_0;
	adcConfig.adcMultiChannel[1]	= ADC_CHANNEL_1;
	adcMultiChannel(&adcConfig, 2);
	adcConfigExternal(&adcConfig);

} // Fin initSystem

/* Timer que gobierna el blinky del led de estado */
void BasicTimer2_Callback(void){
	GPIOxTooglePin (&handlerStateLED);
}

//void BasicTimer5_Callback(void){
//	startSingleADC();
//}

void adcComplete_Callback(void){

	if(cont < numberOfConversion){
		dataADC[cont] = getADC();
		cont++;
	}
	else{
		adcIsComplete = 1;
	}
}
