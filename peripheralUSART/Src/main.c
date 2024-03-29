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
#include <stdint.h>
#include "stm32f4xx.h"
#include "BasicTimer.h"
#include "GPIOxDriver.h"
#include "ExtiDriver.h"

// Se define y se inicializa el LED de estado.
GPIO_Handler_t handlerBlinkyPin = {0};
// Definición del timer para el Blinky.
BasicTimer_Handler_t handlerTim2 = {0};

/* Se definen y se inicializan los elementos del ENCODER.*/
GPIO_Handler_t handlerPinDT 	= {0};
GPIO_Handler_t handlerCLK		= {0};
GPIO_Handler_t handlerButtonSW	= {0};

// Elementos para las interrupciones del encoder.
EXTI_Config_t handlerExtiCLK 		= {0};
EXTI_Config_t handlerExtiButtonSW 	= {0};

// Se declaran las variables necesarias para las banderas
char flagClock 		= 0;
char flagButtonSW 	= 0;
char flagPinDT 		= 0;


// Definición del contador para los números (primer modo)
uint8_t counter = 0;

// Definición del contador para el snake (segundo modo)
uint8_t counterS = 0;

//Definición de elementos necesarios para el DISPLAY
GPIO_Handler_t handlerDisplay_Izq = {0};
GPIO_Handler_t handlerDisplay_Der = {0};
GPIO_Handler_t handlerPin_a = {0};
GPIO_Handler_t handlerPin_b = {0};
GPIO_Handler_t handlerPin_c = {0};
GPIO_Handler_t handlerPin_d = {0};
GPIO_Handler_t handlerPin_e = {0};
GPIO_Handler_t handlerPin_f = {0};
GPIO_Handler_t handlerPin_g = {0};

BasicTimer_Handler_t handlerDisplayTimer = {0};
// Se definen las funciones creadas para el primer modo conteo.
void showNumber (void);
void showDisplay(uint8_t numero);

// Funciones para el timer del encoder y declaración de los callback.
void BasicTimer2_Callback(void);
void callback_extInt11(void);
void callback_extInt15(void);
void BasicTimer5_Callback(void);

// Funciones para el snake game (segundo modo)
void showSegment (void);
void snakeGame(uint8_t posAngular);

// Función que inicializa el sistema.
void init_Hardware(void);

/* Se ajustan los parámetros
 * de la función central del programa
 */
int main(void){

	//Se inician todos los elementos necesarios del sistema.
	init_Hardware();

	while(1){

		/* Dado que por definición inicialmente se define el modo de operación
		 * de acuerdo con el SW button del encoder, se tiene una bandera flagButtonSW
		 * que indica el estado de dicho botón, por tanto, si está en bajo por defecto
		 * el sistema entrará en el modo contador y si se presiona, se pasará al modo
		 * snake game o juego de la culebrita.
		 */
		switch (flagButtonSW){
		//El botón está en bajo por defecto.
		case 0:

			/* Se definen las condiciones generales para la rotación del ENCODER */
			/* El ciclo se fundamenta en la bandera de la función de reloj cuya
			 * señal está siempre llegando al microcontrolador, por tanto, como
			 * se indicó en anteriores comentarios de documentación, si se toma
			 * en un mismo punto del dominio temporal la superposición de la señal
			 * del pin de datos (Pin data) con la señal del reloj, se encuentra
			 * que si la lectura del pin de datos es 1, el contador debe ir aumentando
			 * y si es 0, debe ir disminuyendo. Además, recuerde que se toman como
			 * referencia los flancos de bajada en la configuración.
			 */
			if(flagClock){
				if((GPIO_ReadPin(&handlerPinDT)) == 1){
					if (counter >= 99) {
						counter = 99;
					}
					else{
						counter++;
					}
				}
				/* Se considera que cuando el encoder gire en sentido antihorario,
			 	   hay disminución en el valor. */
				else if((GPIO_ReadPin(&handlerPinDT)) == 0){
					// Se restringe el intervalo de operación del contador entre 0 y 99.
					if(counter <= 0){
					counter = 0;
					}
					else{
						counter--;
					}

				}
				// Se baja la bandera del exti.
				flagClock = 0;
			}
			//Se llama la función que muestra los números.
			showNumber();
			break;
		// Se entra en el segundo modo de operación del snake game.
		case 1:
			/* Se define el sentido de rotación del encoder con los mismos criterios
			 * del modo anterior.
			 */
			if(flagClock){
				if((GPIO_ReadPin(&handlerPinDT)) == 1){
					if (counterS >= 11) {
						counterS = 0;
					}
					else{
						counterS++;
					}
				}
				/* Se considera que cuando el encoder gire en sentido antihorario,
			 	   hay disminución en el valor. */
				else if((GPIO_ReadPin(&handlerPinDT)) == 0){
					/* Se restringe el intervalo de operación del contador entre 0 y 11
					 * dado que estos son los casos definidos en la función showSegment
					 * que indica la secuencia en que deben irse encendiendo los segmentos
					 * del modo culebrita.
					 */

					// Sentido de decrecimiento del nuevo contador counterS del modo 2.
					if(counterS <= 0){
					counterS = 11;
					}
					else{
						counterS--;
					}

				}
				// Se baja la bandera del exti.
				flagClock = 0;
			}
			showSegment();
			break;

		}
	}
}


// Se crea esta función para mostrar las unidades y las decenas en el display.
void showNumber (void){
	if(GPIO_ReadPin(&handlerDisplay_Izq) == RESET){
		GPIO_WritePin(&handlerDisplay_Izq, 0);
		GPIO_WritePin(&handlerDisplay_Der, 1);
		showDisplay(counter % 10);
	}else{
		GPIO_WritePin(&handlerDisplay_Izq, 1);
		GPIO_WritePin(&handlerDisplay_Der, 0);
		showDisplay(counter / 10);
	}
}

/* Se define la función que va a configurar los segmentos en el display de acuerdo
   al número. */
void showDisplay(uint8_t numero){
	switch(numero){
	case 0:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, RESET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;
	case 1:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;
	case 2:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, RESET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;
	case 3:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;
	case 4:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;
	case 5:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;
	case 6:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, RESET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;
	case 7:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;
	case 8:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, RESET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;
	case 9:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, RESET);
		break;

	}

}

/* Se crea esta función para mostrar los segmentos en el display, discriminando
 * si se encuentra en el display de la izquierda o la derecha dependiendo de
 * la distribución de los segmentos dentro de la secuencia escogida.
 */
void showSegment (void){
	if(counterS == 0 || counterS == 5 || counterS == 6 || counterS == 9 || counterS == 10 || counterS == 11){
		GPIO_WritePin(&handlerDisplay_Izq, 0);
		GPIO_WritePin(&handlerDisplay_Der, 1);
		snakeGame(counterS);

	}else if(counterS == 1 || counterS == 2 || counterS == 3 || counterS == 4 || counterS == 7 || counterS == 8){
		GPIO_WritePin(&handlerDisplay_Izq, 1);
		GPIO_WritePin(&handlerDisplay_Der, 0);
		snakeGame(counterS);
	}
}

/* Esta función determina la secuencia en que van a encenderse los segmentos
 * de acuerdo con la rotación del encoder para la culebrita del segundo modo.
 */
void snakeGame (uint8_t posAngular){
	switch (posAngular){
	case 0:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 1:
		GPIO_WritePin(&handlerPin_a, RESET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 2:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 3:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, RESET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 4:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 5:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, RESET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 6:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, RESET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 7:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;
	case 8:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;
	case 9:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, RESET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 10:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, RESET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 11:
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, RESET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;

	case 12:{
		GPIO_WritePin(&handlerPin_a, SET);
		GPIO_WritePin(&handlerPin_b, SET);
		GPIO_WritePin(&handlerPin_c, SET);
		GPIO_WritePin(&handlerPin_d, SET);
		GPIO_WritePin(&handlerPin_e, SET);
		GPIO_WritePin(&handlerPin_f, SET);
		GPIO_WritePin(&handlerPin_g, SET);
		break;
	}
	default:{
		break;
	}
	}
}

// Es la función con la que se establecen las configuraciones generales del sistema.
void init_Hardware(void){


	/* Configuración del LED de estado que indica el funcionamiento del programa */
	handlerBlinkyPin.pGPIOx 									= GPIOA;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinNumber 				= PIN_5;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinSpeed  				= GPIO_OSPEED_FAST;
	handlerBlinkyPin.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;

	// Se carga lo que se hizo sobre el pin A5 sobre el que está el blinkyPin
	GPIO_Config(&handlerBlinkyPin);
	GPIO_WritePin(&handlerBlinkyPin, SET);

	// Configuración general en que se va a manejar el timer del blinky.
	handlerTim2.ptrTIMx								= TIM2;
	handlerTim2.TIMx_Config.TIMx_mode				= BTIMER_MODE_UP;
	handlerTim2.TIMx_Config.TIMx_speed				= BTIMER_SPEED_1ms;
	handlerTim2.TIMx_Config.TIMx_period				= 250; //Lanza una interrupcion cada 250 ms
	handlerTim2.TIMx_Config.TIMx_interruptEnable 	= BTIMER_INTERRUPT_ENABLE;

	/* Se carga ahora la configuración del TIMER */
	BasicTimer_Config(&handlerTim2);

	/* Se configuran las salidas del encoder */
	// Configuración para la salida del clock (CLK) del encoder.
	handlerCLK.pGPIOx									= GPIOC;
	handlerCLK.GPIO_PinConfig.GPIO_PinNumber			= PIN_11;
	handlerCLK.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_IN;
	handlerCLK.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

    // Se carga la configuración.
	GPIO_Config(&handlerCLK);

	// Se configura la interrupcion
    handlerExtiCLK.edgeType								= EXTERNAL_INTERRUPT_FALLING_EDGE; //flancos de bajada
    handlerExtiCLK.pGPIOHandler							= &handlerCLK;

    // Cargando la configuracion del EXTI
    extInt_Config(&handlerExtiCLK);


	/* Se configuran las salidas del encoder */
	// Configuración para la salida del DATA (DT) del encoder.
	handlerPinDT.pGPIOx									= GPIOC;
	handlerPinDT.GPIO_PinConfig.GPIO_PinNumber			= PIN_12;
	handlerPinDT.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerPinDT.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_NOTHING;

    // Se carga la configuración.
	GPIO_Config(&handlerPinDT);

	/* Se configuran las salidas del encoder */
	// Configuración para el botón (sw) del encoder.
	handlerButtonSW.pGPIOx									= GPIOA;
	handlerButtonSW.GPIO_PinConfig.GPIO_PinNumber			= PIN_15;
	handlerButtonSW.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_IN;
	handlerButtonSW.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerButtonSW.GPIO_PinConfig.GPIO_PinPuPdControl		= GPIO_PUPDR_PULLUP;
	handlerButtonSW.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_FAST;
	// Se carga lo hecho sobre el GPIO.
	GPIO_Config(&handlerButtonSW);

    handlerExtiButtonSW.edgeType								= EXTERNAL_INTERRUPT_FALLING_EDGE; //flancos de bajada
    handlerExtiButtonSW.pGPIOHandler							= &handlerButtonSW;
    // Cargando la configuracion del EXTI
    extInt_Config(&handlerExtiButtonSW);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_a.pGPIOx										= GPIOA;
	handlerPin_a.GPIO_PinConfig.GPIO_PinNumber				= PIN_4;
	handlerPin_a.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_a.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_a.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_a.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_a);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_b.pGPIOx										= GPIOA;
	handlerPin_b.GPIO_PinConfig.GPIO_PinNumber				= PIN_1;
	handlerPin_b.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_b.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_b.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_b.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_b);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_c.pGPIOx										= GPIOA;
	handlerPin_c.GPIO_PinConfig.GPIO_PinNumber				= PIN_10;
	handlerPin_c.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_c.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_c.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_c.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_c);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_d.pGPIOx										= GPIOB;
	handlerPin_d.GPIO_PinConfig.GPIO_PinNumber				= PIN_3;
	handlerPin_d.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_d.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_d.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_d.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_d);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_e.pGPIOx										= GPIOB;
	handlerPin_e.GPIO_PinConfig.GPIO_PinNumber				= PIN_4;
	handlerPin_e.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_e.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_e.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_e.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_e);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_f.pGPIOx										= GPIOC;
	handlerPin_f.GPIO_PinConfig.GPIO_PinNumber				= PIN_2;
	handlerPin_f.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_f.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_f.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_f.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_f);

	/* Configuracion de los SEGMENTOS del DISPLAY */
	handlerPin_g.pGPIOx										= GPIOB;
	handlerPin_g.GPIO_PinConfig.GPIO_PinNumber				= PIN_5;
	handlerPin_g.GPIO_PinConfig.GPIO_PinMode				= GPIO_MODE_OUT;
	handlerPin_g.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPin_g.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPin_g.GPIO_PinConfig.GPIO_PinSpeed				= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerPin_g);

	/* Configuracion de los switcheos de los transistores */
	handlerDisplay_Izq.pGPIOx										= GPIOA;
	handlerDisplay_Izq.GPIO_PinConfig.GPIO_PinNumber				= PIN_12;
	handlerDisplay_Izq.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerDisplay_Izq.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerDisplay_Izq.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerDisplay_Izq.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_FAST;

	GPIO_Config(&handlerDisplay_Izq);

	/* Configuracion de los switcheos de los transistores */
	handlerDisplay_Der.pGPIOx										= GPIOA;
	handlerDisplay_Der.GPIO_PinConfig.GPIO_PinNumber				= PIN_11;
	handlerDisplay_Der.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerDisplay_Der.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerDisplay_Der.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerDisplay_Der.GPIO_PinConfig.GPIO_PinSpeed				    = GPIO_OSPEED_FAST;

	GPIO_Config(&handlerDisplay_Der);

	//Configuración del timer para el display.
	// Se selecciona el TIMER que se elegió trabajar en el programa.

	handlerDisplayTimer.ptrTIMx 								= TIM5;
	handlerDisplayTimer.TIMx_Config.TIMx_mode 					= BTIMER_MODE_UP;
	handlerDisplayTimer.TIMx_Config.TIMx_speed 					= BTIMER_SPEED_1ms;
	handlerDisplayTimer.TIMx_Config.TIMx_period 				= 10;
	handlerDisplayTimer.TIMx_Config.TIMx_interruptEnable		= BTIMER_INTERRUPT_ENABLE;

	BasicTimer_Config(&handlerDisplayTimer);

}// Termina el init_Hardware

// Basic timer sobre el que está puesto el led de estado.
void BasicTimer2_Callback(void){
	GPIOxTooglePin(&handlerBlinkyPin);
}

// Se definen los switcheos de los transistores dentro del timer para los display.
void BasicTimer5_Callback(void){
	GPIOxTooglePin(&handlerDisplay_Der);
	GPIOxTooglePin(&handlerDisplay_Izq);
}

// Interrupción de la señal de reloj.
void callback_extInt11(void){
	flagClock = 1;
}

// Interrupción para definir el modo.
void callback_extInt15(void){
	flagButtonSW ^= 1;
}
