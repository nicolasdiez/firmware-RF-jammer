/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: September 2014									*/
/* File: I2C.h											*/
/********************************************************/

#ifndef _I2C_INCLUDE_HEADER_H_
#define _I2C_INCLUDE_HEADER_H_

#include <xdc/cfg/global.h>

void I2C_Communication(int GlobalDirCommunication);
void I2C_Config(void);
void I2C_Transmit(void);
void I2C_Receive(void);
void I2C_ISR (/*UArg arg0*/void);
void I2C_CheckBus(void);
int I2C_LoadParamsFromGeneradora(int HPA);
int I2C_IsGeneradoraReady ();
int I2C_StartGeneradora();
int I2C_LoadParamsFromGeneradoraDoubleCheck(int HPA);

//Event_PrevPowerController
#define I2C_TX_FINISHED  		(UInt)0x01
#define I2C_RX_FINISHED  		(UInt)0x02
#define I2C_TX_ERROR			(UInt)0x04
#define I2C_RX_ERROR			(UInt)0x08
#define I2C_COMM_FINISHED		(UInt)0x10

#endif
