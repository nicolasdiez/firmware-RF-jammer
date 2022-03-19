/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: DAC.h											*/
/********************************************************/


void DAC_Config(void);
void DAC_SetGainControl(unsigned short mVout2);

#define VREF  1.5	//Volts
#define AVCC  3300	//Volts
