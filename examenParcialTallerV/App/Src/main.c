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

// Definición de los handlers GPIO necesarios.
GPIO_Handler_t handlerStateLED				= {0};
GPIO_Handler_t handlerUSARTPINTX			= {0};
GPIO_Handler_t handlerUSARTPINRX			= {0};
GPIO_Handler_t handlerPinFrecuency 			= {0};
// Definimos los basic timer del blinky y de las banderas.
BasicTimer_Handler_t handlerBlinkyTimer 	= {0};
BasicTimer_Handler_t handlerTimer5			= {0};
BasicTimer_Handler_t handlerTimer4			= {0};
// Para la conversión ADC.
ADC_Config_t adcConfig = {0};
// Se define el handler de la comunicacion serial para el USART 6 que me corresponde.
USART_Handler_t USART6Comm = {0};
// PARA LA CONVERSIÓN ADC: los variables y los arreglos necesarios.
uint8_t adcIsComplete = 0;
uint16_t dataADC[1] = {0};
char buffer[64] = {0};
uint8_t cont = 0;
// Number of convertions son la cantidad de canales ADC que se necesitan.
uint8_t numberOfConversion = 2;
uint16_t cont2 = 0;
float dataADCChannel0[256];
float dataADCChannel1[256];

// PWM para el muestreo a las frecuencias que se quieren.
GPIO_Handler_t HandlerPWM_1 = {0};
PWM_Handler_t handlerTIM3PWM_1 = {0};

// Para el PLL.
// Se nombra el PLL que cambia la frecuencia de operación del micro.
PLL_Handler_t handlerPLL = {0};

/* Configración general para el acelerómetro ADXL-345 */
GPIO_Handler_t SDA = {0};
GPIO_Handler_t SCL = {0};
I2C_Handler_t Acelerometer = {0};
// Variables usadas dentro del I2C.
uint8_t i2cBuffer = {0};
uint8_t rxData = 0;
char bufferData[64] = "Accel ADXL-345 testing...";

// Direcciones para la comunicación con ADXL-345
#define ACCEL_ADDRESS          0x1D
#define ACCEL_X1_L             50
#define ACCEL_X1_H             51
#define ACCEL_Y1_L             52
#define ACCEL_Y1_H             53
#define ACCEL_Z1_L             54
#define ACCEL_Z1_H             55

#define POWER_CTL              45
#define WHO_AM_I               0
// Arreglos para mostrar los datos del acelerómetro
float X_1[1024] = {0};
float Y_1[1024] = {0};
float Z_1[1024] = {0};
uint16_t counter = {0};
uint8_t flag = 0;
uint8_t flag2 = 0;
// Variables para los ejes del acelerómetro a 2 decimales
float X_axis = {0};
float Y_axis = {0};
float Z_axis = {0};
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

// Para el real time clock
RTC_Handler_t handlerRTC = {0};
// variables RTC
uint8_t segundos;
uint8_t minutos;
uint8_t horas;
uint8_t dia;
uint8_t mes;
uint16_t año;
//punteros para conseguir los datos
uint8_t *ptrTime;
uint8_t *ptrDate;
void nameMonth (uint8_t numberMonth);

/* Función principal del programa */
int main(void){

	/* inicialización de todos los elementos del sistema */
	initHardware();

	/* Loop infinito */
	while(1){
		// Comunicacion para los comandos.
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
		// Envio de los strings
		if(stringComplete){
			parseCommands(bufferReception);
			stringComplete = false;
		}

	}
	return 0;
}// Final del main

// Función donde se configuran los pines en general.
void initHardware(void){

	// Se desactiva el reloj HSE porque PH0 está conectado a un oscilador HSE.
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

	/* Timer para las banderas necesarias para el muestreo del acelerometro */
	handlerTimer4.ptrTIMx 								    = TIM4;
	handlerTimer4.TIMx_Config.TIMx_mode 				    = BTIMER_MODE_UP;
	handlerTimer4.TIMx_Config.TIMx_speed				    = BTIMER_SPEED_100MHz;
	handlerTimer4.TIMx_Config.TIMx_period 				    = 50;
	handlerTimer4.TIMx_Config.TIMx_interruptEnable 	        = BTIMER_INTERRUPT_ENABLE;
	BasicTimer_Config(&handlerTimer4);

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
	// Se carga GPIO del PWM.
	GPIO_Config(&HandlerPWM_1);

	handlerTIM3PWM_1.ptrTIMx           	  =   TIM5;
	handlerTIM3PWM_1.config.channel       =   PWM_CHANNEL_3;
	handlerTIM3PWM_1.config.duttyCicle    =   1500;
	handlerTIM3PWM_1.config.periodo       =   20000;
	handlerTIM3PWM_1.config.prescaler     =   100;
	// Se carga el PWM.
	pwm_Config(&handlerTIM3PWM_1);
	// Se habilita la señal.
	enableOutput(&handlerTIM3PWM_1);
	startPwmSignal(&handlerTIM3PWM_1);

	/* Salidas de las frecuencias de los relojes por comando */
	handlerPinFrecuency.pGPIOx 									= GPIOA;
	handlerPinFrecuency.GPIO_PinConfig.GPIO_PinNumber 			= PIN_8;
	handlerPinFrecuency.GPIO_PinConfig.GPIO_PinMode 			= GPIO_MODE_ALTFN;
	handlerPinFrecuency.GPIO_PinConfig.GPIO_PinAltFunMode 		= AF0;
	GPIO_Config(&handlerPinFrecuency);

	// Para el RTC.
	handlerRTC.RTC_Days = 1;
	handlerRTC.RTC_Hours = 10;
	handlerRTC.RTC_Minutes = 30;
	handlerRTC.RTC_Seconds = 10;
	// Se carga la configuración sobre el RTC.
	rtc_Config(&handlerRTC);

	//Configuración I2C
	// Para el acelerómetro ADXL-345
	SCL.pGPIOx                                    = GPIOB;
	SCL.GPIO_PinConfig.GPIO_PinNumber             = PIN_8;
	SCL.GPIO_PinConfig.GPIO_PinMode               = GPIO_MODE_ALTFN;
	SCL.GPIO_PinConfig.GPIO_PinOPType             = GPIO_OTYPE_OPENDRAIN;
	SCL.GPIO_PinConfig.GPIO_PinPuPdControl        = GPIO_PUPDR_NOTHING;
	SCL.GPIO_PinConfig.GPIO_PinSpeed              = GPIO_OSPEED_FAST;
	SCL.GPIO_PinConfig.GPIO_PinAltFunMode         = AF4;
	GPIO_Config(&SCL);
	// SDA pin del ADXL-345
	SDA.pGPIOx                                    = GPIOB;
	SDA.GPIO_PinConfig.GPIO_PinNumber             = PIN_9;
	SDA.GPIO_PinConfig.GPIO_PinMode               = GPIO_MODE_ALTFN;
	SDA.GPIO_PinConfig.GPIO_PinOPType             = GPIO_OTYPE_OPENDRAIN;
	SDA.GPIO_PinConfig.GPIO_PinPuPdControl        = GPIO_PUPDR_NOTHING;
	SDA.GPIO_PinConfig.GPIO_PinSpeed              = GPIO_OSPEED_FAST;
	SDA.GPIO_PinConfig.GPIO_PinAltFunMode         = AF4;
	GPIO_Config(&SDA);
	// Se carga en el I2C.
	Acelerometer.ptrI2Cx                            = I2C1;
	Acelerometer.modeI2C                            = I2C_MODE_FM;
	Acelerometer.slaveAddress                       = ACCEL_ADDRESS;
	i2c_config(&Acelerometer);


} // Fin initHardware

void parseCommands(char *ptrBufferReception){

	/* Esta funcion de C lee la cadena de caracteres a la que apunta el "ptr" y la divide
	 * y almacena en tres elementos diferentes : un string llamado "cmd", y dos numeros
	 * integer llamados "firstParameter" y "secondParameter".
	 * De esta forma, podemos introducir información al micro desde el puerto serial.
	 */
	/*
	 * Aquí tenemos básicamente la lista de comandos que se pide debe contener el código
	 * para interactuar con el usuario. Se busca hacerlo lo más fácil de digerir que se
	 * pueda para la persona que necesita operar con el sistema. Inicialmente se debe tener
	 * en cuenta la funcion que cumple coolterm para enviar strings de acuerdo a la función
	 * que necesita que realice el sistema. Debe informarse al usuario que la recomendacion
	 * inicial es que use el comando "help" que despliega un menu de ayuda que le dice cuales
	 * son los comandos y la forma de introducirlos de acuerdo con la función que se requiere.
	 */
	sscanf(ptrBufferReception,"%s %u %u %u %s",cmd,&firstParameter,&secondParameter,&thirdparameter,userMsg);
	//Este primer comando imprime una lista con los otros comandos que tiene el equipo
	if (strcmp(cmd, "help") == 0){
		writeMsg(&USART6Comm, "QUE ESTE MENU DE AYUDA TE ACOMPANE\n");
		writeMsg(&USART6Comm, "⠀⢀⣠⣄⣀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣴⣶⡾⠿⠿⠿⠿⢷⣶⣦⣤⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⢰⣿⡟⠛⠛⠛⠻⠿⠿⢿⣶⣶⣦⣤⣤⣀⣀⡀⣀⣴⣾⡿⠟⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠻⢿⣷⣦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⣀⣀⣀⣀⣀⡀\n");
		writeMsg(&USART6Comm, "⠀⠻⣿⣦⡀⠀⠉⠓⠶⢦⣄⣀⠉⠉⠛⠛⠻⠿⠟⠋⠁⠀⠀⠀⣤⡀⠀⠀⢠⠀⠀⠀⣠⠀⠀⠀⠀⠈⠙⠻⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠟⠛⠛⢻⣿\n");
		writeMsg(&USART6Comm, "⠀⠀⠈⠻⣿⣦⠀⠀⠀⠀⠈⠙⠻⢷⣶⣤⡀⠀⠀⠀⠀⢀⣀⡀⠀⠙⢷⡀⠸⡇⠀⣰⠇⠀⢀⣀⣀⠀⠀⠀⠀⠀⠀⣀⣠⣤⣤⣶⡶⠶⠶⠒⠂⠀⠀⣠⣾⠟\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠈⢿⣷⡀⠀⠀⠀⠀⠀⠀⠈⢻⣿⡄⣠⣴⣿⣯⣭⣽⣷⣆⠀⠁⠀⠀⠀⠀⢠⣾⣿⣿⣿⣿⣦⡀⠀⣠⣾⠟⠋⠁⠀⠀⠀⠀⠀⠀⠀⣠⣾⡟⠁⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠈⢻⣷⣄⠀⠀⠀⠀⠀⠀⠀⣿⡗⢻⣿⣧⣽⣿⣿⣿⣧⠀⠀⣀⣀⠀⢠⣿⣧⣼⣿⣿⣿⣿⠗⠰⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⡿⠋⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠙⢿⣶⣄⡀⠀⠀⠀⠀⠸⠃⠈⠻⣿⣿⣿⣿⣿⡿⠃⠾⣥⡬⠗⠸⣿⣿⣿⣿⣿⡿⠛⠀⢀⡟⠀⠀⠀⠀⠀⠀⣀⣠⣾⡿⠋⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⠿⣷⣶⣤⣤⣄⣰⣄⠀⠀⠉⠉⠉⠁⠀⢀⣀⣠⣄⣀⡀⠀⠉⠉⠉⠀⠀⢀⣠⣾⣥⣤⣤⣤⣶⣶⡿⠿⠛⠉⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⢻⣿⠛⢿⣷⣦⣤⣴⣶⣶⣦⣤⣤⣤⣤⣬⣥⡴⠶⠾⠿⠿⠿⠿⠛⢛⣿⣿⣿⣯⡉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣧⡀⠈⠉⠀⠈⠁⣾⠛⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣴⣿⠟⠉⣹⣿⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣸⣿⣿⣦⣀⠀⠀⠀⢻⡀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣤⣶⣿⠋⣿⠛⠃⠀⣈⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡿⢿⡀⠈⢹⡿⠶⣶⣼⡇⠀⢀⣀⣀⣤⣴⣾⠟⠋⣡⣿⡟⠀⢻⣶⠶⣿⣿⠛⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣿⣷⡈⢿⣦⣸⠇⢀⡿⠿⠿⡿⠿⠿⣿⠛⠋⠁⠀⣴⠟⣿⣧⡀⠈⢁⣰⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⢻⣦⣈⣽⣀⣾⠃⠀⢸⡇⠀⢸⡇⠀⢀⣠⡾⠋⢰⣿⣿⣿⣿⡿⠟⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠿⢿⣿⣿⡟⠛⠃⠀⠀⣾⠀⠀⢸⡇⠐⠿⠋⠀⠀⣿⢻⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⠁⢀⡴⠋⠀⣿⠀⠀⢸⠇⠀⠀⠀⠀⠀⠁⢸⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣿⡿⠟⠋⠀⠀⠀⣿⠀⠀⣸⠀⠀⠀⠀⠀⠀⠀⢸⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣁⣀⠀⠀⠀⠀⣿⡀⠀⣿⠀⠀⠀⠀⠀⠀⢀⣈⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠛⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠿⠟⠛⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n");
		writeMsg(&USART6Comm, "Help Menu CMDs:\n");
		writeMsg(&USART6Comm, "Por favor ingrese el esquema |comando| (espacio) los datos requeridos en cada caso\n");
		writeMsg(&USART6Comm, "Los siguientes comandos indican las diferentes funciones del sistema:\n");
		writeMsg(&USART6Comm, "1)help                     Este comando despliega el menu de ayuda\n");
		writeMsg(&USART6Comm, "2)select_Clock_Signal***** Presione 1 para señal PLL, 2 para LSE y 3 para HSI\n");
		writeMsg(&USART6Comm, "************************** Comando + entero\n");
		writeMsg(&USART6Comm, "3)select_Prescaler******** Indique valores enteros entre 1 y 5 para el prescaler\n");
		writeMsg(&USART6Comm, "************************** Comando + entero\n");
		writeMsg(&USART6Comm, "4)set_Time**************** Usted configura su hora inicial: Horas: minutos: segundos\n");
		writeMsg(&USART6Comm, "************************** Comando + entero + entero + entero\n");
		writeMsg(&USART6Comm, "5)current_time************ El sistema retorna la hora actual: Horas: minutos: segundos\n");
		writeMsg(&USART6Comm, "************************** Comando + entero + entero + entero\n");
		writeMsg(&USART6Comm, "6)configFecha************* Usted configura su fecha inicial: Dias: meses: año\n");
		writeMsg(&USART6Comm, "************************** Comando + entero + entero + entero\n");
		writeMsg(&USART6Comm, "7)Fecha******************* El sistema retorna la fecha actual: Dias: meses: año\n");
		writeMsg(&USART6Comm, "************************** Comando + entero + entero + entero\n");
		writeMsg(&USART6Comm, "8)init_ADC**************** Se inicializa y muestra 256 datos de 2 canales de ADC\n");
		writeMsg(&USART6Comm, "************************** Solo comando\n");
		writeMsg(&USART6Comm, "9)signal_Sampling********* Se ingresa el periodo de muestreo de la señal de PWM en microsegundos\n");
		writeMsg(&USART6Comm, "************************** Comando + entero\n");
		writeMsg(&USART6Comm, "10)muestreo*************** Se inicializa la toma de datos del acelerometro mostrando 256 datos\n");
		writeMsg(&USART6Comm, "************************** Comando\n");
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
		writeMsg(&USART6Comm, "ESPERO HABERTE AYUDADO\n");

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
		}else{
			writeMsg(&USART6Comm, "Invalid comand, please read the help menu\n");
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
		}else{
			writeMsg(&USART6Comm, "Invalid comand, please read the help menu\n");
		}
	}else if(strcmp(cmd,"set_Time") == 0){
		if(firstParameter > 23){
			firstParameter = 23;
		}
		if(secondParameter > 59){
			secondParameter = 59;
		}
		if(thirdparameter > 59){
			thirdparameter = 59;
		}
		handlerRTC.RTC_Hours = firstParameter;
		handlerRTC.RTC_Minutes = secondParameter;
		handlerRTC.RTC_Seconds = thirdparameter;
		rtc_Config(&handlerRTC);
		sprintf(buffer,"Hora %02u : %02u : %02u \n",firstParameter,secondParameter,thirdparameter);
		writeMsg(&USART6Comm, buffer);
	}else if(strcmp(cmd,"current_time") == 0){
		ptrTime = read_Time();
		segundos = ptrTime[0];
		minutos = ptrTime[1];
		horas	= ptrTime[2];
		sprintf(buffer,"Hora Actual %02u : %02u : %02u \n",horas,minutos,segundos);
		writeMsg(&USART6Comm, buffer);
	}else if(strcmp(cmd,"configFecha") == 0){
		if(firstParameter > 31){
			firstParameter = 31;
		}
		if(secondParameter > 12){
			secondParameter = 12;
		}
		if(thirdparameter < 2000 || thirdparameter > 2099){
			thirdparameter = 2099;
		}
		handlerRTC.RTC_Days   = firstParameter;
		handlerRTC.RTC_Months = secondParameter;
		handlerRTC.RTC_Years  = thirdparameter;
		rtc_Config(&handlerRTC);
		sprintf(buffer,"Date %02u : %02u : %02u \n", firstParameter,secondParameter,thirdparameter);
		writeMsg(&USART6Comm,buffer);

	}else if(strcmp(cmd,"Fecha") == 0){
		ptrDate = read_Date();
		dia  = ptrDate[0];
		mes  = ptrDate[1];
		año  = ptrDate[2];
		sprintf(buffer,"Fecha actual %02u : %02u : %02u \n",dia ,mes,año);
		writeMsg(&USART6Comm,buffer);
	}else if(strcmp(cmd,"init_ADC") == 0){
		if(adcIsComplete == 1){
			for(uint16_t j = 0; j < 256; j++){
				sprintf(buffer, "[#%d]: %.2f, %.2f\n",j, dataADCChannel0[j]*3.3f/4095.f, dataADCChannel1[j]*3.3f/4095.f);
				writeMsg(&USART6Comm, buffer);
			}
			adcIsComplete = 0;
		}
	}else if(strcmp(cmd,"signal_Sampling") == 0){
		if((33 <= firstParameter) && (firstParameter <= 62)){
			// Se actualiza la nueva frecuencia con un duttycicle igual a la mitad de la nueva frecuencia
			updateFrequency(&handlerTIM3PWM_1, firstParameter);
			updateDuttyCycle(&handlerTIM3PWM_1, firstParameter/2);
			writeMsg(&USART6Comm, "El periodo de la señal de muestreo fue actualizado en microsegundos\n");
			writeMsg(&USART6Comm, "Recuerde que los datos de la conversion se dan en voltios\n");
		}else{
			writeMsg(&USART6Comm, "Invalid signal_Sampling\n");
		}
	}else if(strcmp(cmd,"muestreo") == 0){
		writeMsg(&USART6Comm, "Tomando muestreo.....\n");
		flag = 1;
		i2c_writeSingleRegister(&Acelerometer, POWER_CTL,45);
		while(counter < 1024){
			if(flag2){
				uint8_t AccelX_low =  i2c_readSingleRegister(&Acelerometer, ACCEL_X1_L);
				uint8_t AccelX_high = i2c_readSingleRegister(&Acelerometer, ACCEL_X1_H);
				int16_t AccelX = AccelX_high << 8 | AccelX_low;
				X_axis = AccelX * 0.0039 * 9.8;

				uint8_t AccelY_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Y1_L);
				uint8_t AccelY_high = i2c_readSingleRegister(&Acelerometer,ACCEL_Y1_H);
				int16_t AccelY = AccelY_high << 8 | AccelY_low;
				Y_axis = AccelY * 0.0039 * 9.8;

				uint8_t AccelZ_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_L);
				uint8_t AccelZ_high = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_H);
				int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
				Z_axis = AccelZ * 0.0039 * 9.8;
				X_1[counter] = X_axis;
			    Y_1[counter] = Y_axis;
				Z_1[counter] = Z_axis;
				counter++;
				flag2 = 0;
				if(counter == 1024){
					break;
				}
			}
		}
		for( int i = 0 ; i < 1024; i++){
			sprintf(bufferData, " AccelX = %.2f ; AccelY = %.2f ; AccelZ = %.2f \n",X_1[i],Y_1[i],Z_1[i]);
			writeMsg(&USART6Comm, bufferData);
		}
		writeMsg(&USART6Comm, "Datos tomados correctamente \n");

	}else{
		writeMsg(&USART6Comm, "invalid comand, please check the help menu\n");
	}
}

/* Timer que gobierna el blinky del led de estado */
void BasicTimer2_Callback(void){
	GPIOxTooglePin (&handlerStateLED);
}
// Banderas para el muestreo del ADXL345.
void BasicTimer4_Callback(void){
if(flag){
	flag2 = 1;
}
}

// Callback para los comandos del USART.
void usart6Rx_Callback(void){
	rxData = getRxData();
}

// Esta función controla el contador del ADC.(Hace las veces de callback)
void adcComplete_Callback(void){
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
