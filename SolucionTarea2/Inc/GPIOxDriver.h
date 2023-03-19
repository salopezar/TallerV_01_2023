/*
 * GPIOxDriver.h
 *
 *  Created on: 10/03/2023
 *      Author: santiago
 */

#ifndef GPIOXDRIVER_H_
#define GPIOXDRIVER_H_

//Se incluye también lo correspondiente al GPIOx
#include "stm32f411xx_hal.h"

typedef struct
{
	uint8_t GPIO_PinNumber;			//PIN con el que se trabaja
	uint8_t GPIO_PinMode;			//Modo de la configuración: entrada, salida, análogo, f. alternativa
	uint8_t GPIO_PinSpeed;			//Velocidad de respuesta del PIN (solo para digital)
	uint8_t GPIO_PinPuPdControl;	//Se selecciona si es una salida Pull-up, Pull-down o "libre"
	uint8_t GPIO_PinOPType;			//Trabaja de la mano del anterior, selecciona salida PUPD u OpenDrain
	uint8_t GPIO_PinAltFunMode;		//Selecciona cual es la funcion alternativa que se está configurando

}GPIO_PinConfig_t;

/* Esta es una estructura que contiene dos elementos:
 * -La direccion del puerto que se está utilizando (la referencia al puerto)
 * -La configuracion específica del PIN que se está utiizando
 * */
typedef struct
{
	GPIOx_RegDef_t 		*pGPIOx;		/*!< Dirección del puerto al que el PIN corresponde >*/
	GPIO_PinConfig_t	GPIO_PinConfig; /*< Configuracion del PIN >*/

}GPIO_Handler_t;

/*Definición de las cabeceras de las funciones del GPIOxDriver */
void GPIO_Config (GPIO_Handler_t *pGPIOHandler);
void GPIO_WritePin(GPIO_Handler_t *pPinHandler, uint8_t newState);
uint32_t GPIO_ReadPin(GPIO_Handler_t *pPinHandler);
/*Definición de la función toogle del segundo punto */
void GPIOxTooglePin (GPIO_Handler_t *pPinHandler);




#endif /* GPIOXDRIVER_H_ */
