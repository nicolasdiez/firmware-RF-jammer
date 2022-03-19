/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: UART.h											*/
/********************************************************/

#ifndef _UART_API_INCLUDE_HEADER_H_
#define _UART_API_INCLUDE_HEADER_H_

#include <xdc/cfg/global.h> //Includes definition of UArg arg0

#define NONE_RXTIMEOUT 		(UInt)0x01
#define RXTIMEOUT/*_EVENT*/ (UInt)0x02

void UART_RTOS_Init(void);
void UART_RTOS_TaskRxTimeout/*Maquina_RxTimeout*/(UArg arg0 , UArg arg1/*void *p*/);
void UART_ClearRxBuffer (void);
void UART_ClearTxBuffer (void);
char UART_Send (const char *lpszMessage);
void UART_ISR(/*UArg arg0*/);
void UART_Config(void);
void Rx_TimeoutInit(void);

#endif // _UART_API_INCLUDE_HEADER_H_
