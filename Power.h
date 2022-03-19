/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: Power.h										*/
/********************************************************/

#ifndef __POWER_H
#define __POWER_H

void POWER_System_Stop (int HPA);
void POWER_System_Start (int HPA);
int POWER_HPA_Start (int HPA);
void POWER_HPA_Stop (int HPA);
void POWER_ControlledReset(unsigned char);

#endif
