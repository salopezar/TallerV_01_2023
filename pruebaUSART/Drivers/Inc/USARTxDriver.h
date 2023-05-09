/*
 * USARTxDriver.h
 *
 *  Created on: 26/04/2023
 *      Author: santiago
 */

#include <stdio.h>

#ifndef USARTXDRIVER_H_
#define USARTXDRIVER_H_

#define USART_MODE_TX		0
#define USART_MODE_RX		1
#define USART_MODE_RXTX		2
#define USART_MODE_DISABLE	3

#define USART_BAUDRATE_9600		0
#define USART_BAUDRATE_19200	1
#define USART_BAUDRATE_115200	2

#define USART_DATASIZE_8BIT		0
#define USART_DATASIZE_9BIT		1

#define USART_PARITY_NONE	0
#define USART_PARITY_ODD	1
#define USART_PARITY_EVEN	2

#define USART_STOPBIT_1		0
#define USART_STOPBIT_0_5	1
#define USART_STOPBIT_2		2
#define USART_STOPBIT_1_5	3

#define USART_RX_INTERRUP_ENABLE 1

/* Estructura para la configuración de la comunicacion:
 * Velocidad (baudrate)
 * Tamaño de los datos
 * Control de errores
 * Bit de parada
 */
typedef struct
{
	uint8_t USART_mode;
	uint8_t USART_baudrate;
	uint8_t USART_datasize;
	uint8_t USART_parity;
	uint8_t USART_stopbits;
}USART_Config_t;

/*
 * Definicion del Handler para un USART:
 * - Estructura que contiene los SFR que controlan el periferico
 * - Estructura que contiene la configuración especifica del objeto
 * - Buffer de recepcion de datos
 * - Elemento que indica cuantos datos se recibieron
 * - Buffer de transmision de datos
 * - Elemento que indica cuantos datos se deben enviar.
 */
typedef struct
{
	USART_TypeDef	*ptrUSARTx;
	USART_Config_t	USART_Config;
	uint8_t			receptionBuffer[64];
	uint8_t			dataInputSize;
	uint8_t			transmisionBuffer[64];
	uint8_t			dataOutputSize;
}USART_Handler_t;



/* Definicion de los prototipos para las funciones del USART */
void USART_Config(USART_Handler_t *ptrUsartHandler);
int writeChar(USART_Handler_t *ptrUsartHandler, int dataToSend );
void usart2Rx_Callback(void);
void usart6Rx_Callback(void);
void writeMsg(USART_Handler_t *ptrUsartHandler, char *msgToSend);
void USART2_IRQHandler(void);
void USART6_IRQHandler(void);
uint8_t getRxData(void);

#endif /* USARTXDRIVER_H_ */
