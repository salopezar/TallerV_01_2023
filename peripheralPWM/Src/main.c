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

/*
 ********************************* TAREA #3 - TALLER V *************************
 *
 * El objetivo principal de esta tarea consiste en mostrar un primer acercamiento
 * al uso de interrupciones mediante un contador numérico que se inicializa en 0
 * y va hasta el 99, que va a variar o a disminuir dependiendo del sentido de giro
 * de un encoder rotativo simple, haciendo uso de las características de las señales
 * que se obtienen de cada una de las salidas de este: Clock (CLK) que consiste
 * en una señal cuadrada de reloj, Data (PinDT) y el switch (Button SW). Las
 * señales CLK y PinDT originalmente están desfasadas 90°, por tanto, identificando
 * flancos de  bajada o de subida, se puede crear un programa que permita reconocer
 * mediante la superposición de estas señales si el encoder está rotando en sentido
 * horario o antihorario. En el caso del presente, se decide configurar los condicionales
 * if para detectar estos cambios de dirección mediante flancos de bajada. En términos
 * generales, la tarea busca traducir estos cambios de dirección del encoder rotativo
 * en dos momentos principales que se valen del uso de una pantalla 7 segmentos de dos
 * dígitos: en principio, se tiene por defecto un modo que muestra en la pantalla 7
 * segmentos un contador que va desde 0 a 99 y varía de acuerdo al sentido de rotación y
 * en un segundo momento, se recrea el tradicional juego del snake para reproducir una
 * barrita LED que sigue una secuencia determinada por este sentido de giro.
 *
 * NOTA IMPORTANTE: Sepa usted, señor usuario, que el sistema tiene dos modos generales
 * de operación que se valen de una interrupción que lanza el switch del encoder
 * rotativo (SW). Por defecto, usted entrará en un primer modo que sirve como contador,
 * que reconoce números entre 0 y 99 en un display de 7 segmentos que usted reconoce
 * fácilmente en el hardware. Cuando usted rota el encoder en sentido horario, el número
 * aumenta, y cuando lo hace en sentido antihorario, disminuye. Si usted decide cambiar de
 * modo, presiona el switch del encoder y se encuentra con el tradicional Snake game
 * donde dependendiendo del sentido de giro que usted decida, conseguirá barrer una
 * secuencia determinada de segmentos. Se le invita con todo respeto a que lo use
 * con responsabilidad y ante todo, se divierta.
 *
 * La tarea implementa las interrupciones vistas en el curso de taller v junto con
 * los temporizadores timmer para el microcontrolador STM32F411.
*/

// Se incluyen las cabeceras que se necesitan para el desarrollo.

/* Se ajustan los parámetros
 * de la función central del programa
 */
int main(void){

	while(1){

	}

}
