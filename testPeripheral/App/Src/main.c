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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <stm32f4xx.h>

#include "BasicTimer.h"
#include "GPIOxDriver.h"
#include "ExtiDriver.h"
#include "USARTxDriver.h"
#include "SysTickDriver.h"
#include "PwmDriver.h"

#define HSI_CLOCK_CONFIGURED 0  // 16MHz
#define HSE_CLOCK_CONFIGURED 1
#define PLL_CLOCK_CONFIGURED 2

GPIO_Handler_t handlerLedBlinky = {0};
BasicTimer_Handler_t handlerTIM2 = {0};

// Prueba del USART
GPIO_Handler_t handlerUSARTPINTX = {0};
GPIO_Handler_t handlerUSARTPINRX = {0};
USART_Handler_t USART2Comm = {0};

// Prueba del PWM
GPIO_Handler_t HandlerPWM = {0};
PWM_Handler_t handlerTIM3PWM = {0};

//Variables para pruebas

uint8_t sendMSG = 0;
char mensaje[] = "HOLA\n";
uint8_t newChar = 0;
uint16_t duttyValue = 1000;



int main(void){


	// Activamos el FPU
	//SCB->CPACR |= (0xF << 20);

	///Configuramos el pin A5 el cual se encargara del Led de estado

	handlerLedBlinky.pGPIOx                             = GPIOA;
	handlerLedBlinky.GPIO_PinConfig.GPIO_PinNumber      = PIN_5;
	handlerLedBlinky.GPIO_PinConfig.GPIO_PinMode        = GPIO_MODE_OUT;
	handlerLedBlinky.GPIO_PinConfig.GPIO_PinOPType      = GPIO_OTYPE_PUSHPULL;
	handlerLedBlinky.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	handlerLedBlinky.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_OSPEED_FAST;
	GPIO_Config(&handlerLedBlinky);
	GPIO_WritePin(&handlerLedBlinky, SET);

	//Configuramos el TIM2, para asì poder lograr el parpadeo en el led de estado
	handlerTIM2.ptrTIMx 							= TIM2;
	handlerTIM2.TIMx_Config.TIMx_mode				= BTIMER_MODE_UP;
	handlerTIM2.TIMx_Config.TIMx_period				= 500;
	handlerTIM2.TIMx_Config.TIMx_speed				= BTIMER_SPEED_1ms;
	BasicTimer_Config(&handlerTIM2);

	handlerUSARTPINTX.pGPIOx  = GPIOA;
	handlerUSARTPINTX.GPIO_PinConfig.GPIO_PinNumber = PIN_2;
	handlerUSARTPINTX.GPIO_PinConfig.GPIO_PinMode 	= GPIO_MODE_ALTFN;
	handlerUSARTPINTX.GPIO_PinConfig.GPIO_PinAltFunMode = AF7;
	GPIO_Config(&handlerUSARTPINTX);

	handlerUSARTPINRX.pGPIOx  = GPIOA;
	handlerUSARTPINRX.GPIO_PinConfig.GPIO_PinNumber= PIN_3;
	handlerUSARTPINRX.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	handlerUSARTPINRX.GPIO_PinConfig.GPIO_PinAltFunMode = AF7;
	GPIO_Config(&handlerUSARTPINRX);

	USART2Comm.ptrUSARTx = USART2;
	USART2Comm.USART_Config.USART_baudrate = USART_BAUDRATE_115200;
	USART2Comm.USART_Config.USART_datasize = USART_DATASIZE_8BIT;
	USART2Comm.USART_Config.USART_parity = USART_PARITY_NONE;
	USART2Comm.USART_Config.USART_mode 	= USART_MODE_RXTX;
	USART2Comm.USART_Config.USART_stopbits = USART_STOPBIT_1;
	USART2Comm.USART_Config.USART_enableIntRX = USART_RX_INTERRUP_ENABLE;


	USART_Config(&USART2Comm);

	HandlerPWM.pGPIOx          = GPIOC;
	HandlerPWM.GPIO_PinConfig.GPIO_PinNumber  = PIN_7;
	HandlerPWM.GPIO_PinConfig.GPIO_PinMode    = GPIO_MODE_ALTFN;
	HandlerPWM.GPIO_PinConfig.GPIO_PinOPType  = GPIO_OTYPE_PUSHPULL;
	HandlerPWM.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	HandlerPWM.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_OSPEED_FAST;
	HandlerPWM.GPIO_PinConfig.GPIO_PinAltFunMode  = AF2;

	GPIO_Config(&HandlerPWM);


	handlerTIM3PWM.ptrTIMx           	= 	TIM3;
	handlerTIM3PWM.config.channel       =   PWM_CHANNEL_2;
	handlerTIM3PWM.config.duttyCicle    =   duttyValue;
	handlerTIM3PWM.config.periodo       =   20000;
	handlerTIM3PWM.config.prescaler     =   16;

	pwm_Config(&handlerTIM3PWM);

	enableOutput(&handlerTIM3PWM);
	startPwmSignal(&handlerTIM3PWM);




    while(1){

    	if (sendMSG != 0){
    		writeMsg(&USART2Comm, mensaje);
    		sendMSG = 0;
    	}
    	updateDuttyCycle(&handlerTIM3PWM, duttyValue);
    }

}

void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerLedBlinky);
	if (duttyValue <= 2000){
		duttyValue = duttyValue + 150;
	}else{
		duttyValue = 1000;
	}
	sendMSG++;


}

void usart2Rx_Callback(void){
	newChar = getRxData();
	writeChar(&USART2Comm, newChar);
}
