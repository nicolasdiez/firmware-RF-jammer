/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: Novemeber 2015									*/
/* File: Alarms.h										*/
/********************************************************/

#ifndef ALARMS_H_
#define ALARMS_H_

#include <xdc/cfg/global.h>

void ALARMS_MarcarEstadoOperativo (void);
void ALARMS_MarcarAlarmas_Pot_SWR_Temp_Volt (unsigned int hpa);
void ALARMS_MarcarBFAF(UArg arg0, UArg arg1);


#endif
