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
uint8_t flag = 0;
uint8_t flag2 = 0;
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
float X_1[2000] = {0};
float Y_1[2000] = {0};
float Z_1[2000] = {0};
uint16_t counter = {0};

// Timer del led de estado.
BasicTimer_Handler_t handlerBlinkyTimer = {0};

// Timer para el muestreo
BasicTimer_Handler_t handlerTimer5 = {0};
// La bandera para la interrupcion del muestreo
char flagMuestreo = {0};

/* CONFIGURACIÓN PARA LAS SEÑALES DE PWM */
GPIO_Handler_t HandlerPWM_1 = {0};
GPIO_Handler_t HandlerPWM_2 = {0};
GPIO_Handler_t HandlerPWM_3 = {0};
PWM_Handler_t handlerTIM3PWM_1 = {0};
PWM_Handler_t handlerTIM3PWM_2 = {0};
PWM_Handler_t handlerTIM3PWM_3 = {0};
uint16_t duttyValue = {0};
// Variables para los ejes del acelerómetro a 2 decimales
float X_axis = {0};
float Y_axis = {0};
float Z_axis = {0};
uint16_t newDutty_1 = {0};
uint16_t newDutty_2 = {0};
uint16_t newDutty_3 = {0};
// Función para las señales de pwm
void signalsPWM(void);

// La función de inicialización del sistema
void init_Hardware(void);


int main(void){
	// Se llama la función de inicialización.
	init_Hardware();

	while(1){

		/*
		 * Se manejan diferentes comandos para realizar diversas operaciones,
		 * como obtener la identificación del acelerómetro, verificar el estado
		 * de alimentación, restablecer la configuración y leer los datos de
		 * los ejes X, Y y Z. El código utiliza funciones de comunicación I2C
		 * para interactuar con el acelerómetro. Se entra en el ciclo cuando lo que
		 * se envía es diferente del caracter nulo,
		 */

		/* Este condicional siempre se ejecuta para tener el muestreo que controla
		 * las señales de salida de PWM con dutty variable.
		 */
		if(flagMuestreo == 1){
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

			signalsPWM();
			flagMuestreo = 0;

		}

		if(rxData != '\0'){
			//writeChar(&USART6Comm, rxData);
			// Lectura sobre el WHO_AM_I del acelerómetro.
			if(flagMuestreo == 1){
				if(rxData == 'w'){
					sprintf(bufferData, "WHO_AM_I? (r)\n");
					writeMsg(&USART6Comm, bufferData);
					// Configuración del I2C e impresión del mensaje,
					i2cBuffer = i2c_readSingleRegister(&Acelerometer, WHO_AM_I);
					sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
					writeMsg(&USART6Comm, bufferData);
					rxData = '\0';
				}
				// Estado del acelerómetro
				else if (rxData == 'p'){
					sprintf(bufferData, "PWR_MGMT_1 state (r)\n");
					writeMsg(&USART6Comm, bufferData);
					i2cBuffer = i2c_readSingleRegister(&Acelerometer, POWER_CTL);
					sprintf(bufferData, "dataRead = 0x%x \n", (unsigned int) i2cBuffer);
					writeMsg(&USART6Comm, bufferData);
					rxData = '\0';
				}
				// Reset para el acelerómetro.
				else if (rxData == 'r'){
					sprintf(bufferData, "PWR_MGMT_1 reset (w)\n");
					writeMsg(&USART6Comm, bufferData);
					// Configuración del reset dentro del I2C.
					i2c_writeSingleRegister(&Acelerometer, POWER_CTL , 0x2D);
					rxData = '\0';
				}
				// Para el eje X y su lectura presionando la tecla "x".
				else if (rxData == 'x'){
					sprintf(bufferData, "Axis X data (r) \n");
					writeMsg(&USART6Comm, bufferData);
					uint8_t AccelX_low =  i2c_readSingleRegister(&Acelerometer, ACCEL_X1_L);
					uint8_t AccelX_high = i2c_readSingleRegister(&Acelerometer, ACCEL_X1_H);
					int16_t AccelX = AccelX_high << 8 | AccelX_low;
					X_axis = AccelX * 0.0039 * 9.8;
					sprintf(bufferData, "AccelX = %.2f \n", X_axis);
					writeMsg(&USART6Comm, bufferData);
					rxData = '\0';
				}
				// Para el eje Y y su lectura presionando la tecla "y".
				else if(rxData == 'y'){
					sprintf(bufferData, "Axis Y data (r)\n");
					writeMsg(&USART6Comm, bufferData);
					uint8_t AccelY_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Y1_L);
					uint8_t AccelY_high = i2c_readSingleRegister(&Acelerometer,ACCEL_Y1_H);
					int16_t AccelY = AccelY_high << 8 | AccelY_low;
					Y_axis = AccelY * 0.0039 * 9.8;
					sprintf(bufferData, "AccelY = %.2f \n", Y_axis);
					writeMsg(&USART6Comm, bufferData);
					rxData = '\0';
				}
				// Para el eje Z y su lectura presionando la tecla "z".
				else if(rxData == 'z'){
					sprintf(bufferData, "Axis Z data (r)\n");
					writeMsg(&USART6Comm, bufferData);
					uint8_t AccelZ_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_L);
					uint8_t AccelZ_high = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_H);
					int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
					Z_axis = AccelZ * 0.0039 * 9.8;
					sprintf(bufferData, "AccelZ = %.2f \n", Z_axis);
					writeMsg(&USART6Comm, bufferData);
					rxData = '\0';
				}
				flagMuestreo = 0;
			}
			// Aquí se define el envío constante de datos.
			/* Se define sobre la macro del timer 5 un muestreo infinito a
			 * 1 kHz de los valores del acelerómetro ADXL-345 puesto sobre los
			 * 3 ejes coordenados. Si se quiere entrar en este modo, debe presionarse
			 * la tecla "c" desde la terminal serial. Se realiza la conversión
			 * establecida dentro del datasheet del ADXL-345 para que los valores
			 * estén dados en unidades del sistema internacional para aceleración.
			 * El condicional se define en función del segundo valor que
			 * puede tomar la bandera que se define en el Basic Callback.
			 * El sistema entra aquí al presionar la letra "c".
			 * Aquí se tiene un primer muestreo infinito a 1 kHz.
			 */
			if(flagMuestreo == 2){
				// Para el eje X
				uint8_t AccelX_low =  i2c_readSingleRegister(&Acelerometer, ACCEL_X1_L);
				uint8_t AccelX_high = i2c_readSingleRegister(&Acelerometer, ACCEL_X1_H);
				int16_t AccelX = AccelX_high << 8 | AccelX_low;
				X_axis = AccelX * 0.0039 * 9.8;
				X_1[counter] = X_axis; // Arreglo definido para X.
				// Para el eje Y
				uint8_t AccelY_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Y1_L);
				uint8_t AccelY_high = i2c_readSingleRegister(&Acelerometer,ACCEL_Y1_H);
				int16_t AccelY = AccelY_high << 8 | AccelY_low;
				Y_axis = AccelY * 0.0039 * 9.8;
				Y_1[counter] = Y_axis; // Arreglo definido para Y.
				// Para el eje Z
				uint8_t AccelZ_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_L);
				uint8_t AccelZ_high = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_H);
				int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
				Z_axis = AccelZ * 0.0039 * 9.8;
				Z_1[counter] = Z_axis; // Arreglo definido para Y.
				// El contador que barre los arreglos.
				counter++;
				// Las 2000 tomas para los 2000 segundos.
				if(counter == 2000){
					flag = 1; // Bandera para entrar al ciclo if y enviar valores de aceleración
					counter = 0;
				}
			}

			/* IMPRESIÓN DE LOS ARREGLOS :
			 * Aquí se define la forma en que se busca dar un formato definido para
			 * la presentación de los arreglos bajo el muestreo de 1 kHz.
			 */
				if(flag == 1){
					for( int i = 0 ; i < 2000; i++){
						sprintf(bufferData, " AccelX = %.2f ; AccelY = %.2f ; AccelZ = %.2f \n",X_1[i],Y_1[i],Z_1[i]);
						writeMsg(&USART6Comm, bufferData);
						flagMuestreo = 0;
				    }
				}

				/* Al presionar la letra k como se define al final del código
				 * dentro del Callback del timer, el sistema retorna los 6000
				 * datos que corresponden al muestreo de 1 kHz durante 2 segundos
				 * como se indica en el enunciado de la tarea. Se incluye dentro
				 * del formato de retorno de los datos un segundo contador que
				 * mostrará que sí se están retornando los 2000 datos (definiendo
				 * un dato como el grupo de valores de las 3 coordenadas) y no
				 * menos.
				 */
				if(flagMuestreo == 3){

					uint8_t AccelX_low =  i2c_readSingleRegister(&Acelerometer, ACCEL_X1_L);
					uint8_t AccelX_high = i2c_readSingleRegister(&Acelerometer, ACCEL_X1_H);
					int16_t AccelX = AccelX_high << 8 | AccelX_low;
					X_axis = AccelX * 0.0039 * 9.8;
					X_1[counter]=X_axis;

					uint8_t AccelY_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Y1_L);
					uint8_t AccelY_high = i2c_readSingleRegister(&Acelerometer,ACCEL_Y1_H);
					int16_t AccelY = AccelY_high << 8 | AccelY_low;
					Y_axis = AccelY * 0.0039 * 9.8;
					Y_1[counter]=Y_axis;

					uint8_t AccelZ_low = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_L);
					uint8_t AccelZ_high = i2c_readSingleRegister(&Acelerometer, ACCEL_Z1_H);
					int16_t AccelZ = AccelZ_high << 8 | AccelZ_low;
					Z_axis = AccelZ * 0.0039 * 9.8;
					Z_1[counter]=Z_axis;
					counter++;
					if(counter == 2000){
						flag2 = 1;
						rxData = '\0';
					}
				}
				/* Dentro de este if y gracias a la configuración de la velocidad
				 * a la cual se pone a trabajar el timer, se imprimen los datos
				 * recopilados dentro del arreglo para su fácil lectura y análisis.
				 */

				if(flag2 == 1){
					for( int i = 0 ; i < 2000; i++){
						sprintf(bufferData, "[m/s²] #%d AccelX = %.2f ; AccelY = %.2f ; AccelZ = %.2f \n",i,X_1[i],Y_1[i],Z_1[i]);
						writeMsg(&USART6Comm, bufferData);
						flagMuestreo =0;
						flag2 = 0;
					}

				}

	} // FIN DEL CONDICIONAL RXData !=0
  }// FIN DEL CICLO WHILE DEL MAIN.
	return (0);
}// FIN DEL MAIN

// Función para calcular el dutty cycle a partir de una regresión lineal.
/* Se toma un rango ideal para el acelerómetro tomando en cuenta las condiciones
 * en que se vió desde la experiencia que oscilan los valores. La línea recta
 * mantiene los valores de tiempo del dutty cicle dentro de los rangos en que
 * solicita mantenerlos según el texto de la tarea.
 */

void signalsPWM(void){
	// Para cada eje debemos conservar la misma recta porque las condiciones so
	newDutty_1 = 1000 * X_axis + 10000;
	newDutty_2 = 1000 * Y_axis + 10000;
	newDutty_3 = 1000 * Z_axis + 10000;
	// Se actualiza la configuración del PWM bajo la nueva condición.
	updateDuttyCycle(&handlerTIM3PWM_1, newDutty_1);
	updateDuttyCycle(&handlerTIM3PWM_2, newDutty_2);
	updateDuttyCycle(&handlerTIM3PWM_3, newDutty_3);
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

	// Definición del timer para el muestreo de 1 KHz
	handlerTimer5.ptrTIMx							= TIM5;
	handlerTimer5.TIMx_Config.TIMx_mode				= BTIMER_MODE_UP;
	handlerTimer5.TIMx_Config.TIMx_speed			= BTIMER_SPEED_80MHz;
	handlerTimer5.TIMx_Config.TIMx_period			= 10;
	handlerTimer5.TIMx_Config.TIMx_interruptEnable 	= BTIMER_INTERRUPT_ENABLE;

	// Se carga lo hecho sobre el timer del muestreo.
	BasicTimer_Config(&handlerTimer5);

	// Configuración para el PWM
	// Como son 3 señales, se tiene PWM_1, PWM_2 Y PWM_3.
	// Se multiplexa el timer 3, dado que cada uno tiene 4 canales de PWM.
	HandlerPWM_1.pGPIOx          					= GPIOC;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinNumber  	= PIN_7;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinMode    	= GPIO_MODE_ALTFN;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinOPType  	= GPIO_OTYPE_PUSHPULL;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_OSPEED_FAST;
	HandlerPWM_1.GPIO_PinConfig.GPIO_PinAltFunMode  = AF2;

	GPIO_Config(&HandlerPWM_1);

	handlerTIM3PWM_1.ptrTIMx           	  =   TIM3;
	handlerTIM3PWM_1.config.channel       =   PWM_CHANNEL_2;
	handlerTIM3PWM_1.config.duttyCicle    =   1500;
	handlerTIM3PWM_1.config.periodo       =   20000;
	handlerTIM3PWM_1.config.prescaler     =   80;

	pwm_Config(&handlerTIM3PWM_1);

	enableOutput(&handlerTIM3PWM_1);
	startPwmSignal(&handlerTIM3PWM_1);

	HandlerPWM_2.pGPIOx          					= GPIOC;
	HandlerPWM_2.GPIO_PinConfig.GPIO_PinNumber  	= PIN_8;
	HandlerPWM_2.GPIO_PinConfig.GPIO_PinMode    	= GPIO_MODE_ALTFN;
	HandlerPWM_2.GPIO_PinConfig.GPIO_PinOPType  	= GPIO_OTYPE_PUSHPULL;
	HandlerPWM_2.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	HandlerPWM_2.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_OSPEED_FAST;
	HandlerPWM_2.GPIO_PinConfig.GPIO_PinAltFunMode  = AF2;

	GPIO_Config(&HandlerPWM_2);

	handlerTIM3PWM_2.ptrTIMx           	  =   TIM3;
	handlerTIM3PWM_2.config.channel       =   PWM_CHANNEL_3;
	handlerTIM3PWM_2.config.duttyCicle    =   1500;
	handlerTIM3PWM_2.config.periodo       =   20000;
	handlerTIM3PWM_2.config.prescaler     =   80;

	pwm_Config(&handlerTIM3PWM_2);

	enableOutput(&handlerTIM3PWM_2);
	startPwmSignal(&handlerTIM3PWM_2);

	HandlerPWM_3.pGPIOx          					= GPIOC;
	HandlerPWM_3.GPIO_PinConfig.GPIO_PinNumber  	= PIN_9;
	HandlerPWM_3.GPIO_PinConfig.GPIO_PinMode    	= GPIO_MODE_ALTFN;
	HandlerPWM_3.GPIO_PinConfig.GPIO_PinOPType  	= GPIO_OTYPE_PUSHPULL;
	HandlerPWM_3.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	HandlerPWM_3.GPIO_PinConfig.GPIO_PinSpeed       = GPIO_OSPEED_FAST;
	HandlerPWM_3.GPIO_PinConfig.GPIO_PinAltFunMode  = AF2;

	GPIO_Config(&HandlerPWM_3);

	handlerTIM3PWM_3.ptrTIMx           	  =   TIM3;
	handlerTIM3PWM_3.config.channel       =   PWM_CHANNEL_4;
	handlerTIM3PWM_3.config.duttyCicle    =   1500;
	handlerTIM3PWM_3.config.periodo       =   20000;
	handlerTIM3PWM_3.config.prescaler     =   80;

	pwm_Config(&handlerTIM3PWM_3);

	enableOutput(&handlerTIM3PWM_3);
	startPwmSignal(&handlerTIM3PWM_3);
}


// Callback para el blinky pin.
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerBlinkyPin);
	sendMSG++;
}

// Callback para la bandera del muestreo de 1 KHz
void BasicTimer5_Callback(void){
	if(rxData == 'c'){
		flagMuestreo = 2;
	}else if(rxData == 'k'){
		flagMuestreo = 3;
	}else if(rxData == 'v'){
		flagMuestreo = 4;
	}else{
		flagMuestreo = 1;
	}
}
// Callback para el usart 6.
void usart6Rx_Callback(void){
	rxData = getRxData();
}
