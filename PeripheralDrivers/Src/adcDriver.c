/*
 * adcDriver.c
 *
 *  Created on: 3/06/2023
 *      Author: santiago
 */

#include "adcDriver.h"
#include "GPIOxDriver.h"

uint16_t	adcRawData = 0;
uint8_t		counter5 = 0;
GPIO_Handler_t handlerAdcPin = {0};

// Configuración general del ADC.
void adc_Config(ADC_Config_t *adcConfig){
	/* 1. Configuramos el PinX para que cumpla la función de canal análogo deseado. */
	configAnalogPin(adcConfig->channel);

	/* 2. Activamos la señal de reloj para el periférico ADC1 (bus APB2)*/
	/* Se nota de acuerdo con el datasheet que el ADC1 se encuentra en el bus
	 * 2 a máximo 100 MHz.
	 */
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	/* Comenzamos la configuración del ADC1 */
	/* 3. Resolución del ADC */
	switch(adcConfig->resolution){
	case ADC_RESOLUTION_12_BIT:
	{

		ADC1->CR1 &= ~ADC_CR1_RES;

		break;
	}

	case ADC_RESOLUTION_10_BIT:
	{
		// Debe escribirse 00 en el registro ADC_CR1->RES
		ADC1->CR1 |= ADC_CR1_RES_0;

		break;
	}

	case ADC_RESOLUTION_8_BIT:
	{
		// Debe escribirse 10 en el registro ADC_CR1->RES
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}

	case ADC_RESOLUTION_6_BIT:
	{
		// Debe escribirse 11 en el registro ADC_CR1->RES
		ADC1->CR1 |= ADC_CR1_RES_0;
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}

	default:
	{
		break;
	}
	}

	/* 4. Configuramos el modo Scan como ACTIVADO */
	ADC1->CR1 &= ~ADC_CR1_SCAN;

	/* 5. Configuramos la alineación de los datos (derecha o izquierda) */
	if(adcConfig->dataAlignment == ADC_ALIGNMENT_RIGHT){
		// Alineación a la derecha (esta es la forma "natural")
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
	}
	else{

		// Alineación a la izquierda (para algunos cálculos matemáticos)
		ADC1->CR2 |= ADC_CR2_ALIGN;
	}

	/* 6. Desactivamos el "continuos mode" */
	ADC1->CR2 &= ~ADC_CR2_CONT;

	/* 7. Acá se debería configurar el sampling...*/
	if(adcConfig->channel <= ADC_CHANNEL_9){
		ADC1->SMPR2 |= (adcConfig->samplingPeriod << 3*(adcConfig->channel));
	}
	else{
		ADC1->SMPR1 |= (adcConfig->samplingPeriod << 3*(adcConfig->channel - 10));

	}

	/* 8. Configuramos la secuencia y cuantos elementos hay en la secuencia */
	// Al hacerlo todo 0, estamos seleccionando solo 1 elemento en el conteo de la secuencia
	ADC1->SQR1 = 0;

	// Asignamos el canal de la conversión a la primera posición en la secuencia
	ADC1->SQR3 |= (adcConfig->channel << 0);

	/* 9. Configuramos el preescaler del ADC en 2:1 (el mas rápido que se puede tener */
	ADC->CCR |= ADC_CCR_ADCPRE_0;

	/* 10. Desactivamos las interrupciones globales */
	__disable_irq();

	/* 11. Activamos la interrupción debida a la finalización de una conversión EOC (CR1)*/
	ADC1->CR1 |= ADC_CR1_EOCIE;

	/* 11a. Matriculamos la interrupción en el NVIC*/
	__NVIC_EnableIRQ(ADC_IRQn);

	/* 11b. Configuramos la prioridad para la interrupción ADC */
	__NVIC_SetPriority(ADC_IRQn, 1);

	/* 12. Activamos el modulo ADC */
	// Esto "prende" o "apaga" la conversión.
	ADC1->CR2 |= ADC_CR2_ADON;

	/* 13. Activamos las interrupciones globales */
	__enable_irq();
}

/*
 * Esta función lanza la conversion ADC y si la configuración es la correcta, solo se hace
 * una conversion ADC.
 * Al terminar la conversion, el sistema lanza una interrupción y el dato es leido en la
 * función callback, utilizando la funciona getADC().
 *
 * */
// Función para la conversión ADC simple (inicio)
void startSingleADC(void){
	/* Desactivamos el modo continuo de ADC */
	ADC1->CR2 &= ~ADC_CR2_CONT;

	/* Limpiamos el bit del overrun (CR1) */
	ADC1->CR1 &= ~ADC_CR1_OVRIE;

	/* Iniciamos un ciclo de conversión ADC (CR2)*/
	ADC1->CR2 |= ADC_CR2_SWSTART;

}

// Función para parar la conversión ADC simple
void stopSingleADC(void){

	/* Iniciamos un ciclo de conversión ADC (CR2)*/
	// Con este bit el sistema "resetea" la conversión.
	ADC1->CR2 &= ~ADC_CR2_SWSTART;

}

/*
 * Esta función habilita la conversion ADC de forma continua.
 * Una vez ejecutada esta función, el sistema lanza una nueva conversion ADC cada vez que
 * termina, sin necesidad de utilizar para cada conversion el bit SWSTART del registro CR2.
 * Solo es necesario activar una sola vez dicho bit y las conversiones posteriores se lanzan
 * automaticamente.
 * */
void startContinousADC(void){

	/* Activamos el modo continuo de ADC */
	ADC1->CR2 |= ADC_CR2_CONT;


	/* Iniciamos un ciclo de conversión ADC */
	ADC1->CR2 |= ADC_CR2_SWSTART;

}

/*
 * Función que retorna el ultimo dato adquirido por la ADC
 * La idea es que esta función es llamada desde la función callback, de forma que
 * siempre se obtiene el valor mas actual de la conversión ADC.
 * */
uint16_t getADC(void){
	// Esta variable es actualizada en la ISR de la conversión, cada vez que se obtiene
	// un nuevo valor.
	return adcRawData;
}

/*
 * Esta es la ISR de la interrupción por conversión ADC
 */
void ADC_IRQHandler(void){
	// Evaluamos que se dio la interrupción por conversión ADC
	if(ADC1->SR & ADC_SR_EOC){
		// Leemos el resultado de la conversión ADC y lo cargamos en una variale auxiliar
		// la cual es utilizada en la función getADC()
		adcRawData = ADC1->DR;

		// Hacemos el llamado a la función que se ejecutará en el main
		adcComplete_Callback();
	}

}

/* Función debil, que debe ser sobreescrita en el main. */
__attribute__ ((weak)) void adcComplete_Callback(void){
	__NOP();
}

/*
 * Con esta función configuramos que pin deseamos que funcione como canal ADC
 * Esta funcion trabaja con el GPIOxDriver, por lo cual requiere que se incluya
 * dicho driver.
 */
void configAnalogPin(uint8_t adcChannel){

	// Con este switch seleccionamos el canal y lo configuramos como análogo.
	switch (adcChannel) {

	/*
	 * NOTA IMPORTANTE:
	 * Para ver a qué PIN debe configurarse el correspondiente canal de conversión
	 * ADC, debe verse la tabla de funciones alternativas para los pines en el
	 * datasheet del microcontrolador. Allí, puede verse claramente lo que se debe
	 * asignar. (Esto está más o menos en la página 40 del datasheet del micro)
	 */

	case ADC_CHANNEL_0: {
		// Es el pin PA0
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_0;
		// Nota: Para el ejercicio inicial solo se necesita este canal, los demas
		// se necesitan para trabajos posteriores.
		break;
	}
		;

	case ADC_CHANNEL_1: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_1;

		break;
	}

	case ADC_CHANNEL_2: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_2;

		break;
	}

	case ADC_CHANNEL_3: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_3;

		break;
	}

	case ADC_CHANNEL_4: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_4;

		break;
	}

	case ADC_CHANNEL_5: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5;

		break;
	}
	case ADC_CHANNEL_6: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_6;

		break;
	}
	case ADC_CHANNEL_7: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOA;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_7;

		break;
	}
	case ADC_CHANNEL_8: {
		//Es el pin PB0
		handlerAdcPin.pGPIOx 						= GPIOB;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_9: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOB;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_1;

		break;
	}
	case ADC_CHANNEL_10: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_0;

		break;
	}
	case ADC_CHANNEL_11: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_1;

		break;
	}
	case ADC_CHANNEL_12: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_2;

		break;
	}
	case ADC_CHANNEL_13: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_3;

		break;
	}
	case ADC_CHANNEL_14: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_4;

		break;
	}
	case ADC_CHANNEL_15: {
		// Buscar y configurar adecuadamente
		handlerAdcPin.pGPIOx 						= GPIOC;
		handlerAdcPin.GPIO_PinConfig.GPIO_PinNumber = PIN_5;

		break;
	}
	default: {
		break;
	}

	}
	/*
	 * Se carga la información de cada pin configurado sobre el GPIO.
	 */
	handlerAdcPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
	GPIO_Config(&handlerAdcPin);
}

/*
 * Esta función recibe como parámetro principal la configuración del
 * adc para seleccionar los eventos que corresponden internamente a cada
 * uno de los timers, dependiendo de cuál se quiere elegir como controlador
 * de la conversión adc. Los bits correspondientes a EXTSEL jerarquizan los
 * eventos externos dependiendo del canal que se quiera utilizar en cada uno
 * de los timmers disponibles para este propósito. Ver página 231 del reference
 * manual.
 */
void adcConfigEvents(ADC_Config_t *adcConfig) {

	// Trigger detection on the rising edge, se toman los flancos
	// de bajada para los trigger externos
	ADC1->CR2 |= ADC_CR2_EXTEN_0;

	if (adcConfig->AdcEventType == TIMER_ADC_EVENT) {
		switch (adcConfig->AdcChannelEvent) {

		case TIM1_CH1: {
			// Evento TIM1 CC1 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x0 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM1_CH2: {
			// Evento TIM1 CC3 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x1 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM1_CH3: {
			// Evento TIM1 CC3 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x2 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM2_CH2: {
			// Evento TIM2 CC2 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x3 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM2_CH3: {
			// Evento TIM2 CC3 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x4 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM2_CH4: {
			// Evento TIM2 CC4 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x5 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM3_CH1: {
			// Evento TIM3 CC1 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x7 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM4_CH4: {
			// Evento TIM4 CC4 event para lanzar la conversion ADC
			ADC1->CR2 |= (0x9 << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM5_CH1: {
			// Evento TIM5 CC1 event para lanzar la conversion ADC
			ADC1->CR2 |= (0xA << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM5_CH2: {
			// Evento TIM5 CC2 event para lanzar la conversion ADC
			ADC1->CR2 |= (0xB << ADC_CR2_EXTSEL_Pos);
			break;
		}

		case TIM5_CH3: {
			// Evento TIM5 CC3 event para lanzar la conversion ADC
			ADC1->CR2 |= (0xC << ADC_CR2_EXTSEL_Pos);
			break;
		}

		default: {
			// 1100: Timer 5 CC3 event
			ADC1->CR2 |= (0xC << ADC_CR2_EXTSEL_Pos);
			break;
		}

		}
	}

	else {
		ADC1->CR2 |= (0xF << ADC_CR2_EXTSEL_Pos);
	}
}

/*
 * Aquí defino una función que me permite utilizar más de un canal
 * para realizar varias conversiones ADC. La función recibe como parámetros
 * la configuración del ADC y el número de conversiones que se requieren,
 * que para efectos prácticos es el número de canales sobre los cuales
 * se van a hacer las conversiones.
 */
void adcMultiChannel(ADC_Config_t *adcConfig, uint8_t numberOfConversion){
	for(counter5 = 0; counter5<numberOfConversion; counter5++){
		/* 1. Configuramos el PinX para que cumpla la función de canal análogo deseado. */
		configAnalogPin(adcConfig->adcMultiChannel[counter5]);
	}

	/* 2. Activamos la señal de reloj para el periférico ADC1 (bus APB2)*/
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	/* Comenzamos la configuración del ADC1 */
	/* 3. Resolución del ADC */
	// Preferible utilizar la conversión a 12 bits para usar
	// la máxima capacidad del microcontrolador.
	switch (adcConfig->resolution) {
	case ADC_RESOLUTION_12_BIT: {

		ADC1->CR1 &= ~ADC_CR1_RES_0;
		ADC1->CR1 &= ~ADC_CR1_RES_1;
		break;
	}

	case ADC_RESOLUTION_10_BIT: {

		ADC1->CR1 |= ADC_CR1_RES_0;
		ADC1->CR1 &= ~ADC_CR1_RES_1;
		break;
	}

	case ADC_RESOLUTION_8_BIT: {

		ADC1->CR1 |= ADC_CR1_RES_1;
		ADC1->CR1 &= ~ADC_CR1_RES_0;
		break;
	}

	case ADC_RESOLUTION_6_BIT: {
		ADC1->CR1 |= ADC_CR1_RES_0;
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}

	default: {
		break;
	}
	}

	/* 4. Configuramos el modo Scan como ACTIVADO */
	ADC1->CR1 |= ADC_CR1_SCAN;
	// Se configura además el EOCS end of conversion.
	ADC1->CR2 |= ADC_CR2_EOCS;

	/* 5. Configuramos la alineación de los datos (derecha o izquierda) */
	if (adcConfig->dataAlignment == ADC_ALIGNMENT_RIGHT) {
		// Alineación a la derecha (esta es la forma "natural")
		ADC1->CR2 &= ~ADC_CR2_ALIGN;
	} else {
		// Alineación a la izquierda (para algunos cálculos matemáticos)
		ADC1->CR2 |= ADC_CR2_ALIGN;
	}

	/* 6. Desactivamos el "continuos mode" */
	ADC1->CR2 &= ~ADC_CR2_CONT;

	/* 7. Acá se debería configurar el sampling...*/



	for(counter5 = 0; counter5 < numberOfConversion; counter5++){
		if (adcConfig->adcMultiChannel[counter5] <= ADC_CHANNEL_9) {
			ADC1->SMPR2 &= ~(0b111 << (3 * (adcConfig->adcMultiChannel[counter5])));
			// Acá se establecen la cantidad de ciclos para cada canal (0 al 9)
			ADC1->SMPR2 |= (adcConfig->samplingPeriod << (3 * (adcConfig->adcMultiChannel[counter5])));

		} else {
			ADC1->SMPR1 &= ~(0b111 << (3 * (adcConfig->adcMultiChannel[counter5] - 10)));
			// Acá se establecen la cantidad de ciclos para cada canal (10 al 18)
			ADC1->SMPR1 |= (adcConfig->samplingPeriod << (3 * (adcConfig->adcMultiChannel[counter5] - 10)));

		}
	}
	ADC1->SQR1 = 0;
	ADC1->SQR2 = 0;
	ADC1->SQR3 = 0;

	/* 8. Configuramos la secuencia y cuantos elementos hay en la secuencia */
	// Al hacerlo todo 0, estamos seleccionando solo 1 elemento en el conteo de la secuencia
	ADC1->SQR1 |= (numberOfConversion - 1) << ADC_SQR1_L_Pos;

	// Asignamos el orden de la conversión dependiendo del canal en que
	// se esté haciendo, de allí la variación en la agrupación de los
	// registros.
	for(counter5 = 0; counter5 < numberOfConversion; counter5++){
		if(adcConfig->adcMultiChannel[counter5] <= 6){
			ADC1->SQR3 |= (adcConfig->adcMultiChannel[counter5] << (5 * counter5));
		}
		else if(adcConfig->adcMultiChannel[counter5] > 6 && adcConfig->adcMultiChannel[counter5] <= 12){
			ADC1->SQR2 |= (adcConfig->adcMultiChannel[counter5] << (5 * (counter5 - 7)));
		}
		else if(adcConfig->adcMultiChannel[counter5] > 12){
			ADC1->SQR1 |= (adcConfig->adcMultiChannel[counter5] << (5 * (counter5 - 13)));
		}
	}

	/* 9. Configuramos el preescaler del ADC en 2:1 (el mas rápido que se puede tener */
	ADC->CCR &= ~ADC_CCR_ADCPRE;

	/* 10. Desactivamos las interrupciones globales */
	__disable_irq();

	/* 11. Activamos la interrupción debida a la finalización de una conversión EOC (CR1)*/
	ADC1->CR1 |= ADC_CR1_EOCIE;

	/* 11a. Matriculamos la interrupción en el NVIC*/
	__NVIC_EnableIRQ(ADC_IRQn);

	/* 11b. Configuramos la prioridad para la interrupción ADC */
	__NVIC_SetPriority(ADC_IRQn, 4);

	/* 12. Activamos el modulo ADC */
	ADC1->CR2 |= ADC_CR2_ADON;

	/* 13. Activamos las interrupciones globales */
	__enable_irq();

}
