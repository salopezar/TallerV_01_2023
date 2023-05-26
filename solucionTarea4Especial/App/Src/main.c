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
 * los que se ha trabajado en clase. También se introduce el funcionamiento de los
 * display de cristal líquido (LCD).
 *
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
#include "math.h"
#include "float.h"

// Se nombra el PLL que cambia la frecuencia de operación del micro.
PLL_Handler_t handlerPLL = {0};

// Definicion de los handlers necesarios.
// Para el Blinky
GPIO_Handler_t handlerBlinkyPin 	= {0};

// Para la prueba del USART (Punto 4).
GPIO_Handler_t handlerUSARTPINTX = {0};
GPIO_Handler_t handlerUSARTPINRX = {0};
USART_Handler_t USART6Comm = {0};
// Variables necesarias para la comprobación.
uint8_t sendMSG = 0;
uint8_t newChar = 0;
char mensaje[64] = "TALLERV_TAREA_ESPECIAL\n";

/* Configración general para el acelerómetro ADXL-345 */
GPIO_Handler_t SDA = {0};
GPIO_Handler_t SCL = {0};
I2C_Handler_t Acelerometer = {0};

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

#define POWER_CTL                45
#define WHO_AM_I                 0


// Timer del led de estado.
BasicTimer_Handler_t handlerBlinkyTimer = {0};


void init_Hardware(void);


int main(void){
	// Se llama la función de inicialización.
	init_Hardware();

	while(1){
		//Hacemos un "eco" con el valor que nos llega por el serial
		if(rxData != '\0'){
			writeChar(&USART6Comm, rxData);

			if(rxData == 'w'){
				sprintf(bufferData, "WHO_AM_I? (r)\n");
				writeMsg(&USART6Comm, bufferData);

				i2cBuffer = i2c_readSingleRegister(&Acelerometer, WHO_AM_I);
				sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
				writeMsg(&USART6Comm, bufferData);
				rxData = '\0';
			}
			else if (rxData == 'p'){
				sprintf(bufferData, "PWR_MGMT_1 state (r)\n");
				writeMsg(&USART6Comm, bufferData);

				i2cBuffer = i2c_readSingleRegister(&Acelerometer, POWER_CTL);
				sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
				writeMsg(&USART6Comm, bufferData);
				rxData = '\0';
			}
			else if (rxData == 'r'){
				sprintf(bufferData, "PWR_MGMT_1 reset (w)\n");
				writeMsg(&USART6Comm, bufferData);

				i2c_writeSingleRegister(&Acelerometer, POWER_CTL , 0x2D); // modo medida en acelerometro
				rxData = '\0';
			}
			else if (rxData == 'x'){
				sprintf(bufferData, "Axis X data (r) \n");
				writeMsg(&USART6Comm, bufferData);

				uint8_t AccelX_low =  i2c_readSingleRegister(&Acelerometer, ACCEL_X1_L);
				uint8_t AccelX_high = i2c_readSingleRegister(&Acelerometer, ACCEL_X1_H);
				int16_t AccelX = AccelX_high << 8 | AccelX_low;
				float X_axis = AccelX * 0.0039 * 9.8;
				sprintf(bufferData, "AccelX = %.2f \n", X_axis);
				writeMsg(&USART6Comm, bufferData);
				rxData = '\0';
			}
			else if(rxData == 'y'){
				sprintf(bufferData, "Axis Y data (r)\n");
				writeMsg(&USART6Comm, bufferData);
				uint8_t AccelY_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Y1_L);
				uint8_t AccelY_high = i2c_readSingleRegister(&Acelerometer,ACCEL_Y1_H);
				int16_t AccelY = AccelY_high << 8 | AccelY_low;
				float Y_axis = AccelY * 0.0039 * 9.8;
				sprintf(bufferData, "AccelY = %.2f \n", Y_axis);
				writeMsg(&USART6Comm, bufferData);
				rxData = '\0';
			}
			else if(rxData == 'z'){
				sprintf(bufferData, "Axis Z data (r)\n");
				writeMsg(&USART6Comm, bufferData);

				uint8_t AccelZ_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_L);
				uint8_t AccelZ_high = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_H);
				int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
				float Z_axis = AccelZ * 0.0039 * 9.8;
				sprintf(bufferData, "AccelZ = %.2f \n", Z_axis);
				writeMsg(&USART6Comm, bufferData);
				rxData = '\0';
			}
			else{
				rxData = '\0';
			}
		}

	}
	return 0;
}

//Función de configuración de los elementos del sistema.
void init_Hardware(void){

	// Se configura el PLL con los parámetros dados.
	handlerPLL.PLL_Config.PLL_voltage		= VOLTAGE_84MHZ;
	handlerPLL.PLL_Config.PLL_frecuency		= FRECUENCY_80MHZ;
	PLL_Config(&handlerPLL);
	getConfigPLL();

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
	handlerBlinkyTimer.TIMx_Config.TIMx_period				= 2500;
	handlerBlinkyTimer.TIMx_Config.TIMx_interruptEnable 	= BTIMER_INTERRUPT_ENABLE;

	// Se carga lo hecho sobre el timer del blinky.
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

	//Configuración I2C
	SCL.pGPIOx                                    = GPIOB;
	SCL.GPIO_PinConfig.GPIO_PinNumber             = PIN_8;
	SCL.GPIO_PinConfig.GPIO_PinMode               = GPIO_MODE_ALTFN;
	SCL.GPIO_PinConfig.GPIO_PinOPType             = GPIO_OTYPE_OPENDRAIN;
	SCL.GPIO_PinConfig.GPIO_PinPuPdControl        = GPIO_PUPDR_NOTHING;
	SCL.GPIO_PinConfig.GPIO_PinSpeed              = GPIO_OSPEED_FAST;
	SCL.GPIO_PinConfig.GPIO_PinAltFunMode         = AF4;
	GPIO_Config(&SCL);

	SDA.pGPIOx                                    = GPIOB;
	SDA.GPIO_PinConfig.GPIO_PinNumber             = PIN_9;
	SDA.GPIO_PinConfig.GPIO_PinMode               = GPIO_MODE_ALTFN;
	SDA.GPIO_PinConfig.GPIO_PinOPType              = GPIO_OTYPE_OPENDRAIN;
	SDA.GPIO_PinConfig.GPIO_PinPuPdControl        = GPIO_PUPDR_NOTHING;
	SDA.GPIO_PinConfig.GPIO_PinSpeed              = GPIO_OSPEED_FAST;
	SDA.GPIO_PinConfig.GPIO_PinAltFunMode         = AF4;
	GPIO_Config(&SDA);

	Acelerometer.ptrI2Cx                            = I2C1;
	Acelerometer.modeI2C                            = I2C_MODE_FM;
	Acelerometer.slaveAddress                       = ACCEL_ADDRESS;
	i2c_config(&Acelerometer);

}

// Callback para el blinky pin.
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerBlinkyPin);
	sendMSG++;
}

void usart6Rx_Callback(void){
	//Leemo el valor del registro DR, donde se encuentra almacenado el dato que llega
	// ESto además debe bajar la bandera de la interrupción
	rxData = getRxData();
}