/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
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

#include <stdint.h>
#include <stm32f4xx.h>

#include "GPIOxDriver.h"

GPIO_Handler_t handlerLed2 = {0};

int main(void){

  /* Configurar el pin */
	handlerLed2.pGPIOx 								= GPIOA;
	handlerLed2.GPIO_PinConfig.GPIO_PinNumber		= PIN_5;
	handlerLed2.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerLed2.GPIO_PinConfig.GPIO_PinOPType		= GPIO_OTYPE_PUSHPULL;
	handlerLed2.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerLed2.GPIO_PinConfig.GPIO_PinSpeed		= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerLed2);

	GPIO_WritePin(&handlerLed2, SET);

	while(1){
		GPIO_TooglePin(&handlerLed2);

		for(int i = 0; i < 2000000 ; i++){
			__NOP();
		}
	}
}

