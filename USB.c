/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: November 2014									*/
/* File: USB.c											*/
/********************************************************/

/// new includes usb event
#include <string.h>
#include "driverlib.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/defMSP430USB.h"
#include "USB_API/USB_Common/usb.h"      //USB-specific Data Structures
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_API/USB_PHDC_API/UsbPHDC.h"
#include "USB_API/USB_HID_API/UsbHidReq.h"
#include "USB_API/USB_MSC_API/UsbMscScsi.h"
#include "USB_API/USB_Common/UsbIsr.h"
#include <descriptors.h>
///////////////////////////////

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>

#include "USB_API/USB_Common/device.h"         
#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/usb.h"
#include <string.h>

#include <ti/sysbios/knl/Event.h>

#include <xdc/runtime/Log.h>

#include "USB_app/usbConstructs.h"
#include <xdc/runtime/System.h>

#include "msp430.h"
#include "driverlib.h"

#include "USB.h"
#include "HAL.h"
#include "PDU.h"
#include "UART.h"

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
	#include <ti/sysbios/knl/Semaphore.h>
	#include <ti/sysbios/knl/Clock.h>
	//#include <ti/sysbios/hal/Hwi.h>
#endif

#ifdef __WDT_ENABLED
	extern unsigned char signatureWDT;
#endif

/* Semaphore handle defined in task.cfg */
extern const Semaphore_Handle semCdc0;


#define MAX_STR_LENGTH 60
#define BUFFER_SIZE 130

uint16_t count;

//char USB_Rx;
char USBPortdataBuffer[USB_BUFFER_SIZE] = "";
char _USB_RxBuffer[USB_RX_SIZE_MAX]/* = {0}*/;		//In CHARs //TODO: cambiar UART_RX_SIZE
char _USB_RxPtr = 0;
char Flag_USBAPI_DataReceived = 0;

/*extern*/ char bRx;

#ifdef __FULL_DUPLEX
	#include <ti/sysbios/knl/Queue.h>
	extern Queue_Handle myQ;
#endif

#ifdef __RTOS_ENABLED
	void USB_RTOS_Init(void)
	{
	  //------------------- TASK CREATION ----------------------------
	  /* Create task*/
//	  Task_Params taskParams;
//	  Task_Params_init(&taskParams);
//	  taskParams.priority = 3/*25*/;					//Lowest priority: 0 , Highest priorit: 31
//	  taskParams.stackSize = 128/*300*/;		 		//Task stack size in MAUs (MINIMUM ADDRESSABLE UNIT !)
//	  taskhdl_USBCom = Task_create (USB_Communication, &taskParams, NULL/*&eb*/);
//	  if (taskhdl_USBCom == NULL)
//	  	System_abort("Task 'taskhdl_USBCom' create failed");

		Event_post(Event_USBBufferRxTimeout, NONE_RXTIMEOUT);
		//Event_post(Event_USBReceive, NO_RECEIVE);

	}
#endif


void USB_RTOS_TaskRxTimeout(UArg arg0 , UArg arg1/*void *p*/)
{
#ifdef __USB_ENABLED
	  UInt posted;
	  uint16_t wUsbEventMask;

	  while (1)
	  {
		#ifdef __WDT_ENABLED
			  signatureWDT |= SIGNATURE_TASK_USB_RXTIMEOUT;    // Firmamos
		#endif

		Uint32 timeout = USB_TIMEOUT_MS;		//20s
		_NOP();

	    posted = Event_pend (Event_USBBufferRxTimeout, RXTIMEOUT, Event_Id_NONE, timeout);

	            if (posted == 0)
	            {
	            	USB_ClearRxBuffer();

	            	//--------
	            	wUsbEventMask = USB_getEnabledEvents();
	            	wUsbEventMask |= kUSB_dataReceivedEvent;
	            	USB_setEnabledEvents (wUsbEventMask);
	            	//---------

	            	Event_post(Event_USB_Port, USB_FRAME_FINISHED);			//Helps to recover from errors
	            	//USB_setEnabledEvents (kUSB_allUsbEvents/*0x01FF*/);	//Quitamos ésto al crear un event USB_FRAME_FINISHED
	            	System_printf("USB Timeout expired - Clearing Rx Buffer \n");
	            }
	            if (posted != 0)
	            	_NOP();
	  }
#endif
}

void USB_Config (void)
{
	//WDT_A_hold(WDT_A_BASE);

    //------ Modified by Nico
    //USBKEYPID   =     0x9628;
    //USBPWRCTL = 0;
    //------------------------

    // Minimum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .
	#ifndef DRIVERLIB_LEGACY_MODE
		PMM_setVCore(PMM_CORE_LEVEL_2);
	#else
		PMM_setVCore(PMM_BASE, PMM_CORE_LEVEL_2);
	#endif

    initClocks(24000000/*8000000*/);   		// Config clocks. MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
    //initClocksNico();						//Cambio Sparta ---> SACO ESTA INIT AL MAIN
    USB_setup(TRUE, TRUE); 					// Init USB & events; if a host is present, connect

    USB_ClearRxBuffer();

    //__enable_interrupt();  			// Enable interrupts globally
}

void USB_Communication ( UArg arg0, UArg arg1 /*void*/ )
{
	int i = 0;
	char USB_Rx;
	//uint16_t wUsbEventMask;
	//UInt posted;

    while (1)
    {
		//OJO! Esta Task tiene un semáforo, quitamos el watchdog aquí

    	_NOP();

    	//Event USB_FRAME_FINISHED para no evaluar el semaforo de rx hasta terminar el frame actual
		/*posted = */Event_pend(Event_USB_Port, USB_FRAME_FINISHED, Event_Id_NONE/*Event_Id_NONE*/, BIOS_WAIT_FOREVER/*ms*/);

    	Semaphore_pend (semCdc0, BIOS_WAIT_FOREVER);		//posted in Acciones2.c by Flag_USBAPI_DataReceived=1

    	Flag_USBAPI_DataReceived = 0;					//Set it to 0 to avoid sem posting from Task ACCIONES_RTOS_MarcarBFAF

    	//----------------------------------------------------
		#ifdef __USB_ENABLED
    	count = cdcReceiveDataInBuffer((uint8_t*)USBPortdataBuffer, USB_BUFFER_SIZE/*1*/, CDC0_INTFNUM);
		#endif

        for(i=0 ; i<count ; i++)
        {
            USB_Rx = USBPortdataBuffer[i];
            USB_Frame(USB_Rx);

			#ifdef __USB_ECHO
			// Echo back to the host.
			if (cdcSendDataInBackground((uint8_t*)dataBuffer, count, CDC0_INTFNUM, 1))
			{
				// Exit if something went wrong.
				break;
			}
			#endif
        }
    }
}


//extern char UART_RXSIZE_variable;
char USB_RXSIZE_variable;
char USBlength_data = 0;
char USBFrameType = 0;

void USB_Frame(char USB_Rx)
{
	uint16_t wUsbEventMask;
	/*char*/ bRx = USB_Rx;
	_USB_RxBuffer[_USB_RxPtr] = bRx;

	//Comprobamos si el inicio de la trama es correcto
	if( ((bRx == 0x3A) && (_USB_RxPtr != 0)) /*|| ((bRx == 0x34) && (_UART_RxPtr != 1))*/ )		//CAMBIO AQUI
	{
		USB_ClearRxBuffer();
		memset (_USB_RxBuffer, 0, USB_RX_SIZE_MAX);
		return;
	}

	//Extraemos la longitud del campo Data
	if((_USB_RxPtr == 15))
	{
		USBFrameType = TABLA_ASCIIHEX[_USB_RxBuffer[8]-0x30]<<4 | TABLA_ASCIIHEX[_USB_RxBuffer[9]-0x30];

		if (USBFrameType == WRITE)
		{
			USBlength_data = TABLA_ASCIIHEX[_USB_RxBuffer[14]-0x30]<<4 | TABLA_ASCIIHEX[_USB_RxBuffer[15]-0x30];
			USB_RXSIZE_variable = (15 + USBlength_data + 4) + 1;
		}
		else if ( (USBFrameType == READ) || (USBFrameType == COMMAND_ACTION) )
		{
			USBlength_data = 4;
			USB_RXSIZE_variable = (15 + USBlength_data + 4) + 1;
		}
	}

	//Comprobamos si el final de la trama es correcto
	if ( /*(bRxpre == 0x0D) &&*/ /*(_UART_RxPtr != USB_RXSIZE_variable-1-1)) ||*/ ((bRx == 0x0A) && (_USB_RxPtr != USB_RXSIZE_variable-1 )) )
	{
		USB_ClearRxBuffer();
		memset (_USB_RxBuffer, 0, USB_RX_SIZE_MAX);
		return;
	}

	//Nico: Si la trama llega bien
	if (_USB_RxPtr != NULL)
	{
		//Nico: if (_UART_RxPtr == USB_RXSIZE_variable - 1)
		//if ((_UART_RxPtr == USB_RXSIZE_variable-1) && ((_UART_RxBuffer[0] == 0x3A) && (_UART_RxBuffer[1] == 0x34) && (_UART_RxBuffer[_UART_RxPtr-1] == 0x0D) && (_UART_RxBuffer[_UART_RxPtr] == 0x0A)))
		if ( (_USB_RxPtr == 15 + USBlength_data + 4) && ( (_USB_RxBuffer[0] == 0x3A) && (_USB_RxBuffer[1] == 0x35) && (_USB_RxBuffer[_USB_RxPtr-1] == 0x0D) && (_USB_RxBuffer[_USB_RxPtr] == 0x0A)))		//CAMBIO AQUI
		{
			#ifdef __FULL_DUPLEX
		    	Queue_enqueue/*Queue_put*/(myQ, &_USB_RxBuffer);	//Enqueue Received buffer frame
			#endif
			#ifndef __FULL_DUPLEX
				// /*IE1*/UCA1IE &= ~UCRXIE/*URXIE0*/;					//Clear interrupción de Recepción
			#endif

		    //-----------
		    //Disable USB reception
		    wUsbEventMask = USB_getEnabledEvents();
		    wUsbEventMask &= ~kUSB_dataReceivedEvent;
		    USB_setEnabledEvents (wUsbEventMask);
		    //-----------

		    Event_post(Event_PDUReceived, USB_RECEIVE);
		    //UART_ClearRxBuffer();						//OJO! Cuidado aqui con la prioridad de la Task que llama a Interpreta_PDU. No borrar buffers de Rx antes de que se ejecute esa tarea

		    return;
		}
	}

	_USB_RxPtr++;
	/*bRxpre = bRx;*/

	if (_USB_RxPtr >= USB_RX_SIZE_MAX)
		USB_ClearRxBuffer();

	Event_post(Event_USBBufferRxTimeout, RXTIMEOUT);
}


void USB_Send (const char *USB_Buffer_Tx , unsigned int Length_USBBufferTx)
{
	uint8_t SendError = 0;
	SendError = cdcSendDataInBackground( (uint8_t*)USB_Buffer_Tx , Length_USBBufferTx/*strlen(USB_Buffer_Tx)*/ , CDC0_INTFNUM , 1);

	//USB_setEnabledEvents (kUSB_allUsbEvents/*0x01FF*/);

	if( SendError == 1)
	{
	    //System_abort("USB Tx Error");		//Este Log SI sale por 'Console'
	    System_printf("USB Tx Error - Timeout \n");
	    //Log_warning2("tsk0 demonstrating warning event. arg0,1 = %d %d", (Int)arg0, (Int)arg1);
	    //break;
	}
	else if(SendError == 2)
	{
	    System_printf("USB Tx Error - Bus is gone \n");
	    //break;
	}
	else if(SendError == 0)
	{
	    //Log_info1("USB Sending Frame = %d \n", (Int)USB_Buffer_Tx);
	    System_printf("USB Sending Frame \n");
	}
}

void USB_ClearRxBuffer (void)
{
  unsigned short i;

  _USB_RxPtr = 0;
  /*bRxpre = 0;*/
  USB_RXSIZE_variable = 0;
  //USB_FlagClearBuffer = 1;

  for (i=0 ; i < USB_RX_SIZE_MAX ; i++)
	  _USB_RxBuffer[i] = 0;
}

void USB_Idle (void)
{
    while (1)
    {
		#ifdef __WDT_ENABLED
		  //signatureWDT |= SIGNATURE_TASK_USBIDLE;		//Esta Task entra en modo LPM0 o tiene while(1) internos, quitamos el watchdog aquí
		#endif

        // Check the USB state and directly main loop accordingly
        switch (USB_connectionState())
        {
            // This case is executed while your device is enumerated on the USB host
            case ST_ENUM_ACTIVE:

                // Sleep if there are no bytes to process.
//              __disable_interrupt();
//              if (!USBCDC_bytesInUSBBuffer(CDC0_INTFNUM))
//              {
//              	// Enter LPM0 until awakened by an event handler
//                  __bis_SR_register(LPM0_bits + GIE);
//              }
//              __enable_interrupt();

            	while(1)

                //break;

            // These cases are executed while your device is disconnected from
            // the host (meaning, not enumerated); enumerated but suspended
            // by the host, or connected to a powered hub without a USB host
            // present.
            case ST_PHYS_DISCONNECTED:
            	break;
            case ST_ENUM_SUSPENDED:
            	break;
            case ST_PHYS_CONNECTED_NOENUM_SUSP:

            	//_NOP();
            	_NOP();
            	_NOP();
            	//__bis_SR_register(LPM3_bits + GIE);
            	_NOP();

            	while(1)

                //break;

            // The default is executed for the momentary state
            // ST_ENUM_IN_PROGRESS.  Usually, this state only last a few
            // seconds.  Be sure not to enter LPM3 in this state; USB
            // communication is taking place here, and therefore the mode must
            // be LPM0 or active-CPU.
            case ST_ENUM_IN_PROGRESS:
            	break;

            default:;
        }
    }
}


void initClocks(uint32_t mclkFreq)
{
#ifndef DRIVERLIB_LEGACY_MODE
	//Inicializa FLL con REFOCLK
	UCS_clockSignalInit(
	   UCS_FLLREF,
	   UCS_REFOCLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

//	do
//	{
//		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);	// Clear XT2,XT1,DCO fault flags
//		SFRIFG1 &= ~OFIFG;                    		// Clear fault flags
//	}while (SFRIFG1 & OFIFG);

	//Inicializa ACLK con REFOCLK (OJO! REFOCLK es interno y fijo y es 32768Hz)
	UCS_clockSignalInit(
	   UCS_ACLK,
	   UCS_REFOCLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

//	do
//	{
//		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);	// Clear XT2,XT1,DCO fault flags
//		SFRIFG1 &= ~OFIFG;                    		// Clear fault flags
//	}while (SFRIFG1 & OFIFG);

	//Inicializa MCLK con mclkFreq
    UCS_initFLLSettle(
        mclkFreq/1000,
        mclkFreq/32768);

//	UCSCTL6 |= XT2DRIVE_3;				//XT2 max drive capability
//	UCSCTL6	&= ~XT2BYPASS;				//XT2 sourced from external xtal
//	UCSCTL5 |= XT2OFF;					//XT2 Off
//
//	UCSCTL6 |= XT1DRIVE_3;				//XT1 max drive capability
//	UCSCTL6	&= ~XTS;					//XT1 Low Freq Mode
//	UCSCTL6	&= ~XT1BYPASS;				//XT1 sourced from external xtal
//	UCSCTL5 |= XT1OFF;					//XT1 Off

//	do
//	{
//		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);	// Clear XT2,XT1,DCO fault flags
//		SFRIFG1 &= ~OFIFG;                    		// Clear fault flags
//	}while (SFRIFG1 & OFIFG);

#else
    UCS_clockSignalInit(
       UCS_BASE,
	   UCS_FLLREF,
	   UCS_REFOCLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

	UCS_clockSignalInit(
       UCS_BASE,
	   UCS_ACLK,
	   UCS_REFOCLK_SELECT,
	   UCS_CLOCK_DIVIDER_1);

    UCS_initFLLSettle(
        UCS_BASE,
        mclkFreq/1000,
        mclkFreq/32768);

#endif

}


