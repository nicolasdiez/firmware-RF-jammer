/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: Acciones2.h									*/
/********************************************************/

#ifndef _ACCIONES_API_INCLUDE_HEADER_H_
#define _ACCIONES_API_INCLUDE_HEADER_H_
#include <xdc/cfg/global.h>

void InitSystem();

void ACCIONES_Regs_Init(void);
//void ACCIONES_RTOS_Task(UArg arg0, UArg arg1/*void *p*/);
//void ACCIONES_MarcarAlarmas(void);
//void ACCIONES_Actuar (int hpa);
int ACCIONES_AdjustFwdPowerPrevio(unsigned int hpa , unsigned short PotObjetivo);
void ACCIONES_SetEstadoOperativo(unsigned short Status);
void ACCIONES_ClearEstadoOperativo(unsigned short Status);
void ACCIONES_RTOS_ActuarBF(UArg arg0, UArg arg1);
void ACCIONES_InitRegisters (void);
int ACCIONES_GetCardType (void);
void ACCIONES_CompoundRS485Address(void);


void RegInit(int i);

#endif
