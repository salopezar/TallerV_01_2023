/*
 * SysTickDriver.h
 *
 *  Created on: 2/05/2023
 *      Author: santiago
 */

#ifndef INC_SYSTICKDRIVER_H_
#define INC_SYSTICKDRIVER_H_

#include <stm32f4xx.h>

#define SYSTICK_LOAD_VALUE_16MHz_1ms		16000   // Número de ciclos
#define SYSTICK_LOAD_VALUE_100MHz_1ms		100000  // Número de ciclos

void config_SysTick_ms(uint8_t systemClock);
uint64_t getTicks_ms(void);
void delay_ms(uint32_t wait_time_ms);


#endif /* SYSTICKDRIVER_H_ */
