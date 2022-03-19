/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: WDT.h											*/
/********************************************************/

#ifndef __LEDS_H
#define __LEDS_H

#include "HAL.h"
#include <xdc/cfg/global.h>

// Tiempo de Tick de Timer B (en milisegundos). Usamos ACLK = 1/8 de MCLK  (50ms es un tick del micro)
#define TIMERB_TICK_TIME   50

#define TIMER_B_TIMEOUT 	(unsigned long int)(((MAIN_XTAL/8) * TIMERB_TICK_TIME) * 0.001)
#define TIMER_B_TIMEOUT_2SEG 	65535/*5000*/

#define TIMER_LEDS_1_SECOND 	(1000/TIMERB_TICK_TIME)		//Ticks que corresponden a 1seg
#define TIMER_LEDS_2_SECOND 	(2000/TIMERB_TICK_TIME)
#define TIMER_LEDS_100_MILLS 	(100/TIMERB_TICK_TIME)
#define TIMER_LEDS_400_MILLS 	(400/TIMERB_TICK_TIME)
#define TIMER_LEDS_1500_MILLS 	(1500/TIMERB_TICK_TIME)

#define TIMERB_CLK_DIV			8									//ACLK/8
#define TIMERB_100_MSEC 		0.1 * (MAIN_XTAL/TIMERB_CLK_DIV)
#define TIMERB_250_MSEC 		0.25 * (MAIN_XTAL/TIMERB_CLK_DIV)
#define TIMERB_500_MSEC 		0.5 * (MAIN_XTAL/TIMERB_CLK_DIV)
#define TIMERB_1_SECOND 		1 * (MAIN_XTAL/TIMERB_CLK_DIV)		//Meter un CLK divider y actualizar estos valores
#define TIMERB_2_SECOND 		2 * (MAIN_XTAL/TIMERB_CLK_DIV)
#define TIMERB_4_SECOND 		4 * (MAIN_XTAL/TIMERB_CLK_DIV)
#define TIMERB_8_SECOND 		8 * (MAIN_XTAL/TIMERB_CLK_DIV)
#define TIMERB_16_SECOND 		16 * (MAIN_XTAL/TIMERB_CLK_DIV)		//16seg -  MAX Timer value with clock divide 8 and clock of 32768Hz

#define INCREMENTO_Tactual_CADA_ISR		65535

#define NUM_TRIES_CONTROLLERPREVIO_BF	35/*30*/
#define NUM_TRIES_CONTROLLERPREVIO_AF	35/*30*/

void TIMER_B0_Config(void);
void TIMER_A1_Config(void);
void WDT_CheckWDT(void);
void ISR_TimerB0 (/*UArg arg0*/);
void ISR_TimerA1(void);
void BlinkLEDs(void);


void StartTimer (unsigned long t_interval , int numTimer);


#endif
