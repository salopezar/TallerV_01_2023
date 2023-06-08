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
 * y el uso del reloj RTC (real time clock) interno del microcontrolador.
 *
 */

/* Cabeceras de las librerías */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f4xx.h"
#include <string.h>
#include "float.h"
#include "math.h"

/* Cabeceras de los drivers */
#include "ExtiDriver.h"
#include "GPIOxDriver.h"
#include "PLLDriver.h"
#include "BasicTimer.h"
#include "USARTxDriver.h"
#include "adcDriver.h"
#include "PwmDriver.h"
#include "I2CDriver.h"
#include "RTCDriver.h"

//Definición de los handlers necesarios
GPIO_Handler_t handlerStateLED				= {0};
GPIO_Handler_t handlerUSARTPINTX			= {0};
GPIO_Handler_t handlerUSARTPINRX			= {0};
GPIO_Handler_t handlerPinFrecuency 			= {0};

BasicTimer_Handler_t handlerBlinkyTimer 	= {0};
BasicTimer_Handler_t handlerTimer5			= {0};

ADC_Config_t adcConfig = {0};

USART_Handler_t USART6Comm = {0};

uint8_t adcIsComplete = 0;
uint16_t dataADC[1] = {0};
char buffer[64] = {0};
uint8_t cont = 0;
uint8_t numberOfConversion = 2;
uint16_t cont2 = 0;
uint16_t dataADCChannel0[256];
uint16_t dataADCChannel1[256];

// PWM para el muestreo a las frecuencias que se quieren.
GPIO_Handler_t HandlerPWM_1 = {0};
PWM_Handler_t handlerTIM3PWM_1 = {0};

// Para el PLL.
// Se nombra el PLL que cambia la frecuencia de operación del micro.
PLL_Handler_t handlerPLL = {0};

//Definición de las cabeceras de las funciones del main
void initHardware(void);
// Función para los comandos
void parseCommands(char *ptrBufferReception);
// Handlers de los comandos
// variables y funciones para los comandos con USART
uint8_t counterReception = 0;
char bufferReception[64] = {0};
char cmd[64] = {0};
bool stringComplete = false;
char userMsg[64] = {0};
unsigned int firstParameter = 0;
unsigned int secondParameter = 0;
unsigned int thirdparameter = 0;
uint8_t rxData = {0};

// handler RTC
RTC_Handler_t handlerRTC = {0};

// variables RTC
uint8_t segundos;
uint8_t minutos;
uint8_t horas;

//punteros
uint8_t *ptrTime;
uint8_t *ptrDate;
void nameMonth (uint8_t numberMonth);

/* Función principal del programa */
int main(void){

	/* inicialización de todos los elementos del sistema */
	initHardware();

	/* Loop infinito */
	while(1){

		if (rxData != '\0'){
			bufferReception[counterReception] = rxData;
			counterReception++;
			if(rxData == '@'){
				stringComplete = true;
				bufferReception[counterReception] = '\0';
				counterReception = 0;
			}
			rxData = '\0';
		}

		if(stringComplete){
			parseCommands(bufferReception);
			stringComplete = false;
		}

	}
	return 0;
}

// Función donde se configuran los pines en general.
void initHardware(void){

	// Se desactiva el reloj HSE
	RCC->CR &= ~(RCC_CR_HSEON);

	// Se configura el PLL con los parámetros dados.
	handlerPLL.PLL_Config.PLL_voltage		= VOLTAGE_100MHZ;
	handlerPLL.PLL_Config.PLL_frecuency		= FRECUENCY_100MHZ;
	PLL_Config(&handlerPLL);
	getConfigPLL();

	/* Configuración del LED de estado */
	handlerStateLED.pGPIOx 									= GPIOH;
	handlerStateLED.GPIO_PinConfig.GPIO_PinNumber 			= PIN_1;
	handlerStateLED.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_OUT;
	handlerStateLED.GPIO_PinConfig.GPIO_PinOPType 			= GPIO_OTYPE_PUSHPULL;
	handlerStateLED.GPIO_PinConfig.GPIO_PinSpeed 			= GPIO_OSPEED_FAST;
	handlerStateLED.GPIO_PinConfig.GPIO_PinPuPdControl 		= GPIO_PUPDR_NOTHING;
	GPIO_Config(&handlerStateLED);

	/* Configuración del TIM2 para que haga un blinky cada 250 ms */
	handlerBlinkyTimer.ptrTIMx 								= TIM2;
	handlerBlinkyTimer.TIMx_Config.TIMx_mode 				= BTIMER_MODE_UP;
	handlerBlinkyTimer.TIMx_Config.TIMx_speed				= BTIMER_SPEED_100MHz;
	handlerBlinkyTimer.TIMx_Config.TIMx_period 				= 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable 	= BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerBlinkyTimer);

	/* Configuración del USART */
	// Transmisión
	handlerUSARTPINTX.pGPIOx  							= GPIOA;
	handlerUSARTPINTX.GPIO_PinConfig.GPIO_PinNumber 	= PIN_11;
	handlerUSARTPINTX.GPIO_PinConfig.GPIO_PinMode 		= GPIO_MODE_ALTFN;
	handlerUSARTPINTX.GPIO_PinConfig.GPIO_PinAltFunMode = AF8;
	GPIO_Config(&handlerUSARTPINTX);

	// Recepción
	handlerUSARTPINRX.pGPIOx 						 	= GPIOA;
	handlerUSARTPINRX.GPIO_PinConfig.GPIO_PinNumber		= PIN_12;
	handlerUSARTPINRX.GPIO_PinConfig.GPIO_PinMode 		= GPIO_MODE_ALTFN;
	handlerUSARTPINRX.GPIO_PinConfig.GPIO_PinAltFunMode = AF8;
	GPIO_Config(&handlerUSARTPINRX);

	// Para el USART 6
	USART6Comm.ptrUSARTx 					= USART6;
	USART6Comm.USART_Config.USART_baudrate 	= USART_BAUDRATE_115200;
	USART6Comm.USART_Config.USART_datasize 	= USART_DATASIZE_8BIT;
	USART6Comm.USART_Config.USART_parity 	= USART_PARITY_NONE;
	USART6Comm.USART_Config.USART_mode 		= USART_MODE_RXTX;
	USART6Comm.USART_Config.USART_stopbits 	= USART_STOPBIT_1;
	USART6Comm.USART_Config.USART_enableIntRX = USART_RX_INTERRUP_ENABLE;

	// Se carga lo hecho sobre el USART
	USART_Config(&USART6Comm);

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
	adcConfigEvents(&adcConfig);

	// El PWM para muestrear la señal a la frecuencias que se quiere.
	HandlerPWM_1.pGPIOx          					= GPIOC;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinNumber  	= PIN_7;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinMode    	= GPIO_MODE_ALTFN;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinOPType  	= GPIO_OTYPE_PUSHPULL;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_OSPEED_FAST;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinAltFunMode  = AF2;

	GPIO_Config(&HandlerPWM_1);

	handlerTIM3PWM_1.ptrTIMx           	  =   TIM5;
	handlerTIM3PWM_1.config.channel       =   PWM_CHANNEL_3;
	handlerTIM3PWM_1.config.duttyCicle    =   1500;
	handlerTIM3PWM_1.config.periodo       =   20000;
	handlerTIM3PWM_1.config.prescaler     =   100;

	pwm_Config(&handlerTIM3PWM_1);

	enableOutput(&handlerTIM3PWM_1);
	startPwmSignal(&handlerTIM3PWM_1);

	handlerPinFrecuency.pGPIOx 									= GPIOA;
	handlerPinFrecuency.GPIO_PinConfig.GPIO_PinNumber 			= PIN_8;
	handlerPinFrecuency.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinFrecuency.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF0;
	GPIO_Config(&handlerPinFrecuency);

	handlerRTC.RTC_Days = 1;
	handlerRTC.RTC_Hours = 10;
	handlerRTC.RTC_Minutes = 30;
	handlerRTC.RTC_Seconds = 10;

	rtc_Config(&handlerRTC);

} // Fin initHardware

void parseCommands(char *ptrBufferReception){

	/* Esta funcion de C lee la cadena de caracteres a la que apunta el "ptr" y la divide
	 * y almacena en tres elementos diferentes : un string llamado "cmd", y dos numeros
	 * integer llamados "firstParameter" y "secondParameter".
	 * De esta forma, podemos introducir información al micro desde el puerto serial.
	 */
	sscanf(ptrBufferReception,"%s %u %u %u %s",cmd,&firstParameter,&secondParameter,&thirdparameter,userMsg);
	//Este primer comando imprime una lista con los otros comandos que tiene el equipo
	if (strcmp(cmd, "help") == 0){

		writeMsg(&USART6Comm, "BIENVENIDO AL PARCIAL MÁS DECISIVO DE MI VIDA :3\n");
		writeMsg(&USART6Comm, "▒▒▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▒▒▒▒▒▓▓▓\n");
		writeMsg(&USART6Comm, "▒▓▓▓▓▓▓░░░▓\n");
		writeMsg(&USART6Comm, "▒▓░░░░▓░░░░▓\n");
		writeMsg(&USART6Comm, "▓░░░░░░▓░▓░▓\n");
		writeMsg(&USART6Comm, "▓░░░░░░▓░░░▓\n");
		writeMsg(&USART6Comm, "▓░░▓░░░▓▓▓▓\n");
		writeMsg(&USART6Comm, "▒▓░░░░▓▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▓▓▓▓▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▒▒▒▒▒▒▓▓▓▓\n");
		writeMsg(&USART6Comm, "▒▒▒▒▒▓▓▓▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▒▒▓▒▒▒▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▒▓▒▒▒▒▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▓▒▒▒▒▒▒▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▓▒▓▒▒▒▒▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▓▒▓▓▓▓▓▓▓▓▓▓\n");
		writeMsg(&USART6Comm, "▒▓▒▒▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "▒▒▓▒▒▒▒▒▓\n");
		writeMsg(&USART6Comm, "Help Menu CMDs:\n");
		writeMsg(&USART6Comm, "1)help                     --Imprime este menu \n");
		writeMsg(&USART6Comm, "2)Select clock signal      --PLL,LSE,HSI\n");
		writeMsg(&USART6Comm, "3)Select prescaler         --MCO1 : 1,2,3,4,5\n");
		writeMsg(&USART6Comm, "4)set Time         		      --Hora inicial: Horas: minutos: segundos\n");
		writeMsg(&USART6Comm, "5)current time        	    --Hora actual: Horas: minutos: segundos\n");
		writeMsg(&USART6Comm, "6)init ADC        	    --Inicializa y muestra 256 datos de 2 canales de ADC\n");
		writeMsg(&USART6Comm, "7)signal sampling       	    --Periodo de muestreo de la señal de PWM\n");


	}else if(strcmp(cmd, "select_Clock_Signal") == 0){
		if(firstParameter == 1){
		chooseCLK(firstParameter);
		writeMsg(&USART6Comm, "selected PLL\n");
		}else if(firstParameter == 2){
		chooseCLK(firstParameter);
		writeMsg(&USART6Comm, "selected LSE\n");
		}else if(firstParameter == 3){
		chooseCLK(firstParameter);
		writeMsg(&USART6Comm, "selected HSI\n");
		}

	}else if(strcmp(cmd, "select_Prescaler") == 0){
		if(firstParameter == 1){
		prescalerNumber(firstParameter);
		writeMsg(&USART6Comm, "Prescaler null = 1\n");
		}else if(firstParameter == 2){
		prescalerNumber(firstParameter);
		writeMsg(&USART6Comm, "division in 2\n");
		}else if(firstParameter == 3){
		prescalerNumber(firstParameter);
		writeMsg(&USART6Comm, "division in 3\n");
		}else if(firstParameter == 4){
		prescalerNumber(firstParameter);
		writeMsg(&USART6Comm, "division in 4\n");
		}else if(firstParameter == 5){
		prescalerNumber(firstParameter);
		writeMsg(&USART6Comm, "division in 5\n");
		}
	}else if(strcmp(cmd,"set_Time") == 0){
		handlerRTC.RTC_Hours = firstParameter;
		handlerRTC.RTC_Minutes = secondParameter;
		handlerRTC.RTC_Seconds = thirdparameter;
		rtc_Config(&handlerRTC);
		sprintf(buffer,"Hora %u : %u : %u \n",firstParameter,secondParameter,thirdparameter);
		writeMsg(&USART6Comm, buffer);
	}else if(strcmp(cmd,"current_time") == 0){
		ptrTime = read_Time();
		segundos = ptrTime[0];
		minutos = ptrTime[1];
		horas	= ptrTime[2];
		sprintf(buffer,"Hora Actual %u : %u : %u \n",horas,minutos,segundos);
		writeMsg(&USART6Comm, buffer);
	}else if(strcmp(cmd,"init_ADC") == 0){
		if(adcIsComplete == 1){
			for(uint16_t j = 0; j < 257; j++){
				sprintf(buffer, "Lectura Canal 0 y 1: %u, %u\n", dataADCChannel0[j], dataADCChannel1[j]);
				writeMsg(&USART6Comm, buffer);
			}
			adcIsComplete = 0;
		}
	}else if(strcmp(cmd,"signal_Sampling") == 0){
//		float periodoMuestreo = 1 / firstParameter;
		handlerTIM3PWM_1.config.periodo = firstParameter;
		if(firstParameter >= 66 && firstParameter <= 125){
			pwm_Config(&handlerTIM3PWM_1);
			sprintf(buffer,"Velocidad de muestreo %u \n",firstParameter);
			writeMsg(&USART6Comm, buffer);
		}else{
			writeMsg(&USART6Comm, "Invalid signal_Sampling\n");
		}
	}
}

/* Timer que gobierna el blinky del led de estado */
void BasicTimer2_Callback(void){
	GPIOxTooglePin (&handlerStateLED);
}

void usart6Rx_Callback(void){
	rxData = getRxData();
}

// Esta función controla el contador del ADC.
void adcComplete_Callback(void){

//	if(cont < numberOfConversion){
//		dataADC[cont] = getADC();
//		cont++;
//	}
//	else{
//		adcIsComplete = 1;
//	}


	if(cont == 0){
		dataADCChannel0[cont2] = getADC();
	}
	else{
		dataADCChannel1[cont2] = getADC();
		cont2++;
	}
	cont++;
	if(cont2 == 256){
		cont2 = 0;
		adcIsComplete = 1;
	}
	if(cont == 2){
		cont = 0;
	}
}


// Función para decirle al micro qué mes es cada número.
//void nameMonth (uint8_t numberMonth){
//	if(numberMonth == 1){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Enero\n");
//	}else if(numberMonth == 2){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Febrero\n");
//	}else if(numberMonth == 3){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Marzo\n");
//	}else if(numberMonth == 4){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Abril\n");
//	}else if(numberMonth == 5){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Mayo\n");
//	}else if(numberMonth == 6){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Junio\n");
//	}else if(numberMonth == 7){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Julio\n");
//	}else if(numberMonth == 8){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Agosto\n");
//	}else if(numberMonth == 9){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Septiembre\n");
//	}else if(numberMonth == 10){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Octubre\n");
//	}else if(numberMonth == 11){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Noviembre\n");
//	}else if(numberMonth == 12){
//		handlerRTC.RTC_Months = secondParameter;
//		writeMsg(&USART6Comm, "Diciembre\n");
//	}
//}
