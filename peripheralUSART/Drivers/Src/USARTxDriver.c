/*
 * USARTxDriver.c
 *
 *  Created on: 26/04/2023
 *      Author: santiago
 */

#include <stm32f4xx.h>
#include "USARTxDriver.h"

/**
 * Configurando el puerto Serial...
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se está utilizando.
 */
void USART_Config(USART_Handler_t *ptrUsartHandler){

	//Se desactivan las interrupciones globales mientras se configura el sistema.
	__disable_irq();

	/* 1. Activamos la señal de reloj que viene desde el BUS al que pertenece el periferico */
	/* Lo debemos hacer para cada uno de las posibles opciones que tengamos (USART1, USART2, USART6) */
    /* 1.1 Configuramos el USART1 */
	if(ptrUsartHandler->ptrUSARTx == USART1){
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	}else if(ptrUsartHandler->ptrUSARTx == USART6){
		/*Configuramos el USART 6 que está en el mismo bus del USART 1 */
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
		// Luego el USART 2 que está en el bus 1.
	}else if(ptrUsartHandler->ptrUSARTx == USART2){
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	}


	/* 2. Configuramos el tamaño del dato, la paridad y los bit de parada */
	/* En el CR1 estan parity (PCE y PS) y tamaño del dato (M) */
	/* Mientras que en CR2 estan los stopbit (STOP)*/
	/* Configuracion del Baudrate (registro BRR) */
	/* Configuramos el modo: only TX, only RX, o RXTX */
	/* Por ultimo activamos el modulo USART cuando todo esta correctamente configurado */

	// 2.1 Comienzo por limpiar los registros, para cargar la configuración desde cero
	ptrUsartHandler->ptrUSARTx->CR1 = 0;
	ptrUsartHandler->ptrUSARTx->CR2 = 0;

	// 2.2 Configuracion del Parity:
	// Verificamos si el parity esta activado o no
    // Tenga cuidado, el parity hace parte del tamaño de los datos...
	if(ptrUsartHandler->USART_Config.USART_parity != USART_PARITY_NONE){

		// Verificamos si se ha seleccionado ODD or EVEN
		if(ptrUsartHandler->USART_Config.USART_parity == USART_PARITY_EVEN){
			// Es even, entonces cargamos la configuracion adecuada
			ptrUsartHandler->ptrUSARTx->CR1 &= ~(USART_CR1_PS);
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PCE;

		}else{
			// Si es "else" significa que la paridad seleccionada es ODD, y cargamos esta configuracion
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PS;
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_PCE;
		}
	}else{
		// Si llegamos aca, es porque no deseamos tener el parity-check
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_PCE;
	}

	// 2.3 Configuramos el tamaño del dato
	if(ptrUsartHandler->USART_Config.USART_datasize ==USART_DATASIZE_8BIT){

	    	// Inicialmente debe corroborarse si se quiere o no la paridad.
	    	if (ptrUsartHandler->USART_Config.USART_parity == USART_PARITY_NONE){
	    		// Si se quieren datos de 8 bits.
	    		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_M;
	    	}
	    	else{
	    		// Si la partidad esta activada, se agrega un BIT adicional.
	    		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;
	    	}
	    }
	    else{
	    	// Se configuran 9 bits para el tamaño.
	    	ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_M;
	    }

	// 2.4 Configuramos los stop bits (SFR USART_CR2)
	switch(ptrUsartHandler->USART_Config.USART_stopbits){
	case USART_STOPBIT_1: {
		// Debemos cargar el valor 0b00 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(USART_CR2_STOP_0);
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(USART_CR2_STOP_1);
		break;
	}
	case USART_STOPBIT_0_5: {
		// Debemos cargar el valor 0b01 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= (USART_CR2_STOP_0);
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(USART_CR2_STOP_1);
		break;
	}
	case USART_STOPBIT_2: {
		// Debemos cargar el valor 0b10 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(USART_CR2_STOP_0);
		ptrUsartHandler->ptrUSARTx->CR2 |= (USART_CR2_STOP_1);
		break;
	}
	case USART_STOPBIT_1_5: {
		// Debemos cargar el valor 0b11 en los dos bits de STOP
		ptrUsartHandler->ptrUSARTx->CR2 |= (USART_CR2_STOP_0);
		ptrUsartHandler->ptrUSARTx->CR2 |= (USART_CR2_STOP_1);
		break;
	}
	default: {
		// En el caso por defecto seleccionamos 1 bit de parada
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(USART_CR2_STOP_0);
		ptrUsartHandler->ptrUSARTx->CR2 &= ~(USART_CR2_STOP_1);
		break;
	}
	}

	// 2.5 Configuracion del Baudrate (SFR USART_BRR)
	// Ver tabla de valores (Tabla 73), Frec = 16MHz, overr = 0;
	if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_9600){
		// El valor a cargar es 104.1875 -> Mantiza = 104,fraction = 0.1875
		// Mantiza = 104 = 0x68, fraction = 16 * 0.1875 = 3
		// Valor a cargar 0x0683
		// Configurando el Baudrate generator para una velocidad de 9600bps
		ptrUsartHandler->ptrUSARTx->BRR = 0x0683;
	}

	else if (ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_19200) {
		// El valor a cargar es 52.0625 -> Mantiza = 52,fraction = 0.0625
		// Mantiza = 52 = 0x34, fraction = 16 * 0.1875 = 1
		// Escriba acá su código y los comentarios que faltan
		// Valor a cargar 0x0341
		ptrUsartHandler->ptrUSARTx->BRR = 0x0341;
	}

	else if(ptrUsartHandler->USART_Config.USART_baudrate == USART_BAUDRATE_115200){
		// Escriba acá su código y los comentarios que faltan
		// El valor a cargar es 8.6875 -> Mantiza = 8,fraction = 0.6875
		// Mantiza = 8 = 0x8, fraction = 16 * 0.6875 = 11 = B.
		ptrUsartHandler->ptrUSARTx->BRR = 0x008B;
	}

	// 2.6 Configuramos el modo: TX only, RX only, RXTX, disable
	switch(ptrUsartHandler->USART_Config.USART_mode){
	case USART_MODE_TX:
	{
		// Activamos la parte del sistema encargada de enviar
		ptrUsartHandler->ptrUSARTx->CR1 |=USART_CR1_TE;
		break;
	}
	case USART_MODE_RX:
	{
		// Activamos la parte del sistema encargada de recibir
		ptrUsartHandler->ptrUSARTx->CR1 |=USART_CR1_RE;
		break;
	}
	case USART_MODE_RXTX:
	{
		// Activamos ambas partes, tanto transmision como recepcion
		ptrUsartHandler->ptrUSARTx->CR1 |=USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 |=USART_CR1_RE;
		break;
	}
	case USART_MODE_DISABLE:
	{
		// Desactivamos ambos canales
		// Escriba acá su código
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		break;
	}

	default:
	{
		// Actuando por defecto, desactivamos ambos canales
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_TE;
		ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_UE;
		break;
	}
	}

	// 2.7 Activamos el módulo serial.
	if(ptrUsartHandler->USART_Config.USART_mode != USART_MODE_DISABLE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;
	}
	// Se verifica la activación de la interrupcion Rx
	if (ptrUsartHandler->ptrUSARTx == USART1){
	}

	/* 1.2 Configuramos el USART2 */
	else if (ptrUsartHandler->ptrUSARTx == USART2) {
		if (ptrUsartHandler->USART_Config.USART_enableIntRX == USART_RX_INTERRUPT_ENABLE) {
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RXNEIE;
			NVIC_EnableIRQ(USART2_IRQn);
		} else {
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RXNEIE;
		}
	}

	/* 1.3 Configuramos el USART6 */
	else if (ptrUsartHandler->ptrUSARTx == USART6) {
		if (ptrUsartHandler->USART_Config.USART_enableIntRX == USART_RX_INTERRUPT_ENABLE) {
			ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_RXNEIE;
			NVIC_EnableIRQ(USART6_IRQn);
		} else {
			ptrUsartHandler->ptrUSARTx->CR1 &= ~USART_CR1_RXNEIE;
		}
	}

	// 2.8 Activamos el módulo serial.
	if(ptrUsartHandler->USART_Config.USART_mode != USART_MODE_DISABLE){
		ptrUsartHandler->ptrUSARTx->CR1 |= USART_CR1_UE;
	}

	// Activamos las interrupciones globales
	__enable_irq();
}

/* funcion para escribir un solo char */
int writeChar(USART_Handler_t *ptrUsartHandler, int dataToSend ){
	while( !(ptrUsartHandler->ptrUSARTx->SR & USART_SR_TXE)){
		__NOP();
	}

	ptrUsartHandler->ptrUSARTx->DR = dataToSend;

	return dataToSend;
}


