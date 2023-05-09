/*
 * SysTickDriver.c
 *
 *  Created on: 2/05/2023
 *      Author: santiago
 */

#include <stm32f4xx.h>
#include "SysTickDriver.h"

// Se definen las variables que se requieren y se inicializan.
uint64_t ticks = 0;
uint64_t ticks_start = 0;
uint64_t ticks_counting = 0;

void config_SysTick_ms(uint8_t systemClock){
	// Se reinicia el valor de la variable que cuenta el tiempo
	ticks = 0;

	// Se carga el valor que contiene el límite de incrementos que representan 1 ms.
	switch(systemClock){

	// Caso 0 para el reloj HSI -> 16 MHz
	case 0:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
		break;

	// Caso 1 para el reloj HSE
	case 1:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
		break;

	// Caso 2 para el reloj PLL a 100 MHz
	case 2:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_100MHz_1ms;
		break;

	// En caso de que se ingrese un valor diferente
	default:
		SysTick->LOAD = SYSTICK_LOAD_VALUE_16MHz_1ms;
		break;
	}

	// Limpiamos el valor actual del Systick
	SysTick->VAL = 0;

	// Configuramos el reloj interno como el reloj para el timer
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

	// Se desactivan las interrupciones globales
	__disable_irq();

	// Matriculamos la interrupcion en el NVIC
	NVIC_EnableIRQ(SysTick_IRQn);

	// Activamos la interrupción debida al conteo a cero del SysTick
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

	// Activamos el timer
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	// Activamos de nuevo las interrupciones globales
	__enable_irq();

}

uint64_t getTicks_ms(void){
	return ticks;
}

void delay_ms(uint32_t wait_time_ms){
	// Captura el primer valor del tiempo para comparar
	ticks_start = getTicks_ms();

	// Captura el segundo valor del tiempo para comparar
	ticks_counting = getTicks_ms();

	// Compara: Si el valor del counting es menor que el start + wait
	// actualiza el valor del counting
	// Repite esta operación hasta que counting sea mayor que el tiempo de espera
	while(ticks_counting < (ticks_start + (uint64_t)wait_time_ms)){

		// actualizar el valor
		ticks_counting = getTicks_ms();
	}

}

void SysTick_Handler(void){
	// Verificamos si se lanzó la interrupción
	if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk){

		// Se limpia la bandera
		SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG_Msk;

		// Incrementamos en 1 el contador
		ticks++;
	}
}


