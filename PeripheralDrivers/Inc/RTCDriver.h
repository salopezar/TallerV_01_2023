/*
 * RTCDriver.h
 *
 *  Created on: 7/06/2023
 *      Author: santiago
 */

#ifndef RTCXDRIVER_H_
#define RTCXDRIVER_H_
#include "stm32f4xx.h"


#define MONDAY 			0b001
#define TUESDAY 		0b010
#define WEDNESDAY		0b011
#define THURSDAY		0b100
#define FRIDAY			0b101
#define SATURDAY		0b110
#define SUNDAY			0b111

#define RTC_AMPM		1
#define RTC_24H			0

#define AM				0
#define PM				1


typedef struct
{
	uint8_t   RTC_Hours;
	uint8_t   RTC_Minutes;
	uint8_t   RTC_Seconds;
	uint8_t   RTC_Days;
	uint8_t   RTC_Months;
	uint16_t  RTC_Years;
	uint8_t	  RTC_Wdu;
	uint8_t   RTC_AmPm;

}RTC_Handler_t;

void rtc_Config(RTC_Handler_t *ptrRtcHandler);
uint8_t* read_Time(void);
uint8_t* read_Date(void);
#endif /* RTCXDRIVER_H_ */

