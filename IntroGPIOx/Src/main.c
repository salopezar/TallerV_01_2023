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
/*Con este programa se desea mostrar el uso básico de los registros que controlan
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
 * -Velocidad de respuesta.
 * -Tipo de entrada.
 * -Tipo de salida.
 * -Entrada análoga.
 *
 * Debemos definir entonces todos los registros que manejan el periférico GPIOx y luego
 * crear algunas funciones para utilizar adecuadamente el equipo.
 *
 * **************************************************************************************
 */
//Libreria para los tipos de variables
#include <stdint.h>

#include <stm32f411xx_hal.h>
#include <GPIOxDriver.h>

/*Función principal del programa. Es acá donde se ejecuta todo */
int main(void){

	//Definimos el Handler para el PIN que deseamos configurar
	GPIO_Handler_t handleUserLedPin = {0};

	//Se decide trabajar con el puerto GPIOA
	handlerUserPin.pGPIOx = GPIOA;
	handlerUserPin.GPIO_PinConfig.GPIO_PinNumber			=PIN_5;
	handlerUserPin.GPIO_PinConfig.GPIO_PinMode				=GPIO_MODE_OUT;
	handlerUserPin.GPIO_PinConfig.GPIO_PinOPType			=GPIO_OTYPE_PUSHPULL;
	handlerUserPin.GPIO_PinConfig.GPIO_PinPuPdControl		=GPIO_PUPDR_NOTHING;
	handlerUserPin.GPIO_PinConfig.GPIO_PinSpeed				=GPIO_OSPEED_MEDIUM;
	handlerUserPin.GPIO_PinConfig.GPIO_PinAltFunMode		=AF0;                //Ninguna función


	//Cargamos la configuración del pin específico
	GPIO_Config(&handlerUserLedPin);

	//Hacemos que el PIN_A5 quede encendido
	GPIO_WritePin(&handlerUserLedPin, SET);

	//Este es el ciclo principal, donde se ejecuta todo el programa
	while(1){
		NOP();
	}

}


