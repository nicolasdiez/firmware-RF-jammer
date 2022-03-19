/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: November 2015									*/
/* File: DAC.c											*/
/********************************************************/

#include "HAL.h"
#include "ports.h"
#include "DAC.h"

#ifdef	__MSP430F5638
	#include "msp430f5638.h"
#endif
#ifdef	__MSP430F5636
	#include "msp430f5636.h"
#endif

void DAC_Config(void)
{
	// Reference is automatically turned on from the REF module on DAC request
	// REF module default 1.5V is used

	DAC12_1CTL0 = DAC12OPS + DAC12IR + DAC12SREF_1 + DAC12AMP_5 /*+ DAC12ENC*/ + DAC12CALON;	//Pin Pn.z, DAC12 full-scale output, AVCC as ref +, Select settling time vs current consumption, DAC enable conversion, DAC calibration on
	__delay_cycles(100);	// Tsettle = 75us

	//NICO: MIRAR DAC12OPS para seleccionar el PIN de output del DAC

	//DAC12_1DAT = 0x7FF;
	//__bis_SR_register(LPM4_bits);             // Enter LPM4
}


void DAC_SetGainControl(unsigned short mVout2)
{
	unsigned long Vdac12dat1 = 0;
	unsigned long Vdac12dat2 = 0;

	Vdac12dat1 = /*(12<<mVout2)*/ (unsigned long)mVout2 * 4096 /*/ VREF*/;		//Vout = Vref × (DAC12_xDAT/4096)
	Vdac12dat2 = Vdac12dat1 / AVCC;

	DAC12_1DAT = Vdac12dat2 & 0xFFF;
}

//ST TS922IDT (éste es el nuevo en uso)
//Vout		VVA		DAT12_1DAT	G_dB
//2500mV	5V		3103		0
//2350mV	4.7V	2916		0
//2300mV	4.6V	2854		19
//2250mV	4.5V	2792		29
//2200mV	4.4V	2730		34
//2150mV	4.3V	2668		37
//2100mV	4.2V	2606		39
//2050mV	4.1V	2544		40
//2000mV	4V		2482		42
//1750mV	3.5V	2172		45
//1500mV	3V		1861		47
//1250mV	2.5V	1551		49
//1000mV	2V		1241		50
//750mV		1.5V	930			50
//500mV		1V		620			50
//0mV		0V		0			51

//TI LM7332MA
//Vout		VVA		DAT12_1DAT	G_dB
//2500mV	5V		3103		27
//0mV		0V		0			50


