/*
 * adcDriver.h
 *
 *  Created on: 3/06/2023
 *      Author: santiago
 */

#ifndef ADCDRIVER_H_
#define ADCDRIVER_H_

#include "stm32f4xx.h"

#define ADC_CHANNEL_0		0
#define ADC_CHANNEL_1		1
#define ADC_CHANNEL_2		2
#define ADC_CHANNEL_3		3
#define ADC_CHANNEL_4		4
#define ADC_CHANNEL_5		5
#define ADC_CHANNEL_6		6
#define ADC_CHANNEL_7		7
#define ADC_CHANNEL_8		8
#define ADC_CHANNEL_9		9
#define ADC_CHANNEL_10		10
#define ADC_CHANNEL_11		11
#define ADC_CHANNEL_12		12
#define ADC_CHANNEL_13		13
#define ADC_CHANNEL_14		14
#define ADC_CHANNEL_15		15
#define ADC_CHANNEL_16		16

#define ADC_RESOLUTION_12_BIT	0
#define ADC_RESOLUTION_10_BIT	1
#define ADC_RESOLUTION_8_BIT	2
#define ADC_RESOLUTION_6_BIT	3

#define ADC_ALIGNMENT_RIGHT		0
#define ADC_ALIGNMENT_LEFT		1

#define ADC_SAMPLING_PERIOD_3_CYCLES	0b000;
#define ADC_SAMPLING_PERIOD_15_CYCLES	0b001;
#define ADC_SAMPLING_PERIOD_28_CYCLES	0b010;
#define ADC_SAMPLING_PERIOD_56_CYCLES	0b011;
#define ADC_SAMPLING_PERIOD_84_CYCLES	0b100;
#define ADC_SAMPLING_PERIOD_112_CYCLES	0b101;
#define ADC_SAMPLING_PERIOD_144_CYCLES	0b110;
#define ADC_SAMPLING_PERIOD_480_CYCLES	0b111;

#define EXTI_ADC_EVENT			0
#define	TIMER_ADC_EVENT			1

#define TIM1_CH1				0
#define TIM1_CH2				1
#define TIM1_CH3				2
#define TIM2_CH2				3
#define TIM2_CH3				4
#define TIM2_CH4				5
#define TIM3_CH1				6
#define TIM4_CH4				7
#define TIM5_CH1				8
#define TIM5_CH2				9
#define TIM5_CH3				10

typedef struct
{
	uint8_t		channel;		// Canal ADC que será utilizado para la conversión ADC
	uint8_t		resolution;		// Precisión con la que el ADC hace la adquisición del dato
	uint16_t	samplingPeriod;	// Tiempo deseado para hacer la adquisición del dato
	uint8_t		dataAlignment;	// Alineación a la izquierda o a la derecha
	uint16_t	adcData;		//Dato de la conversión
	uint8_t		AdcEventType;	// Tipo de evento; EXTI O TIMER
	uint8_t		AdcChannelEvent; // Canal del timer para usar en el evento ADC
	uint8_t		adcMultiChannel[];
}ADC_Config_t;

void adc_Config(ADC_Config_t *adcConfig);
void configAnalogPin(uint8_t adcChannel);
void adcComplete_Callback(void);
void startSingleADC(void);
void stopSingleADC(void);
void startContinousADC(void);
void adcConfigEvents(ADC_Config_t *adcConfig);
uint16_t getADC(void);
void adcMultiChannel(ADC_Config_t *adcConfig, uint8_t numberOfConversion);


#endif /* ADCDRIVER_H_ */
