/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: UART.c											*/
/********************************************************/
#include <string.h>
#include "UART.h"
#include "HAL.h"
#include "stdlib.h"
#include "PDU.h"
//#include "driverlib.h"

#ifdef	__MSP430F5638
	#include "msp430f5638.h"
#endif
#ifdef	__MSP430F5636
	#include "msp430f5636.h"
#endif

#ifdef __RTOS_ENABLED
	#include <ti/sysbios/BIOS.h>
	#include <ti/sysbios/knl/Task.h>
	#include <ti/sysbios/knl/Event.h>
	#include <xdc/runtime/Error.h>
	#include <xdc/std.h>
	#include <xdc/runtime/System.h>
	#include <xdc/cfg/global.h>
	#include <xdc/runtime/Memory.h>
	#include <xdc/runtime/Types.h>
	#include <ti/sysbios/hal/Timer.h>
	#include <ti/sysbios/knl/Queue.h>
	#include <ti/sysbios/hal/Hwi.h>

	//---------------- TASK & EVENTS HANDLERS-----------------
	Task_Handle Taskhdl_RxTimeout;
#endif

#ifdef __FULL_DUPLEX
	#include <ti/sysbios/knl/Queue.h>
	Queue_Handle myQ;
#endif

#ifdef __WDT_ENABLED
	extern unsigned char signatureWDT;
#endif

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
char _UART_RxBuffer[UART_RX_SIZE_MAX/*+1*/]/* = {0}*/;		//In CHARs //TODO: cambiar UART_RX_SIZE
char _UART_TxBuffer[UART_TX_SIZE_MAX/*+1*/] = {0};			//In CHARs
char _UART_RxPtr = 0;
char _UART_TxPtr = 0;
char _UART_TxIsFree = true;
char bRx;
//extern char USB_Rx;
char bRxpre = 0;							//To store the previous received byte
char UART_RXSIZE_variable/* = 0*/;			//To store RX frame length ('Data' field has variable length)
char UART_TXSIZE_variable = 0;				//To store TX frame length ('Data' field has variable length)
char UARTlength_data;
char UARTFrameType = 0;
char Flag_UART_DataReceived = 0;


void UART_Config (void)
{
  //OJO!! CAMBIAR SI SE CAMBIA MCLK/SMCLK !!!!
  //CON BRCLK = 8MHz y BR=57600 -> Uart_Kbps = 138, UCBRSx = 7 (Reg UCA1MCTL)
  //CON BRCLK = 24MHz y BR=57600 -> Uart_Kbps = 416, UCBRSx = 5 (Reg UCA1MCTL)

  unsigned int Uart_Kbps;
  //unsigned int Ucbrsx;
  //Uart_Kbps = (MAIN_XTAL/8)/__UART0_KBPS;
  Uart_Kbps = 416/*MCLK / UART_A1_KBPS*/ /*138*/ /*(8000000/16)/57600*/;		//VER TABLA 34-4 DATASHEET FAMILIA MICRO!!

  UCA1CTL1 |= UCSWRST;  					// SW Reset for UART configuration
  P8SEL |= 0x0C;                            // Select P8.2 and P8.3 = USART0 option select
  UCA1CTL0 &= ~UC7BIT;				// 8-bit character
  UCA1CTL1 |= UCSSEL_2;   			// UCLK = SMCLK
  UCA1BR0 = (char)Uart_Kbps;		// (7.3728MHz/8)/9600 = 96 (0x0060) -> 57600Kbps = 16 (0x0010)
  UCA1BR1 = (char)(Uart_Kbps>>8);
  UCA1MCTL = 0x00;                	// modulation
  UCA1CTL0 |= UCPEN;      		 	// Paridad
  UCA1CTL0 |= UCPAR;        		// Par

  //UCA1MCTL |= (0x0E);		//VER TABLA DATASHEET FAMILIA MICRO!! (UCOS16 = 0 | UCBRSx = 7 VER TABLA 34-4 !!)
  //Ucbrsx = round( ((MCLK / UART_A1_KBPS) - int(MCLK / UART_A1_KBPS)) * 8 );
  UCA1MCTL |= 0x0A;

  UCA1CTL1 &= ~UCSWRST;  	// Initialize USART state machine

  UART_ClearRxBuffer();
  UART_ClearTxBuffer();

  RS485_TERMINADORA_POUT |= RS485_TERMINADORA_MASK;	//Set terminadora for being the last device in the 485 bus

  UCA1IE |= UCRXIE;								// Enable USART0 RX interrupt
  UCA1IE &= ~UCTXIE;							// Disable USART0 TX interrupt

  RS485_RXENABLE_POUT &= ~RS485_RXENABLE_MASK;	// Enable Rx Data Enable (ACTIVE LOW!)
  RS485_TXENABLE_POUT &= ~RS485_TXENABLE_MASK;	// Dissable Tx Data Enable(ACTIVE HIGH!)

  UCA1IFG &= ~UCRXIFG;							//Manually clear IFG for UART RX

  #ifdef __FULL_DUPLEX

  	  //Para poder meter _UART_RxBuffer en una Queue hay que meter esa variable en una estructura cuyo primer elemento sea del tipo Queue_Elem. QUEDA SIN HACER DE MOMENTO.
  	  typedef struct Rec
  	  {
  		  Queue_Elem elem;
  		  char _UART_RxBuffer[UART_RX_SIZE_MAX/*+1*/];
  	  } Rec;

  	  myQ = Queue_create(NULL, NULL);		//Queue for multi RxBuffer
  #endif

}


void UART_RTOS_Init(void)
{
	Event_post(Event_BufferRxTimeout, NONE_RXTIMEOUT);		//comento Sparta

	//---------- Task Rx timeout ---------- 				//Comento Sparta
	Task_Params taskParams;
	Task_Params_init(&taskParams);
	taskParams.priority = 6/*20*/;				//Lowest priority: 0 , Highest priorit: 31
	taskParams.stackSize = /*Sparta 240*//*bien 320*/240;		 	//Task stack size in MAUs (MINIMUM ADDRESSABLE UNIT !)
	Taskhdl_RxTimeout = Task_create (UART_RTOS_TaskRxTimeout, &taskParams, NULL/*&eb*/);
	if (Taskhdl_RxTimeout == NULL)
		System_abort("Task 'taskhdl_RxTimeout' create failed");
}



//---------------------------------------- FUCNTION METHODS ----------------------------------------
void UART_RTOS_TaskRxTimeout(UArg arg0 , UArg arg1/*void *p*/)
{
#ifdef __RS485_ENABLED
  UInt posted;
  while (1)
  {
	#ifdef __WDT_ENABLED
		  signatureWDT |= SIGNATURE_TASK_UART_RXTIMEOUT;    // Firmamos
	#endif

	Uint32 timeout = UART_TIMEOUT_MS;		//6s
    posted = Event_pend(Event_BufferRxTimeout, RXTIMEOUT, Event_Id_NONE, timeout/*x1000us*/);

            if (posted == 0)
            {
            	UART_ClearRxBuffer();
            	UCA1IE |= UCRXIE;				//In case something crashes -> enable uart rx
            	System_printf("UART - Timeout expired\n");
                //break;	//CAGADA! NO poner break, sale del while y hasta luego el pend en loop
            }
            if (posted != 0)
            {
            	_NOP();
            }

            int a = 3;	//Let 1 cycle pass
            a = a + 1;	//Let 1 cycle pass
  }
#endif
}

//Hago dissable del transciver 485-232 en cuanto marco una trama como válida
//Subo stack size de UART_RTOS_TaskRxTimeout de 256 a 320
//Subo stack size de Task USB_Idle de 128 a 196
//Quito la ISR=46 Hwi del Task.cfg. Int priority 10 - REPUESTO
//Marco variable en ISR46 y event en Task
//#pragma vector=/*UCA1IV*/USCI_A1_VECTOR
//__interrupt void usart0_rx (void)
void UART_ISR(/*UArg arg0*/)
{
	//Check UART Errors
	if( (UCA1STAT & 0b01110100) != 0x00 )
		_NOP();
		//System_abort("\n UART Error - Check UCA1STAT \n");		//BEWARE, do not call system libs from HWI

	//RX ISR
	if( ((UCA1IFG & UCRXIFG) == UCRXIFG) /*|| (USB_FlagCharRx == 1)*/)	//CAMBIO AQUI
	{
		//UCA1IFG &= ~UCRXIFG;	//Clear rx interrupt flag	//CAMBIO AQUI (borrar flag, no se borra al leer UCA1IFG)
		#ifdef __RS485_ENABLED
			bRx = UCA1RXBUF;
		#endif

		_UART_RxBuffer[_UART_RxPtr] = bRx;

		/*
		//Check if we are in the end of the frame (fin de trama definido como 0x0D 0x0A)	//CAMBIO AQUI
		if( (bRxpre == 0x0D) && (bRx == 0x0A))
			UART_RXSIZE_variable = _UART_RxPtr + 1;				//When the final chars are received we set the frame length (UART_Rxsize_variable está en chars)
		*/

		//Comprobamos si el inicio de la trama es correcto
		if( ((bRx == 0x3A) && (_UART_RxPtr != 0)) /*|| ((bRx == 0x34) && (_UART_RxPtr != 1))*/ )		//CAMBIO AQUI
		{
			UART_ClearRxBuffer();
			memset (_UART_RxBuffer, 0, UART_RX_SIZE_MAX);

			//----CAMBIO 18/05/2016!!
			_UART_RxBuffer[_UART_RxPtr] = bRx;
			_UART_RxPtr++;
			Event_post(Event_BufferRxTimeout, RXTIMEOUT);
			//------------

			return;
		}

		//Extraemos la longitud del campo Data
		if((_UART_RxPtr == 15))
		{
			UARTFrameType = TABLA_ASCIIHEX[_UART_RxBuffer[8]-0x30]<<4 | TABLA_ASCIIHEX[_UART_RxBuffer[9]-0x30];

			if (UARTFrameType == WRITE)
			{
				UARTlength_data = TABLA_ASCIIHEX[_UART_RxBuffer[14]-0x30]<<4 | TABLA_ASCIIHEX[_UART_RxBuffer[15]-0x30];
				UART_RXSIZE_variable = (15 + UARTlength_data + 4) + 1;
			}
			else if ( (UARTFrameType == READ) || (UARTFrameType == COMMAND_ACTION) )
			{
				UARTlength_data = 4;
				UART_RXSIZE_variable = (15 + UARTlength_data + 4) + 1;
			}
		}

		//Comprobamos si el final de la trama es correcto
		if ( /*(bRxpre == 0x0D) &&*/ /*(_UART_RxPtr != UART_RXSIZE_variable-1-1)) ||*/ ((bRx == 0x0A) && (_UART_RxPtr != UART_RXSIZE_variable-1 )) )
		{
			UART_ClearRxBuffer();
			memset (_UART_RxBuffer, 0, UART_RX_SIZE_MAX);
			return;
		}

		//Nico: Si la trama llega bien
		if (_UART_RxPtr != NULL)
		{
			//Nico: if (_UART_RxPtr == UART_Rxsize_variable - 1)
			//if ((_UART_RxPtr == UART_RXSIZE_variable-1) && ((_UART_RxBuffer[0] == 0x3A) && (_UART_RxBuffer[1] == 0x34) && (_UART_RxBuffer[_UART_RxPtr-1] == 0x0D) && (_UART_RxBuffer[_UART_RxPtr] == 0x0A)))
			if ( (_UART_RxPtr == 15 + UARTlength_data + 4) && ( (_UART_RxBuffer[0] == 0x3A) && (_UART_RxBuffer[1] == 0x34 || _UART_RxBuffer[1] == 0x35) && (_UART_RxBuffer[_UART_RxPtr-1] == 0x0D) && (_UART_RxBuffer[_UART_RxPtr] == 0x0A)))
			{
				#ifdef __FULL_DUPLEX
			    	Queue_enqueue/*Queue_put*/(myQ, &_UART_RxBuffer);	//Enqueue Received buffer frame
				#endif
				#ifndef __FULL_DUPLEX
					UCA1IE &= ~UCRXIE;								//Dissable interrupción de Recepción
				#endif

				UCA1IFG &= ~UCRXIFG;								//Manually clear IFG for UART RX
				RS485_RXENABLE_POUT |= RS485_RXENABLE_MASK;			//Disable Rx Transciver (ACTIVE LOW!)

				//LED_3_POUT |= LED_3_MASK;
				//__delay_cycles(2000);
				//LED_3_POUT &= ~LED_3_MASK;

				Event_post(Event_PDUReceived, RS485_RECEIVE);		//Marcamos una variable en vez de un evento. El evento lo marcamos en un Task
			    //Flag_UART_DataReceived = 1;

			    //UART_ClearRxBuffer();			//OJO! Cuidado aqui con la prioridad de la Task que llama a Interpreta_PDU. No borrar buffers de Rx antes de que se ejecute esa tarea
			    return;
			}
		}

		_UART_RxPtr++;
		//bRxpre = bRx;

		if (_UART_RxPtr >= UART_RX_SIZE_MAX)
			UART_ClearRxBuffer();

		//-->ctl_events_set_clear(&Buffer_Rx_Timeout_Event, RXTIMEOUT_EVENT, 0);
		Event_post(Event_BufferRxTimeout, RXTIMEOUT);

		UCA1IFG &= ~UCRXIFG;			//Manually clear IFG for UART RX
	}


	//TX ISR
	else if( (UCA1IFG & UCTXIFG) == UCTXIFG)
	{
		//UCA1IFG &= ~UCTXIFG;	//Clear rx interrupt flag
		_UART_TxPtr++;

		if ( (_UART_TxPtr == UART_TXSIZE_variable) )
		{
			//__delay_cycles(MACRO_TIME_MICROS(MACRO_delay_485(__UART0_KBPS)));
			__delay_cycles(10000/*3000000*/);	//Delay to let bits from last char be txed
			RS485_TXENABLE_POUT &= ~RS485_TXENABLE_MASK;	//Deshabilitamos Enable de la transmisión del RS485
			RS485_RXENABLE_POUT &= ~RS485_RXENABLE_MASK; 	//Habilitamos Enable de la recepción del RS485 (ACTIVE LOW!)
			_UART_TxIsFree = true;

			#ifndef __FULL_DUPLEX
				UCA1IE &= ~UCTXIE; 		//Clear interrupción de Tx
			#endif

			UCA1IFG &= ~UCTXIFG; 		//Borramos Flag interrupcion de TX

			#ifndef __FULL_DUPLEX
				UCA1IE |= UCRXIE;  		//Enable interrupción de RX
			#endif

			return;
		}

		UCA1TXBUF = _UART_TxBuffer[_UART_TxPtr]; 			//Enviamos el resto del _UART_TxBuffer ( el primer 8-bit-caracter se envía en UART_TxBuffer(...) )
	}
}

void UART_ClearRxBuffer (void)
{
  //unsigned short i;

  _UART_RxPtr = 0;
  bRxpre = 0;
  UART_RXSIZE_variable = 0;


  //for (i=0 ; i < UART_RX_SIZE_MAX ; i++)	//cambiar por memset
  //	_UART_RxBuffer[i] = 0;

  memset (_UART_RxBuffer, 0, UART_RX_SIZE_MAX);
}

char UART_Send (const char *lpszMessage)
{
  if ( !_UART_TxIsFree )	//Nico Si el buffer está ocupado RETURN
     return false;

  UCA1IE &= ~UCRXIE;								//Deshabilito interrupción de recepción - ACCIÓN DUPLICADA??
  _UART_TxIsFree = false;
  UART_ClearTxBuffer();
  memcpy( _UART_TxBuffer , lpszMessage , UART_TXSIZE_variable );
  RS485_TXENABLE_POUT |= RS485_TXENABLE_MASK; 		//Habilitamos Enable del transceiver del RS485 (ACTIVE HIGH!)
  RS485_RXENABLE_POUT |= RS485_RXENABLE_MASK;		//Disable Rx transceiver (ACTIVE LOW!)
  UCA1TXBUF = _UART_TxBuffer[0];					//Nico: El registro UCA1TXBUF es dónde se colocan los bits a transmitir
  UCA1IE |= UCTXIE;									//Set interrupción de TX
  return true;
}

void UART_ClearTxBuffer (void)
{
  _UART_TxPtr = 0;
  _UART_TxBuffer[0] = '\0';
  memset (_UART_TxBuffer, 0, UART_TX_SIZE_MAX);
}
