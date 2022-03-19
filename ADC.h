/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: ADC.h											*/
/********************************************************/

#ifndef __ADC_H
#define __ADC_H
#include <xdc/cfg/global.h> //Includes definition of UArg arg0

//#include <ti/sysbios/knl/Task.h>
#define  NO_ALLSAMPLED    (UInt)0x01/*0x00*/
#define  ALLSAMPLED       (UInt)0X02/*0x01*/

//Number of samples to compute average
#define AD_SAMPLES        			64    					//Preguntar: unused?
#define AD_SAMPLES_PRUEBA       	128/*1024*/					//*4096*/
#define AD_SAMPLES_PRUEBA_EXP       7/*10*/ 					//2^10 = 1024 = AD_SAMPLES_PRUEBA

//Cada muestra se multiplica por Vref=2500mV y se divide por 4096 (2^12 = 2^3 * 2^9)
#define AD_SAMPLE2mV_EXP1       3
#define AD_SAMPLE2mV_EXP2       9

#define AD_SAMPLE2mV_EXP2_SAMPLES_PRUEBA_EXP	16		//(AD_SAMPLE2mV_EXP2 + AD_SAMPLES_PRUEBA_EXP)

//En total hay hay que: /1024_vecessample  x2500mV_refADC  /4096_escalonesADC
/*
#define Pin900              0     //   --> signed int [Power (dBm*10)]
#define Pin1800             1     //   --> signed int [Power (dBm*10)]
#define Pin2100             2     //   --> signed int [Power (dBm*10)]
#define Pre900              3     //   --> signed int [Power (dBm*10)]
#define Pre1800             4     //   --> signed int [Power (dBm*10)]
#define Pre2100             5     //   --> signed int [Power (dBm*10)]
#define T1_RF               6     //   --> signed int [Temp (ºC)]
#define T2_RF               7     //   --> signed int [Temp (ºC)]
*/

/*
 * Conversion Constants
 */
#define AD_STEP_uV            (818L)
#define AD_VREF_mV            3000    //(3310L) //3353
#define AD_VREF_PLUS_STEP_mV  (2500L)//(3300L) //3353

void ADC12_Config(void);
void ADC12_ISR(UArg arg0);
void ADC12_RTOS_Init(void);
unsigned short ADC12_Convert_V2T (unsigned short Temp_mV);
//void AD_ConvertToUnits(int wValue, const char bCh);
//float calcularPotencia(int conversion);
//void AD_ConvertToPot(double potencia, const char bCh);

#endif
