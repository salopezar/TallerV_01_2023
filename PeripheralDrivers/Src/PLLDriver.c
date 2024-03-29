/*
 * PLLDriver.c
 *
 *  Created on: 20/05/2023
 *      Author: santiago
 */

#include <stm32f4xx.h>
#include "PLLDriver.h"

uint32_t HSI_VALUE = 16000000;


void PLL_Config(PLL_Handler_t *ptrPLLHandler){

	// Se selecciona HSI como reloj interno del PLL en vez de un oscilador.
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);

	// Se observa el valor del HSI RDY.
	while( !(RCC->CR & RCC_CR_HSIRDY) );

	// Se inicializa el power enable clock del APB1.
	RCC->APB1ENR |= RCC_APB1RSTR_PWRRST;


	// Estas líneas adecúan el acceso a la memoria del microcontrolador
	// para comenzar a ejecutar las instrucciones. Ver ACR.
	FLASH->ACR |= 1<<8;
	FLASH->ACR |= 1<<9;
	FLASH->ACR |= 1<<10;


	/*
	 *  IMPORTANTE: SELECCIÓN DE POTENCIA Y TIEMPOS DE ESPERA.
	 *  Debe tener en cuenta que el microcontrolador necesita condiciones
	 *  diferentes internas de voltaje para operar cada una de las frecuencias
	 *  que se requiere, por ello, en el archivo .h se definen las macros para ello.
	 *  Por ello, en el reference manual existe una tabla con rangos adecuados para
	 *  los voltajes de las distintas frecuencias que pueden llegar a configurarse.
	 *  Además, deben establecerse diferentes tiempos de espera para cada rango.
	 */

	// Voltajes para las frecuencias dentro de un rango máximo de 64 MHz.
	if(ptrPLLHandler->PLL_Config.PLL_voltage == VOLTAGE_64MHZ){
		// Se toma 0 en el bit 15 y 1 en el bit 14 del PWR para la potencia.
		PWR->CR |= 01<<14;

		// Se configura el tiempo de espera.
		FLASH->ACR |= 1<<0;
	}
	else if(ptrPLLHandler->PLL_Config.PLL_voltage == VOLTAGE_84MHZ){
		// Se toma 1 en el bit 15 y 0 en el bit 14 del PWR para la potencia.
		PWR->CR |= 10<<14;

		// Se configura el tiempo de espera.
		FLASH->ACR |= 3<<0;
	}
	else if(ptrPLLHandler->PLL_Config.PLL_voltage == VOLTAGE_100MHZ){
		// Se toma 1 en el bit 15 y 1 en el bit 14 del PWR para la potencia.
		PWR->CR |= 11<<14;

		// Se configura el tiempo de espera.
		FLASH->ACR |= 5<<0;
	}
	else{
		// Implementamos la máxima frecuencia para el caso por defecto.
		PWR->CR |= 11<<14;

		// Tiempo de espera.
		FLASH->ACR |= 5<<0;
	}

	/*
	 * El preescaler null debe ser 1 para conseguir la máxima salida de frecuencia.
	 */
	RCC->CFGR |= 0001<<4;

	/*
	 * IMPORTANTÍSIMO:
	 * Debe tenerse como premisa inicial que la máxima frecuencia a la que el
	 * fabricante recomienda utilizar el APB1 es de 50 MHz, por tanto, como se
	 * espera inicialmente en el objetivo de la tarea operar el microcontrolador
	 * a 80 MHz que se encuentra evidentemente en un intervalo superior a este valor,
	 * se decide no incluir el primer preescaler que divide en 1 la frecuencia que se
	 * quiere implementar, sino comenzar con la división en 2,4,8,16...
	 */
	// Se divide en 2 el bus 1.
	RCC->CFGR |= (RCC_CFGR_PPRE1_DIV2);

	/*
	 * IMPORTANTÍSIMO:
	 * Debe tenerse como premisa inicial que la máxima frecuencia a la que el
	 * fabricante recomienda utilizar el APB2 es de 100 MHz, por tanto, como se
	 * espera inicialmente en el objetivo de la tarea operar el microcontrolador
	 * a 80 MHz, esta frecuencia se encuentra cómodamente ubicada antes de la
	 * frecuencia máxima del bus de datos, por tanto, el preescaler 0 que mantiene
	 * la frecuencia original, puede implementarse, dividiendo en 1.
	 */
	// Se divide en 1 el bus 2.
	RCC->CFGR |= (RCC_CFGR_PPRE2_DIV1);

// Se debe configurar el sistema de manera que cuando la entrada del HSI
// sean 80 MHz, se mantenga.
	// La fuente de reloj del micro se configura sobre el HSI,
	RCC->PLLCFGR |= (0 << 22);

	// Se divide la entrada de frecuencia en 8, para obtener 2 MHz.
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_4);
	RCC->PLLCFGR |= (RCC_PLLCFGR_PLLM_3);

	// Se aplica una máscara un poco rudimentaria para garantizar la limpieza del
	// del registro.
	RCC->PLLCFGR &= 0b1111111111111000000000111111;

	// Se ecriben los diferentes valores de las frecuencias que se quieren multiplicar.
	/* NOTE que aquí es donde se escriben cada una de las frecuencias dentro del
	 * registro de 9 bits que se tiene dentro del registro PLLCFGR del RCC qur se deciden
	 * aleatoriamente a gusto del usuario dentro del intervalo de voltaje escogido.
	 */
	if(ptrPLLHandler->PLL_Config.PLL_frecuency == FRECUENCY_80MHZ){
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_4);
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_6);
	}else if(ptrPLLHandler->PLL_Config.PLL_frecuency == FRECUENCY_100MHZ){
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_2);
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_5);
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_6);
	}else if(ptrPLLHandler->PLL_Config.PLL_frecuency == FRECUENCY_70MHZ){
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_1);
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_2);
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_6);
	}else if(ptrPLLHandler->PLL_Config.PLL_frecuency == FRECUENCY_65MHZ){
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_0);
		RCC->PLLCFGR |= (RCC_PLLCFGR_PLLN_6);
	}
	// Se escoge como factor de reloj principal PLLP el número dos para obtener
	// la salida deseada en cada caso.
	RCC->PLLCFGR |= (00 << 16);

	/*ACTIVACIÓN DEL PLL CON LOS PARÁMETROS ELEGIDOS ANTERIORMENTE. */
	// Se activa el PLL.
	RCC->CR |= (1<<24);
	// Se da un retardo que indica la activación del PLL.
	while (!(RCC->CR & (1<<25)));
	// Se introduce un 2 dentro del clock source del PLL.
	RCC->CFGR |= (2<<0);

	// Aquí se espera a que el PLL esté listo como reloj principal de la configuracion.
	while (!(RCC->CFGR & (2<<2)));

	// Se selecciona la señal del PLL.
	RCC->CFGR |= (RCC_CFGR_MCO1_0);
	RCC->CFGR |= (RCC_CFGR_MCO1_1);

	// Para un preescaler de 5 unidades.
	RCC->CFGR |=  (RCC_CFGR_MCO1PRE_0);
	RCC->CFGR |=  (RCC_CFGR_MCO1PRE_1);
	RCC->CFGR |=  (RCC_CFGR_MCO1PRE_2);

	// Limpiamos el registo clock control register -> HSITRIM
	RCC->CR &= ~(RCC_CR_HSITRIM);

	/* En control register se indica el resultado de la ecuación:
	 * (F_deseada - F_real)/48 kHz.
	 * Para ajustar el reloj interno del MCU, se escribe 12 en HSITRIM.
	 */
	RCC->CR |= RCC_CR_HSITRIM_2;
	RCC->CR |= RCC_CR_HSITRIM_3;

}// FIN DE LA FUNCIÓN DE CONFIGURACIÓN PARA DISTINTAS FRECUENCIAS.

// Función para saber el estado del PLL y así poder caracterizar lo demás.
uint32_t getConfigPLL(void){
	// Se buscan los valores que están definiendo los valores actuales del reloj PLL
	uint32_t PLLN = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN_Msk) >> RCC_PLLCFGR_PLLN_Pos;
	uint32_t PLLM = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM_Msk) >> RCC_PLLCFGR_PLLM_Pos;
	// Se hace la operación para saber el valor actual del reloj. (Asume PLLP =2)
	uint32_t clockMicro = (HSI_VALUE / PLLM) * PLLN / 2;
	return clockMicro;

}

void chooseCLK(uint8_t clock){
	switch(clock){
	case 1:{
		//Seleccionamos la señal PLL
		RCC->CFGR |= (RCC_CFGR_MCO1_0);
		RCC->CFGR |= (RCC_CFGR_MCO1_1);
		break;
	}
	case 2:{
		// Seleccionamos la señal LSE
		RCC->CFGR |=  (RCC_CFGR_MCO1_0);
		RCC->CFGR &= ~(RCC_CFGR_MCO1_1);
		break;
	}
	case 3:{
		// Seleccionamos la señal HSI
		RCC->CFGR &= ~(RCC_CFGR_MCO1_0);
		RCC->CFGR &= ~(RCC_CFGR_MCO1_1);
		break;
	}
	default:{
		break;
		} //Fin caso por defecto
	}
}

// Función para el comando que selecciona el preescaler
void prescalerNumber(uint8_t prescaler){
	switch(prescaler){
	case 1:{
		// Sin prescaler
		RCC->CFGR &= ~(RCC_CFGR_MCO1PRE_0);
		RCC->CFGR &= ~(RCC_CFGR_MCO1PRE_1);
		RCC->CFGR &= ~(RCC_CFGR_MCO1PRE_2);
		break;
	}
	case 2:{
		// prescaler de 2
		RCC->CFGR &= ~(RCC_CFGR_MCO1PRE_0);
		RCC->CFGR &= ~(RCC_CFGR_MCO1PRE_1);
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_2);
		break;
	}
	case 3:{
		// prescaler de 3
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_0);
		RCC->CFGR &= ~(RCC_CFGR_MCO1PRE_1);
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_2);
		break;
	}
	case 4:{
		//prescaler de 4
		RCC->CFGR &=  ~(RCC_CFGR_MCO1PRE_0);
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_1);
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_2);
		break;
	}
	case 5:{
		//prescaler de 5
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_0);
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_1);
		RCC->CFGR |=  (RCC_CFGR_MCO1PRE_2);
		break;
	}
	default:{
		break;
		} //Fin caso por defecto
	}

}



