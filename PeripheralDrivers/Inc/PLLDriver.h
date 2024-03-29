/*
 * PLLDriver.h
 *
 *  Created on: 20/05/2023
 *      Author: santiago
 */

#include <stdio.h>

#ifndef PLLDRIVER_H_
#define PLLDRIVER_H_

// Para el caso de 80 MHz, se va dividiendo entre 1,2,4,8,16
// Además, este bus sí puede dividirse en 1 porque va hasta 100 MHz.
/*#define APB2_PRESCALER_0	0		// 80 MHz
#define APB2_PRESCALER_2	1		// 40  MHz
#define APB2_PRESCALER_4	2		// 20  MHz
#define APB2_PRESCALER_8	3		// 10  MHz
#define APB2_PRESCALER_16	4		// 5   MHz

// Dada la limitación de los 50 MHz del Bus 1, NO hay división entre 1.
#define APB1_PRESCALER_2	0		// 40 MHz
#define APB1_PRESCALER_4	1		// 20 MHz
#define APB1_PRESCALER_8	2		// 10 MHz
#define APB1_PRESCALER_16	3		// 5 MHz*/

// Los voltajes para las frecuencias límite.
#define VOLTAGE_64MHZ	0	// Límite de 64MHz (menor o igual)
#define VOLTAGE_84MHZ	1	// Límite de 84MHz (menor o igual)
#define VOLTAGE_100MHZ	2	// Límite de 100MHz (menor o igual)

// Queremos poder configurar algunas frecuencias más entre 50 y 100 MHz.
#define FRECUENCY_100MHZ 0	// 100 MHZ
#define FRECUENCY_80MHZ  1 	// 80 MHz
#define FRECUENCY_70MHZ  2	// 70 MHz
#define FRECUENCY_65MHZ  3	// 60 MHZ

// Macro para la función del retorno de estado del PLL.
#define actualFrecuency		getConfigPLL()

typedef struct
{
	uint8_t PLL_voltage;
	/*uint8_t	APB1_prescaler;
	uint8_t	APB2_prescaler;*/
	uint8_t PLL_frecuency;
}PLL_Config_t;

typedef struct
{
	PLL_Config_t	PLL_Config;
}PLL_Handler_t;


// Definicion de cabeceras para funciones del PLL
void PLL_Config(PLL_Handler_t *ptrPLLHandler);
uint32_t getConfigPLL(void);
void prescalerNumber(uint8_t prescaler);
void selectCLock(uint8_t clock);
void chooseCLK(uint8_t clock);

#endif /* PLLDRIVER_H_ */

