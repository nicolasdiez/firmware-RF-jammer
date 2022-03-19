/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: PowerController.h								*/
/********************************************************/

#ifndef POWERCONTROLLER_H_
#define POWERCONTROLLER_H_

#include <xdc/cfg/global.h>

#define POTPREVIO_CONTROLLER_RANGOERROR		7	/*0.7dB*/
#define POTPREVIO_MAXADMITTED_ERROR			25	/*1.5dB*/

void ACCIONES_PowerControllerPD (unsigned int hpa);
int ACCIONES_PowerPrevioControllerPD (unsigned int hpa);
//int ACCIONES_AdjustFwdPowerPrevio(unsigned int hpa , unsigned short PotObjetivo);

//Event_PrevPowerController
#define PREVCONTROLLER_BF_FINISHED  		(UInt)0x01
#define PREVCONTROLLER_BF_RUNNING  			(UInt)0x02
#define PREVCONTROLLER_AF_FINISHED  		(UInt)0x04
#define PREVCONTROLLER_AF_RUNNING  			(UInt)0x08
#define PREVCONTROLLER_BF_POTPREVIO_EXCES	(UInt)0x10
#define PREVCONTROLLER_AF_POTPREVIO_EXCES	(UInt)0x20
#define PREVCONTROLLER_BF_PREVIOERROR		(UInt)0x40
#define PREVCONTROLLER_AF_PREVIOERROR		(UInt)0x80

#endif /* POWERCONTROLLER_H_ */
