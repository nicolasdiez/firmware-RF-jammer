/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: RS485.h										*/
/********************************************************/

#ifndef _RS485_API_INCLUDE_HEADER_H_
#define _RS485_API_INCLUDE_HEADER_H_
#include <xdc/cfg/global.h>

void PDU_RTOS_Init(void);
void PDU_RTOS_Task(UArg arg0, UArg arg1/*void *p*/);
void PDU_Interpreta(char UART_RXSIZE_variable2/*void*/);
void PDU_Send(PDU_RS485 *lp);


#define NO_RECEIVE  		(UInt)0x01
#define RECEIVE     		(UInt)0x02
#define RS485_RECEIVE     	(UInt)0x04
#define USB_RECEIVE     	(UInt)0x08

//void I2C_Communication(int W_R);
#endif
