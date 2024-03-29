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
 */

 /*************************** TAREA #2 - TALLER V - 2023_01 ****************************************
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
 * *************************************** PUNTO 1 **************************************************
 *
 * A) En la línea 161 del archivo correspondiente al GPIOxDriver.c se declara una función GPIO_ReadPin()
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
 * El problema que se detecta con los anteriores comandos de código es que al desplazar este
 * registro a la derecha, no se están limpiando los demás bits que no corresponden al bit menos
 * significativo que es el que realmente se quiere leer mediante la función ya definida anteriormente.
 * Además, debe tenerse en cuenta que dentro del input data register (IDR) se tienen 16 Bits que están
 * inicialmente reservados, por tanto no puede observarse la información existente en ellos y los Bits
 * del 0 al 15 sólo pueden configurarse como lectura, teniendo un total de 32 Bits dentro del IDR.
 * Visto de una forma un poco más clara, se puede suponer un caso donde se quiere leer el estado del
 * Pin_A3 que se encuentra apagado (se tiene un 0), pero existe un segundo Pin_A6 que se encuentra
 * encendido (se tiene un 1) para la posición del bit 6 en el IDR, por tanto, mediante la operación
 * shift que se plantea se mueve todo el registro 3 posiciones a la derecha, por lo cual se obtendría 0
 * en la posición 0 y 1 en la posición 3, así: 0b1000. El problema con esta operación es que no se
 * está llegando a lo que se quiere, dado que el Pin_A3 que se desea leer es un 0, por tanto,
 * esta debería ser la lectura. De allí la necesidad de limpiar las posiciones que no interesen
 * en el momento mediante el uso de una máscara que se indicará en (B).
 *
 * B) Para dar solución al problema que se plantea en el punto anterior, se añade una línea de código
 * que genera una copia del registro obtenido a partir del IDR que se almacena en una variable pinValue
 * desarrollando una operación AND (recordando que para operaciones bitwise la operación AND entre un 1
 * y cualquier otra cosa, normalmente 0, genera como salida esa "cualquier otra cosa") por tanto, si se
 * pone un 1 en la posición que se quiere leer y se limpia el resto del registro (poniendo 0 en los demás
 * bits) y se hace una operación AND bit a bit con el registro original, se obtiene una nueva situación
 * donde se leerá solamente el bit que interesa.
 *
 * C) Se crea la variable prueba que se inicializa en 0 para luego actualizar su valor a la lectura del
 * estado del Led User Pin que dentro del código se define en estado alto (o sea 1) tomando como base
 * una función ya creada para leer ese PIN. Se prueba entrando en el modo debug luego de compilar. Se
 * obtiene prueba = 1, por tanto la solución es efectiva.
 *
 ************************************************ PUNTO 2 **********************************************
 *
 * Para este ítem se pide implementar una función GPIOxTooglePin que reciba un único parámetro del tipo
 * GPIO_Handler_t que básicamente cambie el estado de un determinado PINx, es decir, si se encuentra en
 * estado alto (1) pase a bajo (0) y viceversa. En resumidas cuentas, se propone mediante una operación
 * XOR (^=) definida dentro de la función GPIOxTooglePin cambiar el valor del registro del ODR (Output
 * data register) del puerto GPIOx especificado (la operación XOR básicamente retorna un 0 si los estados
 * (alto o bajo) que se operan son iguales y 1 si son diferentes), entre ese valor actual del registro y
 * una máscara que hace un shift a izquierda de un 1 cuantas veces lo indique la posición del bit del
 * registro que se quiere cambiar, como se observa en las últimas líneas del archivo GPIOxDriver.c,
 * cambiando el estado del bit del puerto GPIO que se quiere. En otras palabras, si estaba en estado alto (1)
 * lo lleva a bajo (0) ó viceversa.
 *
 * Aquí, la definición de la función toogle que se analizó anteriormente:
 *
 * void GPIOxTooglePin (GPIO_Handler_t *pPinHandler){
 * pPinHandler->pGPIOx->ODR ^= (1 << pPinHandler->GPIO_PinConfig.GPIO_PinNumber);
 *
 * Además, se define adecuadamente esta función dentro de su correspondiente cabecera GPIOxDriver.h
 *
 *********************************************** PUNTO 3 ***********************************************
 *
 * En este ítem se propone realizar un contador de segundos binario que es una buena aproximación a un
 * reloj que muestre de una manera simple y elegante la conversión binaria que puede hacerse sobre el
 * sistema convencional en que se toman los segunderos comunes. El montaje consta de 7 diodos LED que se
 * disponen desde el bit menos significativo (posición 0) hasta el más significativo (bit 6) que cuenta
 * la dirección de conteo del reloj binario desde menor a mayor (ascendente) si el boton USER (color azul
 * del microcontrolador) está apagado (sin presionar) y va descendiendo si está presionado. Además,
 * se tiene en cuenta que al tener un minuto 60 segundos, el contador está acotado entre 0 y 60 segundos.
 *
 * Visto de izquierda a derecha desde el bit más significativo hasta el menos significativo se configuran
 * como salida los siguientes pines de propósito general GPIO para cada una de las posiciones binarias
 * en el siguiente orden: PC9, PC6, PB8, PA6, PC7, PC8 y PA7. Luego, para cada uno de estos pines se
 * disponen los parámetros de operación que deben definirse dentro del registro de acuerdo con la utilidad
 * que tendrán para realizar el conteo teniendo en cuenta la subdivisión del GPIO en el que se encuentra
 * cada PINx (A, B ó C) y se enciende el LED 2 (Color verde) del microcontrolador que hará las veces de led
 * de estado para el sistema que se está implementando. Luego, se implementa la configuración del botón USER
 * que interactúa directamente con el usuario para definir la dirección de conteo del sistema. El estado de
 * este botón será fundamental para la decisión del conteo, por ello, es base importante para entender los
 * condicionales que se implementan luego.
 *
 * Luego de cargar las configuraciones que se hicieron, se implementa la escritura del led de estado que
 * se mantendrá encendido. Dentro del ciclo infinito while de main.c se crea un primer ciclo for que se encarga
 * de mantener acotado el intervalo de operación del conteo del microcontrolador para que tenga siempre
 * un delay de aproximadamente 1 segundo para que el sistema cuente segundo a segundo, teniendo en cuenta
 * que el microcontrolador tiene una "frecuencia natural de operación" de 16 MHz. Ahora, conociendo que
 * la lectura del estado del boton user define la dirección de conteo, se plantea una condición if que
 * direcciona el programa por la ruta del conteo ascendente en caso que el botón esté en estado bajo y
 * descendente cuando se encuentra en estado alto.
 *
 * Se crea una variable auxiliar counter que se inicializa en 0 que sirve para definir los incrementos
 * o la disminución del sistema de acuerdo con el estado del botón user. Dentro de los condicionales if
 * se llama la función de escritura dentro de cada PIN que recibe además del parámetro &Handler una
 * operación AND entre un estado alto en la posición del bit que representa la salida de cada uno de los
 * pines y el contador que va aumentando o disminuyendo de acuerdo con el conteo de cada uno de los
 * segundos y luego lo desplaza a la posición del bit menos significativo para encender a apagar el led
 * de ese pin de acuerdo con la posición del conteo (para representar en qué segundo está el sistema).
 * Como tenemos 7 leds, tendremos las posiciones desde el bit-0 hasta el bit-6.
 *
 */

//Libreria para los tipos de variables
#include <stdint.h>

#include <stm32f411xx_hal.h>
#include <GPIOxDriver.h>


// Se crea la variable necesaria para inicializar el contador del tercer punto.
uint8_t counter = 0;


/*Función principal del programa. Es acá donde se ejecuta todo */
int main(void){


	//Definimos el Handler para cada uno de los pines que se requiere utilizar.
	GPIO_Handler_t handlerUserLedPin 	= {0}; //Led 2 azul del microcontrolador.
	GPIO_Handler_t handlerPC9 	 		= {0}; //Pin de la posición del bit 6.
	GPIO_Handler_t handlerPC6 			= {0}; //Pin de la posición del bit 5.
	GPIO_Handler_t handlerPB8 			= {0}; //Pin de la posición del bit 4.
	GPIO_Handler_t handlerPA6 			= {0}; //Pin de la posición del bit 3.
	GPIO_Handler_t handlerPC7			= {0}; //Pin de la posición del bit 2.
	GPIO_Handler_t handlerPC8 			= {0}; //Pin de la posición del bit 1.
	GPIO_Handler_t handlerPA7 			= {0}; //Pin de la posición del bit 0.
	GPIO_Handler_t handlerUserButton	= {0}; //Botón USER azul del microcontrolador.

	/*Se configura el led 2 del microcontrolador para que funcione como un led de estado.*/
	handlerUserLedPin.pGPIOx 								= GPIOA;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinNumber			= PIN_5;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_OUT;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinSpeed			= GPIO_OSPEED_MEDIUM;
	handlerUserLedPin.GPIO_PinConfig.GPIO_PinAltFunMode		= AF0;                //Ninguna función

	/*Se configura el pin PC9.*/
	handlerPC9.pGPIOx 										= GPIOC;
	handlerPC9.GPIO_PinConfig.GPIO_PinNumber				= PIN_9;
	handlerPC9.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC9.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC9.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC9.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC9.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el pin PC6.*/
	handlerPC6.pGPIOx 										= GPIOC;
	handlerPC6.GPIO_PinConfig.GPIO_PinNumber				= PIN_6;
	handlerPC6.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC6.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC6.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC6.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC6.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el pin PB8.*/
	handlerPB8.pGPIOx 										= GPIOB;
	handlerPB8.GPIO_PinConfig.GPIO_PinNumber				= PIN_8;
	handlerPB8.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPB8.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPB8.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPB8.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPB8.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el pin PA6.*/
	handlerPA6.pGPIOx 										= GPIOA;
	handlerPA6.GPIO_PinConfig.GPIO_PinNumber				= PIN_6;
	handlerPA6.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPA6.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPA6.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPA6.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPA6.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el pin PC7.*/
	handlerPC7.pGPIOx 										= GPIOC;
	handlerPC7.GPIO_PinConfig.GPIO_PinNumber				= PIN_7;
	handlerPC7.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC7.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC7.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC7.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC7.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el pin PC8.*/
	handlerPC8.pGPIOx 										= GPIOC;
	handlerPC8.GPIO_PinConfig.GPIO_PinNumber				= PIN_8;
	handlerPC8.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPC8.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPC8.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPC8.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPC8.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el pin PA7.*/
	handlerPA7.pGPIOx 										= GPIOA;
	handlerPA7.GPIO_PinConfig.GPIO_PinNumber				= PIN_7;
	handlerPA7.GPIO_PinConfig.GPIO_PinMode					= GPIO_MODE_OUT;
	handlerPA7.GPIO_PinConfig.GPIO_PinOPType				= GPIO_OTYPE_PUSHPULL;
	handlerPA7.GPIO_PinConfig.GPIO_PinPuPdControl			= GPIO_PUPDR_NOTHING;
	handlerPA7.GPIO_PinConfig.GPIO_PinSpeed					= GPIO_OSPEED_MEDIUM;
	handlerPA7.GPIO_PinConfig.GPIO_PinAltFunMode			= AF0;                //Ninguna función

	/*Se configura el BOTÓN USER como una entrada, a diferencia de los demás pines.*/
	handlerUserButton.pGPIOx 								= GPIOC;
	handlerUserButton.GPIO_PinConfig.GPIO_PinNumber			= PIN_13;
	handlerUserButton.GPIO_PinConfig.GPIO_PinMode			= GPIO_MODE_IN;
	handlerUserButton.GPIO_PinConfig.GPIO_PinOPType			= GPIO_OTYPE_PUSHPULL;
	handlerUserButton.GPIO_PinConfig.GPIO_PinPuPdControl	= GPIO_PUPDR_NOTHING;

	//Cargamos la configuración de cada uno de los pines, del led 2 y del botón user.
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

	// Prueba de efectividad del punto 1
	// Se crea una variable auxiliar para probar la solución del numeral 1.
	uint32_t prueba = 0;
	// Se llama la función ya definida para leer el estado del Pin User del micro.
	GPIO_ReadPin(&handlerUserLedPin);
	// Se actualiza el valor de la variable de prueba para verificar el estado del Pin User.
		prueba = GPIO_ReadPin(&handlerUserLedPin);
		//Retorno del estado de la prueba
		(void) prueba;

	//Este es el ciclo principal, donde se ejecuta todo el programa
	while(1){
		//Se define el delay de aproximadamente 1s.
		for (uint32_t i = 0; i < 1300000; i++);

		//Se crean condicionales IF para ver si el botón está presionado o no.
		//Si no está presionado:
		if (GPIO_ReadPin(&handlerUserButton) == SET){
			//Se condiciona la dirección del incremento para la variable auxiliar del contador.
			if (counter > 60){
				counter =1;

			}
			/* Se escribe en cada pin de acuerdo  con el valor del contador
			 * y la posición del bit que corresponde a cada pin (operación &)
			 * para ir incrementando el valor binario del segundero
			 */
			GPIO_WritePin(&handlerPA7, (0b1 << 0) & counter);
			GPIO_WritePin(&handlerPC8, ((0b1 << 1) & counter) >> 1);
			GPIO_WritePin(&handlerPC7, ((0b1 << 2) & counter) >> 2);
			GPIO_WritePin(&handlerPA6, ((0b1 << 3) & counter) >> 3);
			GPIO_WritePin(&handlerPB8, ((0b1 << 4) & counter) >> 4);
			GPIO_WritePin(&handlerPC6, ((0b1 << 5) & counter) >> 5);
			GPIO_WritePin(&handlerPC9, ((0b1 << 6) & counter) >> 6);
			//la variable auxiliar contador va aumentando.
			counter++;

		}else{
			//Si el botón está presionado
			if (GPIO_ReadPin(&handlerUserButton) == RESET){
				//Se condiciona la dirección del incremento para la variable auxiliar del contador.
				if (counter < 1){
					counter = 60;

				}
		    /* Se escribe en cada pin de acuerdo  con el valor del contador y la posición del
		     * bit que corresponde a cada pin (operación &) para ir decrementando el valor
			 * binario del segundero
			 */
			GPIO_WritePin(&handlerPA7, (0b1 << 0) & counter);
			GPIO_WritePin(&handlerPC8, ((0b1 << 1) & counter) >> 1);
			GPIO_WritePin(&handlerPC7, ((0b1 << 2) & counter) >> 2);
			GPIO_WritePin(&handlerPA6, ((0b1 << 3) & counter) >> 3);
			GPIO_WritePin(&handlerPB8, ((0b1 << 4) & counter) >> 4);
			GPIO_WritePin(&handlerPC6, ((0b1 << 5) & counter) >> 5);
			GPIO_WritePin(&handlerPC9, ((0b1 << 6) & counter) >> 6);
			//la variable auxiliar contador va decayendo.
			counter--;

		}
		}

}
}//FIN DEL PROGRAMA.

