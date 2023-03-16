/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Santiago López Aranzazu - CC. 1007429871
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

/* ************* COMENTARIO GENERAL PARA INTRODUCIR EL CÓDIGO BASE ****************
 *
 * Con este programa se desea mostrar el uso básico de los registros que controlan
 * al Micro (SFR) y la forma adecuada para utilizar los operadores &, |, ~ y =, para
 * cambiar la configuración de algún registro.
 * También es importante para entender la utilidad de los números BIN y HEX.
 *
 * HAL-> Hardware Abstraction Layer.
 *
 * Este programa introduce el periférico más simple que tiene el microcontrolador, que es
 * el encargado de manejar los pines de cada puerto.
 *
 * Cada PINx de cada puerto GPIO puede desarrollar funciones básicas de tipo entrada
 * y salida de datos digitales (1 ó 0), además se les puede asignar funciones especiales
 * que generalmente están ligadas a otro periférico adicional (se verá más adelante).
 *
 * De igual forma, varias caaracterísticas pueden ser configuradas para cada PINx
 * específico, como son:
 *
 * -Velocidad de respuesta.
 * -Tipo de entrada.
 * -Tipo de salida.
 * -Entrada análoga.
 *
 * Debemos definir entonces todos los registros que manejan el periférico GPIOx y luego
 * crear algunas funciones para utilizar adecuadamente el equipo.
 *
 * ************************** TAREA #2 - TALLER V - 2023_01 **************************************
 *
 * El presente programa es una compilación de las configuraciones básicas que se deben tener
 * en cuenta para manipular los registros que utiliza el periférico GPIO (que es el originalmente
 * más simple del microcontrolador) para manejar los pines de cada puerto, de allí su nombre que
 * indica que es de "propósito general".
 * Los códigos base de las cabeceras (.h) y los archivos (.c) fueron tomados de las indicaciones
 * de clase del curso de taller V.
 * La metodología escogida por quien desarrolla el presente consiste en desglosar cada uno de los
 * (3) numerales con sus respectivos subindices uno a uno para conseguir una mejor claridad en la
 * explicación del procedimiento que se considera adecuado para dar solución en cada ítem, por
 * tanto, señor usuario, por favor considere los siguientes bloques:
 *
 * *************************************** PUNTO 1 ***********************************************
 *
 * En la línea 161 del archivo correspondiente al GPIOxDriver.c se declara una función GPIO_ReadPin()
 * de 32 Bits que originalmente está diseñada para leer el estado de un PIN específico y recibe
 * parámetros del tipo GPIO_Handler_t.
 * En el enunciado de la tarea se plantea que esta función tiene un error que no permite obtener
 * el valor real que se debería leer de un PINx. Inicialmente la definición de esta función estaba
 * dada de la siguiente manera:
 *
 * uint32_t GPIO_ReadPin(GPIO_Handler_t *pPinHandler){
 *
 *	Se crea una variable auxiliar de 32 bits que después se busca retornar.
 *	uint32_t pinValue = 0;
 *
 *	Luego, la variable pinValue sirve para almacenar la lectura del input data register (IDR)
 *	para luego desplazar el registro mediante una operación bitwise, haciendo un shift a la
 *	derecha (>>) tantas veces como lo indique el PinNumber.
 *	pinValue = (pPinHandler->pGPIOx->IDR >> pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
 *
 *	return pinValue;
 *}
 *
 *El problema que se detecta con los anteriores comandos de código es que al desplazar este registro
 *a la derecha, no se están limpiando los demás bits que no corresponden al bit menos significativo que
 *es el que realmente se quiere leer mediante la función ya definida anteriormente. Además, debe tenerse
 *en cuenta que dentro del input data register se tienen 16 Bits que están inicialmente reservados, por
 *tanto no puede observarse la información existente en ellos.
 *
 *
 */

//Libreria para los tipos de variables
#include <stdint.h>

#include <stm32f411xx_hal.h>
#include <GPIOxDriver.h>

uint8_t counter = 0;


/*Función principal del programa. Es acá donde se ejecuta todo */
int main(void){

	//Definimos el Handler para cada uno de los pines que se requiere utilizar.
	GPIO_Handler_t handlerUserLedPin 	= {0};
	GPIO_Handler_t handlerPC9 	 		= {0};
	GPIO_Handler_t handlerPC6 			= {0};
	GPIO_Handler_t handlerPB8 			= {0};
	GPIO_Handler_t handlerPA6 			= {0};
	GPIO_Handler_t handlerPC7			= {0};
	GPIO_Handler_t handlerPC8 			= {0};
	GPIO_Handler_t handlerPA7 			= {0};
	GPIO_Handler_t handlerUserButton	= {0};

	//Se decide trabajar con el puerto GPIOA
	handlerUserLedPin.pGPIOx 								= GPIOA;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;                //Ninguna función


	handlerPC9.pGPIOx 										= GPIOC;
	handlerPC9.GPIO_PinConfig.GPIO_PinNumber				= PIN_9;
	handlerPC9.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC9.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC9.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC9.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC9.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	handlerPC6.pGPIOx 										= GPIOC;
	handlerPC6.GPIO_PinConfig.GPIO_PinNumber				= PIN_6;
	handlerPC6.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC6.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC6.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC6.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC6.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	handlerPB8.pGPIOx 										= GPIOB;
	handlerPB8.GPIO_PinConfig.GPIO_PinNumber				= PIN_8;
	handlerPB8.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPB8.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPB8.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPB8.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPB8.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	handlerPA6.pGPIOx 										= GPIOA;
	handlerPA6.GPIO_PinConfig.GPIO_PinNumber				= PIN_6;
	handlerPA6.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPA6.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPA6.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPA6.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPA6.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	handlerPC7.pGPIOx 										= GPIOC;
	handlerPC7.GPIO_PinConfig.GPIO_PinNumber				= PIN_7;
	handlerPC7.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC7.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC7.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC7.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC7.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	handlerPC8.pGPIOx 										= GPIOC;
	handlerPC8.GPIO_PinConfig.GPIO_PinNumber				= PIN_8;
	handlerPC8.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC8.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC8.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC8.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC8.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	handlerPA7.pGPIOx 										= GPIOA;
	handlerPA7.GPIO_PinConfig.GPIO_PinNumber				= PIN_7;
	handlerPA7.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPA7.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPA7.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPA7.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPA7.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	//Configuracion del botón azul
	handlerUserButton.pGPIOx 								= GPIOC;
	handlerUserButton.GPIO_PinConfig.GPIO_PinNumber			= PIN_13;
	handlerUserButton.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerUserButton.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuración de cada uno de los pines
	GPIO_Config(&handlerUserLedPin);
	GPIO_Config(&handlerPC9);
	GPIO_Config(&handlerPC6);
	GPIO_Config(&handlerPB8);
	GPIO_Config(&handlerPA6);
	GPIO_Config(&handlerPC7);
	GPIO_Config(&handlerPC8);
	GPIO_Config(&handlerPA7);
	GPIO_Config(&handlerUserButton);

	//Hacemos que el PIN_A5 quede encendido
	GPIO_WritePin(&handlerUserLedPin, SET);

	//Este es el ciclo principal, donde se ejecuta todo el programa
	while(1){
		//Se define el delay de 1s
		for (uint32_t i = 0; i < 1400000; i++);

		//Ver si el botón está presionado o no
		if (GPIO_ReadPin(&handlerUserButton) == SET){

			if (counter > 60){
				counter =1;

			}

			GPIO_WritePin(&handlerPA7, (0b1 << 0) & counter);
			GPIO_WritePin(&handlerPC8, ((0b1 << 1) & counter) >> 1);
			GPIO_WritePin(&handlerPC7, ((0b1 << 2) & counter) >> 2);
			GPIO_WritePin(&handlerPA6, ((0b1 << 3) & counter) >> 3);
			GPIO_WritePin(&handlerPB8, ((0b1 << 4) & counter) >> 4);
			GPIO_WritePin(&handlerPC6, ((0b1 << 5) & counter) >> 5);
			GPIO_WritePin(&handlerPC9, ((0b1 << 6) & counter) >> 6);

			counter++;

		}else{
			if (GPIO_ReadPin(&handlerUserButton) == RESET){

				if (counter < 1){
					counter = 60;

				}

			GPIO_WritePin(&handlerPA7, (0b1 << 0) & counter);
			GPIO_WritePin(&handlerPC8, ((0b1 << 1) & counter) >> 1);
			GPIO_WritePin(&handlerPC7, ((0b1 << 2) & counter) >> 2);
			GPIO_WritePin(&handlerPA6, ((0b1 << 3) & counter) >> 3);
			GPIO_WritePin(&handlerPB8, ((0b1 << 4) & counter) >> 4);
			GPIO_WritePin(&handlerPC6, ((0b1 << 5) & counter) >> 5);
			GPIO_WritePin(&handlerPC9, ((0b1 << 6) & counter) >> 6);

			counter--;

		}
			NOP();
		}

}
}
