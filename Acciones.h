/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: Acciones.h										*/
/********************************************************/

#ifndef _ACCIONES_API_INCLUDE_HEADER_H_
#define _ACCIONES_API_INCLUDE_HEADER_H_
#include <xdc/cfg/global.h>


void ACCIONES_RTOS_Init(void);
void ACCIONES_RTOS_Task(UArg arg0, UArg arg1/*void *p*/);

void ACCIONES_MarcarAlarmas(void);
void ACCIONES_Actuar (int hpa);
void ACCIONES_AdjustFwdPower(unsigned int hpa);
void ACCIONES_MarcarEstadoOperativo (void);
void ACCIONES_MarcarAlarmas_PotSWRT (unsigned int hpa);
int ACCIONES_LoadParamsFromGeneradora();
void ACCIONES_SetEstadoOperativo(unsigned short Status);
void ACCIONES_ClearEstadoOperativo(unsigned short Status);




void RegInit(int i);

#endif
