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
/***************************** TAREA ESPECIAL ********************************/
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
 */

// Cabeceras de las librerías.
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"

// Cabeceras de todos los perifericos que están en peripheral drivers.
#include "GPIOxDriver.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "BasicTimer.h"
#include "I2CDriver.h"
#include "PwmDriver.h"
#include "PLLDriver.h"

// Se nombra el PLL que cambia la frecuencia de operación del micro.
PLL_Handler_t handlerPLL = {0};

// Definicion de los handlers necesarios.
GPIO_Handler_t handlerBlinkyPin 	= {0};


// Timer del led de estado.
BasicTimer_Handler_t handlerBlinkyTimer = {0};


void init_Hardware(void);


int main(void)
{
	// Se llama la función de inicialización.
	init_Hardware();
	while(1){

	}
	return 0;
}

//Función de configuración de los elementos del sistema.
void init_Hardware(void){

	// Se configura el PLL con los parámetros dados.
	handlerPLL.PLL_Config.APB1_prescaler 	= APB1_PRESCALER_2;
	handlerPLL.PLL_Config.APB2_prescaler	= APB2_PRESCALER_0;
	handlerPLL.PLL_Config.PLL_voltage		= VOLTAGE_84MHZ;
	handlerPLL.PLL_Config.PLL_frecuency		= FRECUENCY_80MHZ;
	PLL_Config(&handlerPLL);

	/* LED DE ESTADO A 250 ms aproximadamente. */
	handlerBlinkyPin.pGPIOx 									= GPIOA;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber 				= PIN_5;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed  				= GPIO_OSPEED_FAST;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;

	// Se carga la configuración del Blinky,
	GPIO_Config(&handlerBlinkyPin);
	GPIO_WritePin(&handlerBlinkyPin, SET);

	// Se configura el timer del blinky.
	handlerBlinkyTimer.ptrTIMx								= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode				= BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed				= BTIMER_SPEED_80MHz;
	handlerBlinkyTimer.TIMx_Config.TIMx_period				= 25000;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable 	= BTIMER_INTERRUPT_ENABLE;

	// Se carga lo hecho sobre el timer del blinky.
	BasicTimer_Config(&handlerBlinkyTimer);
}

// Callback para el blinky pin.
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerBlinkyPin);
}
