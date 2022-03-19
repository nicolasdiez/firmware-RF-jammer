/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: USB.h											*/
/********************************************************/

#include "msp430.h"

#include "driverlib.h"

#include "USB_API/USB_Common/device.h"
#include "USB_config/descriptors.h"

//#include "hal.h"
#include <xdc/cfg/global.h> //Includes definition of UArg arg0

#define NO_TXCHAR  (UInt)0x01
#define TXCHAR     (UInt)0x02

#define USB_FRAME_FINISHED  (UInt)0x01

void initClocks(uint32_t mclkFreq);
void USB_Communication ( UArg arg0, UArg arg1 /*void*/ );
void USB_Config (void);
void USB_Idle(void);
void USB_RTOS_Init(void);
void USB_ClearRxBuffer (void);
void USB_Frame(char USB_Rx);
void USB_RTOS_TaskRxTimeout(UArg arg0 , UArg arg1/*void *p*/);
void USB_Send (const char *USB_Buffer_Tx , unsigned int Length_USB_Buffer_Tx);
void initClocksNico(void);

//#define __ECHO
