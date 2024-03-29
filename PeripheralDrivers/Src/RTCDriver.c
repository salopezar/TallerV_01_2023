/*
 * RTCDriver.c
 *
 *  Created on: 7/06/2023
 *      Author: santiago
 */

#include "RTCDriver.h"


/* Driver RTC utilizando el cristal de baja velocidad LSE*/

// Definicion de arreglos para leer la hora y los datos del calendario
uint8_t time[6] = {0};
uint8_t date[5] = {0};

// Variables para almacenar los datos de tiempo
uint8_t RTC_UnidadesHoras    = 0;
uint8_t RTC_DecenasHoras     = 0;
uint8_t RTC_UnidadesMinutos  = 0;
uint8_t RTC_DecenasMinutos 	 = 0;
uint8_t RTC_UnidadesSegundos = 0;
uint8_t RTC_DecenasSegundos  = 0;
uint8_t RTC_AmPm 	 		 = 0;

//Definicion de variables para almacenar los datos de calendario
uint8_t RTC_diaUnidades 	 = 0;
uint8_t RTC_diaDecenas  	 = 0;
uint8_t RTC_mes   			 = 0;
uint8_t RTC_añoUnidades	 	 = 0;
uint8_t RTC_añoDecenas 	     = 0;
uint8_t RTC_wdu              = 0;

// Funcion que configura el RTC con el cristal LSE
void rtc_Config(RTC_Handler_t *ptrRtcHandler){

	/*Es necesario habilitar el reloj del APB1 donde se encuentra el periferico RTC. Además,
	 *se habilita el acceso de escritura porque el periferico se encuentra protegido
	 */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_DBP;

	RCC->BDCR |= RCC_BDCR_RTCEN;    // Se activa la señal de reloj
	RCC->BDCR |= RCC_BDCR_LSEON;    // Se activa el cristal LSE
	RCC->BDCR |= RCC_BDCR_RTCSEL_0; // Se selecciona el LSE como fuente del RTC

	// Se espera hasta que el LSE este listo
	while(!(RCC->BDCR & RCC_BDCR_LSERDY));

	 /* Se ingresan las claves dadas por el fabricante en el registro de proteccion contra
	 * escritura (Para más detalles, ver página 438 del reference manual)
	 */

    RTC->WPR |= 0xCA;
    RTC->WPR |= 0x53;

    // Se entra en el modo de inicializacion (Se habilita que el usuario pueda cambiar valores)
    RTC->ISR |= RTC_ISR_INIT;

    // Se espera a que se active este modo
    while(!(RTC->ISR & RTC_ISR_INITF));

    //Se configuran los prescaler para obtener 1 Hz, 127 para el asincrono y 255 para el sincrono.
    //32768 Hz / (127 +1) = 256 Hz
    //256 Hz /(255 + 1) = 1 Hz
    /* Las ecuaciones y la explicacion estan en la pagina 457 del reference manual */

    RTC->PRER |= 127 << RTC_PRER_PREDIV_A_Pos;
    RTC->PRER |= 255 << RTC_PRER_PREDIV_S_Pos;


    //Se configura el RTC para que tomen directamente valores de los contadores del calendario

    RTC->CR |= RTC_CR_BYPSHAD;

    // Se reinician los contadores del calendario
    RTC->TR = 0;
    RTC->DR = 0;

    // El formato está en 24 horas.
    RTC->CR &= ~RTC_CR_FMT;

    /* El registro time register esta en formato BCD. se realiza la division y el modulo para
     * obtener decenas y unidades.
     */

    RTC->TR |= ptrRtcHandler->RTC_Hours/10 << RTC_TR_HT_Pos;
    RTC->TR |= ptrRtcHandler->RTC_Hours%10 << RTC_TR_HU_Pos;
    RTC->TR |= ptrRtcHandler->RTC_Minutes/10 << RTC_TR_MNT_Pos;
    RTC->TR |= ptrRtcHandler->RTC_Minutes%10 << RTC_TR_MNU_Pos;
    RTC->TR |= ptrRtcHandler->RTC_Seconds/10  << RTC_TR_ST_Pos;
    RTC->TR |= ptrRtcHandler->RTC_Seconds%10 << RTC_TR_SU_Pos;

    //Para el formato AM-PM
    RTC->TR |= ptrRtcHandler->RTC_AmPm << RTC_TR_PM_Pos;

    /* El registro RTC data register se encuentra en formato BCD,se realiza la division y
     * el modulo para obtener decenas y unidades.
     */

    RTC->DR |= ptrRtcHandler->RTC_Months%10 << RTC_DR_MU_Pos;
    RTC->DR |= ptrRtcHandler->RTC_Months/10 << RTC_DR_MT_Pos;
    RTC->DR |= ptrRtcHandler->RTC_Days/10 << RTC_DR_DT_Pos;
    RTC->DR |= ptrRtcHandler->RTC_Days%10 << RTC_DR_DU_Pos;
    RTC->DR |= ptrRtcHandler->RTC_Wdu<< RTC_DR_WDU_Pos;

    /*El sistema por defecto tiene la fecha del año 2000, de este modo se resta 2000 al valor del handler
     * y se realiza la separacion de decenas y unidades porque los años se encuentran en formato BCD.
     */
    RTC->DR |= ((ptrRtcHandler->RTC_Years -2000)%10) << RTC_DR_YU_Pos;
    RTC->DR |= (ptrRtcHandler->RTC_Years - 2000)/10 << RTC_DR_YT_Pos;

    // Activamos nuevamente el real time clock
    RCC->BDCR |= RCC_BDCR_RTCEN;

    // Salimos del modo de inicializacion
    RTC->ISR &= ~RTC_ISR_INIT;

    // Se habilita la proteccion de bits
    PWR->CR &= ~ PWR_CR_DBP;

    // Se escribe una clave erronea para que write protection se bloquee
    RTC->WPR = (0xFF);

}

// Funcion que entrega un puntero para almacenar los resultados en el arreglo Tiempo
uint8_t* read_Time(void){

	//Extraemos el dato de los registros y los desplazamos a la primera posicion

	RTC_UnidadesHoras = ((RTC->TR & RTC_TR_HU_Msk)>>RTC_TR_HU_Pos);
	RTC_DecenasHoras = ((RTC->TR & RTC_TR_HT_Msk)>> RTC_TR_HT_Pos);
	RTC_UnidadesMinutos = ((RTC->TR & RTC_TR_MNU_Msk)>>RTC_TR_MNU_Pos);
	RTC_DecenasMinutos = ((RTC->TR & RTC_TR_MNT_Msk)>>RTC_TR_MNT_Pos);
	RTC_UnidadesSegundos = ((RTC->TR & RTC_TR_SU_Msk)>>RTC_TR_SU_Pos);
	RTC_DecenasSegundos = ((RTC->TR & RTC_TR_ST_Msk)>>RTC_TR_ST_Pos);
    RTC_AmPm = ((RTC->TR & RTC_TR_PM_Msk)>>RTC_TR_PM_Pos);

    //Se almacenan los datos en el arreglo
    time[0] = (RTC_DecenasSegundos * 10) + RTC_UnidadesSegundos ;
    time[1] = (RTC_DecenasMinutos * 10) + RTC_UnidadesMinutos;
    time[2] = (RTC_DecenasHoras * 10) +  RTC_UnidadesHoras;
    time[3] = RTC_AmPm;

     return time;
}

// Funcion que entrega un puntero para almacenar los resultados en el arreglo Date
uint8_t* read_Date(void){

	//Extraemos el dato de los registros y los desplazamos a la primera posicion

	RTC_diaDecenas = ((RTC->DR & RTC_DR_DT_Msk)>> RTC_DR_DT_Pos);
    RTC_diaUnidades = ((RTC->DR & RTC_DR_DU_Msk)>>RTC_DR_DU_Pos);
    RTC_añoUnidades = ((RTC->DR & RTC_DR_YU_Msk)>>RTC_DR_YU_Pos);
    RTC_añoDecenas = ((RTC->DR & RTC_DR_YT_Msk)>>RTC_DR_YT_Pos);
    RTC_mes = (((RTC->DR & RTC_DR_MT_Msk)>>RTC_DR_MT_Pos)*10) + (((RTC->DR & RTC_DR_MU_Msk)>>RTC_DR_MU_Pos)%10);
    RTC_wdu = (RTC->DR & RTC_DR_WDU_Msk)>>RTC_DR_WDU_Pos;

    // Se almacenan los datos en el arreglo
    date[0] = (RTC_diaDecenas * 10) + RTC_diaUnidades;
    date[1] = RTC_mes;
    date[2] = (RTC_añoDecenas * 10) + RTC_añoUnidades;
    date[3] = RTC_wdu;

    return date;

}


