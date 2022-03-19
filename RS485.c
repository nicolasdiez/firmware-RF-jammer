/********************************************************/
/* Author: Nicolás Díez									*/
/* Date: July 2014										*/
/* File: RS485.c										*/
/********************************************************/

//#include <ctl_api.h>
//#include <ctl.h>  //<--- La incluye Nico, sino falla compilador
//#include <cross_studio_io.h>
#include <string.h>
#include <stdlib.h>
#include "HAL.h"
#include "RS485.h"
#include "uart.h"
#include "Power.h"
#include "USB_test.h"
#include "I2C.h"
#include "Acciones2.h"
#include "FlashMem.h"
//#include "EstadoOperativo.h"
//#include "ATT.h"
//#include "FlashConfig.h"
//#include "I2C.h"


#ifdef __DEBUG
	#include "debug.h"
#endif

#ifdef __WDT_ENABLED
	extern unsigned char signatureWDT;
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
	#include <ti/sysbios/knl/Semaphore.h>
	#include <ti/sysbios/knl/Clock.h>
	//#include <ti/sysbios/hal/Hwi.h>

	//---------------- TASK & EVENTS HANDLERS-----------------
	Task_Handle tskhdl_RS485StatMach;
	//Event_Handle Event_RS485Receive;
	//Event_Handle Event_USBReceive;
	//Event_Handle Event_EscrituraRegs;
#endif

#ifdef __FULL_DUPLEX
	#include <ti/sysbios/knl/Queue.h>
	extern Queue_Handle myQ;
#endif

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
extern char _UART_RxBuffer[/*UART_RX_SIZE_MAX*/];
char _UART_TxBuffer[UART_TX_SIZE_MAX] = {0};		//In CHARS
extern char UART_RXSIZE_variable;
extern char UART_TXSIZE_variable;
PDU_RS485 pdu_read;
char USB_Buffer_Tx[UART_TX_SIZE_MAX] = {0};
extern char USB_RXSIZE_variable;

extern char _USB_RxBuffer[UART_RX_SIZE_MAX/*+1*/]/* = {0}*/;

PDU_I2C pdu_I2C;

char MaskEnclavamiento = 0;		//TODO: Hacer extern para usarla fuera
char MaskBorrarAlarmas = 0;
int toI2C = 0;
char Command_Action;
extern char length_data;
unsigned int Flag_I2C_Comm_Completed = 0;		//CHAPUCILLA? Poner quizá un Mutex o un semáforo, preguntar
extern int USB_FlagCharTx;
extern char Flag_System_Stop[2];
extern char Flag_HPA_Stop[2];

extern union register_entry ArrayTControl[TC_NUM_REGS];				//Nico: Cada slot de estos [] ocupa 2bytes
extern union register_entry ArrayGen[GEN_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes

//extern long EstadoNuevo;
//extern long EstadoNuevo[3];

//extern unsigned long MaskAlarm;
unsigned char LedBusON = 0;

//ESTAS LINEAS SON PARA DEPURAR
signed long direccion = 0;
signed long framenumber = 0;
signed long registro = 0;
signed long comando = 0;
signed long datos = 0;

//-->CTL_TASK_t RS485_StatMachTask;
//-->extern CTL_EVENT_SET_t EventRS485_Receive, Escritura;
//unsigned RS485_StatMachStack[1+RS485_StatMachSTACKSIZE+1];


//extern union registro ArrayTControl[0x20];
//char ADD_DEVICE;

//////////////////////////////
/*//Main status structure
typedef struct
{
  char StandBy, ONreciente;
  union register_entry ArrayTControl[0x106F];
  char ATT;
  char ATTTEMPCOUNTER;
  char WarSWR, ErrorSWR, OKSWR, WarPII, ErrorPII, OKPII, WarTEMP, ErrorTEMP, OKTEMP, ErrorATETEMP;
  char CriticalSWR, CriticalTEMP;
  char MaxATETEMP;
  long TEMPanterior;
}parametros;*/
//extern parametros param[3];
//////////////////////////////

//---------------------------------------- FUCNTION METHODS ----------------------------------------
#ifdef __RTOS_ENABLED
	void PDU_RTOS_Init(void)
	{
	  //------------------- EVENT RS485 RECEIVED ----------------------------
	  Error_Block eb;
	  Error_init(&eb);
//	  Event_Params eventParams;
//	  Event_Params_init(&eventParams);
//	  Event_RS485Receive = Event_create(&eventParams, /*NULL*/&eb);

	  //if ( /*!*/(Event_RS485Receive = Event_create(NULL, &eb)) )
	  //	  System_abort("Event RS485Receive create failed");

	  Event_post(Event_PDUReceived, NO_RECEIVE);
	  Event_post(Event_USBTxChar, NO_TXCHAR);

	  //------------------- EVENT ESCRITURA EN REGS ----------------------------
	  /*if ( !(Event_EscrituraRegs = Event_create(NULL, &eb)) )
		  System_abort("Event EscrituraRegs create failed");
	  Event_post(Event_EscrituraRegs, NO_ESCRITURA);*/

	  //------------------- TASK CREATION ----------------------------
	  /* Create task*/
	  Task_Params taskParams;
	  Task_Params_init(&taskParams);
	  taskParams.priority = 4/*25*/;					//Lowest priority: 0 , Highest priorit: 31
	  taskParams.stackSize = /*256*/512/*384*/;		 		//Task stack size in MAUs (MINIMUM ADDRESSABLE UNIT !)
	  tskhdl_RS485StatMach = Task_create (PDU_RTOS_Task, &taskParams, &eb);
	  if (tskhdl_RS485StatMach == NULL)
	  	System_abort("Task 'tskhdl_RS485StatMach' create failed");

	  //memset(RS485_StatMachStack, 0xcd, sizeof(RS485_StatMachStack));
	  //RS485_StatMachStack[0]=RS485_StatMachStack[1+RS485_StatMachSTACKSIZE]=0xfacefeed; // put marker values at the words before/after the stack
	  //--> ctl_events_init(&EventRS485_Receive,NO_RECEIVE);
	  //--> ctl_task_run(&RS485_StatMachTask, 25, RS485_StatMach, 0, "ME", RS485_StatMachSTACKSIZE, RS485_StatMachStack+1, CALLSTACKSIZE); // Modem Driver
	}

	void PDU_RTOS_Task(UArg arg0, UArg arg1/*void *p*/)
	{
	  UInt posted;

	  while (1)
	  {
		#ifdef __WDT_ENABLED
		  signatureWDT |= SIGNATURE_TASK_RS485;
		#endif

		//--> if (ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR, &EventRS485_Receive, RECEIVE, 1, ctl_get_current_time()+ MACRO_TIME_MILS(300)) != 0)

		//if (posted = Event_pend(Event_RS485Receive, Event_Id_NONE/*RS485_RECEIVE*/, USB_RECEIVE+RS485_RECEIVE/*Event_Id_NONE*/, 100000/*MACRO_TIME_MILS(300)*/) != 0)
		//{
			posted = Event_pend(Event_PDUReceived, Event_Id_NONE/*RS485_RECEIVE*/, USB_RECEIVE+RS485_RECEIVE/*Event_Id_NONE*/, 100000);

			if (posted == 0)
				System_printf("Timeout expired for Event_pend()\n");

			//RS485_Interpreta_PDU(UART_RXSIZE_variable);

			if (posted & RS485_RECEIVE)
			{
				#ifdef __RS485_ENABLED
					PDU_Interpreta(UART_RXSIZE_variable);
					UART_ClearRxBuffer();			//OJO! Cuidado aqui con la prioridad de la Task que llama a Interpreta_PDU. No borrar buffers de Rx antes de que se ejecute esa tarea
				#endif
			}

			if (posted & USB_RECEIVE)
			{
				#ifdef __USB_ENABLED
					strcpy ( _UART_RxBuffer , _USB_RxBuffer );
					PDU_Interpreta(USB_RXSIZE_variable);
					USB_ClearRxBuffer();
				#endif
			}

		//-->else
		//else
		//  /*IE1*/UCA1IE |= UCRXIE/*URXIE0*/;
	  }
	}
#endif


void PDU_Interpreta(char UART_RXSIZE_variable2/*void*/)
{
  //char UART_RXSIZE_variable2 = UART_RXSIZE_variable;
  char LRC_received;
  char LRC_calculated = 0;
  char j,k,m;
  unsigned short i;
  Flag_I2C_Comm_Completed = 0;


//  #ifdef __USB_ENABLED
//  	  strcpy ( _UART_RxBuffer , _USB_RxBuffer );
//  #endif

  #ifdef __FULL_DUPLEX
  	  char* _UART_RxBuffer;
  	  if (!Queue_empty(myQ))
  		  _UART_RxBuffer = Queue_dequeue/*Queue_get*/(myQ);
  	  else
  		  return;
  #endif

  //Init Data from PDU structs
  for (i=0 ; i<PDU_DATASIZE ; i++)
  {
	  pdu_read.Data[i] = 0x0000;
	  pdu_I2C.Data[i] = 0x00;
  }

  //RS485 Frame - Length of DATA FIELD in CHARS
  //char length_data = UART_RXSIZE_variable - (LENGTH_HEADER + LENGTH_ADDRESS + LENGTH_FRAMENUM + LENGTH_COMMAND + LENGTH_REGISTERS + LENGTH_DATALENGTH + LENGTH_LRC + LENGTH_ENDMARK);

  //Buffer to cointain recived frame in HEX
  //char Buffer_recepcion[ (UART_RX_SIZE_MAX - (LENGTH_HEADER+LENGTH_ENDMARK)) / 2 ];		//In BYTES. Declare this buffer according to the size of the received frame
  char Buffer_recepcion[ 264 ] = {0};

  // Pasamos a hexadecimal el Buffer de recepción (omitimos el Header y el End Mark)
  for (i=0,j=2 ; i < ((UART_RXSIZE_variable2 - (LENGTH_HEADER + LENGTH_ENDMARK))/2)  ; i++,j+=2)
    Buffer_recepcion[i] = TABLA_ASCIIHEX[_UART_RxBuffer[j]-0x30]<<4 | TABLA_ASCIIHEX[_UART_RxBuffer[j+1]-0x30];		//Pasamos los CHARS a HEX

  //Calculamos el LRC con los valores en HEX
  for (i = 0 ; i < ((UART_RXSIZE_variable2 - (LENGTH_HEADER + LENGTH_ENDMARK))/2)-1; i++)
    LRC_calculated = LRC_calculated ^ Buffer_recepcion[i];

  // Cogemos Dirección 485 del destinatario
  pdu_read.Add = (unsigned short)Buffer_recepcion[0]<<8 | (unsigned short)Buffer_recepcion[1];		//2 bytes
  //pdu_read.Add = direccion;			//DEPURANDO


  //Si la dirección de la trama recibida coincide con la cargada en el registro del micro (num serie XX) o es la de broadcast
  if ( (ArrayTControl[REG_NUMSERIE].s_16 == pdu_read.Add) || (pdu_read.Add == 0xFFFF) )
  {
		pdu_read.Frame_Number = Buffer_recepcion[2];															// 1byte
		pdu_read.Command = Buffer_recepcion[3];																	// 1byte
		pdu_read.Register = (unsigned short)Buffer_recepcion[4]<<8 | (unsigned short)Buffer_recepcion[5];		// 2bytes
		pdu_read.Data_Length = Buffer_recepcion[6];			//LONGITUD DEL CAMPO DATA EN CHARS !				// 1byte

		//Caso de que llegue solo 1 byte -> lo meto en Data[0] en el LSByte
		if (pdu_read.Data_Length == 2)
		{
			pdu_read.Data[0] = (unsigned short)0x00 | (unsigned short)Buffer_recepcion[7];	//2bytes
			pdu_read.Data_Length = 4;
			LRC_received = Buffer_recepcion[8];
		}
		else if (pdu_read.Data_Length > 2  &&  pdu_read.Data_Length <= 256)
		{
			for(k=0 , m=7 ; k < pdu_read.Data_Length/4 ; k++ , m=m+2)
			{
				pdu_read.Data[k] = (unsigned short)Buffer_recepcion[m]<<8 | (unsigned short)Buffer_recepcion[m+1];	//2bytes
			}
			LRC_received = Buffer_recepcion[7 + pdu_read.Data_Length/2];
		}

		/*if (pdu_read.Data_Length == 2)	//Nico: fuerzo a que Data ocupe 2 bytes mín para poder ponerlo en un unsigned short siempre
			pdu_read.Data_Length = 4;

		for(k=0 , m=7 ; k < pdu_read.Data_Length/4 ; k++ , m=m+2)
		{
			pdu_read.Data[k] = (unsigned short)Buffer_recepcion[m]<<8 | (unsigned short)Buffer_recepcion[m+1];	//2bytes
		}*/

		//Marco si la trama va para un dispositivo I2C que cuelga del principal 485
		if( ((pdu_read.Register >> 12 & 0x0F) != 0) /*(pdu_read.Register >= REG_START_GEN && pdu_read.Register <= REG_END_GEN)*/ /*|| ( (pdu_read.Command == COMMAND_ACTION) && (pdu_read.Register == 0x0010) )*/ )
			toI2C = YES;
		else
			toI2C = NO;

		//pdu_read.Frame_Number = framenumber;		//DEPURANDO
		//pdu_read.Command = comando;				//DEPURANDO
		//pdu_read.Register = registro;				//DEPURANDO
		//pdu_read.Data = datos;					//DEPURANDO
		//char command_data;

		if( LRC_calculated == LRC_received )
		{
			 // Enciendo Led Bus y pongo LedBusON a 1 para que desde leds.c se apague al rato
			 //LED1_POUT |= LED1_MASK;
			 //LedBusON = 1;

			 //En el caso de que la trama vaya al dispositivo principal (Tarjeta de Control)
			 if (toI2C == NO)
			 {
				switch (pdu_read.Command)
				{
					case COMMAND_ACTION:

						Command_Action = (char)((pdu_read.Data[0] >> 4) & 0x0F);		//When Command == 0x11 Data is 1bytes, MSByte is 0

						MaskEnclavamiento = pdu_read.Data[0] & 0x0F;		//bits: 0000HIJK (K=0 -> Enclavamiento etapa 1 OFF | K=1 -> Enclavamiento etapa 1 ON)
						MaskBorrarAlarmas = pdu_read.Data[0] & 0x01;		// 0b0000 = Borrar alarmas NO activas / 0b0001 Borrar todas las alarmas

						switch(Command_Action/*[i]*/)
						{
									  ///////////////////////////////////////////////////////////////
									  case ACTION_ENCLAVAMIENTO:
										  if ( ((MaskEnclavamiento & 0x01) == 0x01) && (Flag_HPA_Stop[BF] == 0) )			//SHUT-DOWN HPA BF
											  ACCIONES_System_Stop (BF);
										  else if( ((MaskEnclavamiento & 0x01) == 0x00) && (Flag_HPA_Stop[BF] == 1) )		//START HPA BF
										  	  ACCIONES_System_Start (BF);

										  if ( ((MaskEnclavamiento & 0x02) == 0x02) && (Flag_HPA_Stop[AF] == 0) )			//SHUT-DOWN HPA AF
										  	  ACCIONES_System_Stop (AF);
										   else if( ((MaskEnclavamiento & 0x02) == 0x00) && (Flag_HPA_Stop[AF] == 1))	//START HPA AF
											  ACCIONES_System_Start (AF);

									  break;

									  case ACTION_BITE: 		//No implementado en Platinum
										  break;

									  case ACTION_CAMBIOPERFIL:	//No implementado en Platinum
										  break;

									  case ACTION_BORRARALARMAS:

										  if (MaskBorrarAlarmas == 0)	//Borrar de los registros NO RECONOCIDOS las alarmas que NO estén ACTIVAS
										  {
											  ArrayTControl[REG_TC_ALARMASNORECTC].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECTC].s_16) & ArrayTControl[REG_TC_ALARMASNORECTC].s_16 );
											  ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 );
											  ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 );
										  }
										  else if (MaskBorrarAlarmas == 1)
										  {
											  ArrayTControl[REG_TC_ALARMASNORECTC].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECTC].s_16) & ArrayTControl[REG_TC_ALARMASNORECTC].s_16 );
											  ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 );
											  ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 );

											  /*ArrayTControl[REG_TC_ALARMASNORECTC].s_16 = 0x0000;
											  ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 = 0x0000;
											  ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 = 0x0000;*/
										  }

										  break;

									  case ACTION_BORRAREMERGENCIA:	//No implementado en Platinum
										  break;

									  case ACTION_RESET:
										POWER_ControlledReset(0);
										break;
									  ///////////////////////////////////////////////////////////////
						}

						pdu_read.Add = ArrayTControl[REG_NUMSERIE].s_16;		//Contestamos con la dirección 485 propia
						pdu_read.Data[0] = 0x2BB4;
						pdu_read.Data_Length = 4;		//Data_length in CHARS
						PDU_Send(&pdu_read);
						break;

					case WRITE:

						//------------ Writing Serial Number 0x0045 8bytes for the 1st time
						if( ((pdu_read.Register & 0xFF) == REG_CODIGOMODULO1) && (ArrayTControl[REG_CODIGOMODULO1].s_16 == 0xFFFF) && (ArrayTControl[REG_CODIGOMODULO2].s_16 == 0xFFFF) && (ArrayTControl[REG_MESANYO].s_16 == 0xFFFF) && (ArrayTControl[REG_NUMSERIE].s_16 == 0xFFFF) )
						{
							FLASH_Write((unsigned char *)INFO_SEGMENT_C, &pdu_read.Data[0], pdu_read.Data_Length/2 /*8bytes*/);			//Se escribe el num de serie válido por primera vez (Hay ya 0xFF en ese campo que es lo que se lee de la flash al arrancar)
							memcpy(&ArrayTControl[REG_CODIGOMODULO1]  , (char *)INFO_SEGMENT_C , 8 /*sizeof(UNIT_INFO)*/);
							//memcpy(&ArrayTControl[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);
						}
						else if( ((pdu_read.Register & 0xFF) == REG_CODIGOMODULO1) && ( (ArrayTControl[REG_CODIGOMODULO1].s_16 != 0xFFFF) || (ArrayTControl[REG_CODIGOMODULO2].s_16 != 0xFFFF) || (ArrayTControl[REG_MESANYO].s_16 != 0xFFFF) || (ArrayTControl[REG_NUMSERIE].s_16 != 0xFFFF) ) )
						{
							//ERROR
							pdu_read.Command = 0xFF;
							pdu_read.Data[0] = 0x0000;
							pdu_read.Data_Length = 4;
							PDU_Send(&pdu_read);
							break;
						}
						//------------------------------------------------

						//------------ Writing HW version 0x0046 2bytes for the 1st time
						if( ((pdu_read.Register & 0xFF) == REG_VERSION_HW) && (ArrayTControl[REG_VERSION_HW].s_16 == 0xFFFF) )
						{
							FLASH_Write((unsigned char *)INFO_SEGMENT_B, &pdu_read.Data[0], pdu_read.Data_Length/2 /*2bytes*/);			//Se escribe la version HW por primera vez (Hay ya 0xFF en ese campo que es lo que se lee de la flash al arrancar)
							memcpy(&ArrayTControl[REG_VERSION_HW]  , (char *)INFO_SEGMENT_B , 2 /*sizeof(UNIT_INFO_2)*/);
							//memcpy(&ArrayTControl[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);
						}
						else if( ((pdu_read.Register & 0xFF) == REG_VERSION_HW) && (ArrayTControl[REG_VERSION_HW].s_16 != 0xFFFF) )
						{
							//ERROR
							pdu_read.Command = 0xFF;
							pdu_read.Data[0] = 0x0000;
							pdu_read.Data_Length = 4;
							PDU_Send(&pdu_read);
							break;
						}
						//------------------------------------------------

						//Standard Register Write
						if( ((pdu_read.Register & 0xFF) != REG_CODIGOMODULO1) && ((pdu_read.Register & 0xFF) != REG_VERSION_HW) )
							memcpy(&ArrayTControl[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);		//Escribo los datos que vienen en 'Data' en 'ArrayTControl'

						pdu_read.Add = ArrayTControl[REG_NUMSERIE].s_16;		//Contestamos con la dirección 485 propia
						pdu_read.Data[0] = 0x2BB4;
						pdu_read.Data_Length = 4;		//Data_length in CHARS
						PDU_Send(&pdu_read);
						break;

					case READ:
						if(pdu_read.Data[0] == 0x2BB4)
						{
							memcpy(&pdu_read.Data[0], &ArrayTControl[pdu_read.Register & 0xFF].s_16, pdu_read.Data_Length/2);	//Nico: leo la posición de array (coincide con la direcc de mem de registros) y devuelvo el valor como dato de la pdu

							if (pdu_read.Add == 0xFFFF)	//Si es un broadcast para extraer la dirección 485 (TODO: Siempre contestar con la dirección 485 propia en Add)
							{
								pdu_read.Add = ArrayTControl[REG_NUMSERIE].s_16;
							}

							PDU_Send(&pdu_read);
						}
						break;

					case RESET:
						_DINT();
						WDTCTL = WDTPW;
						while(1);
						break;
				}
			 }

			 //En el caso de que la trama vaya a un dispositivo I2C
			 else if(toI2C == YES)
			 {
				switch (pdu_read.Command)
				{
					case COMMAND_ACTION:
						//Poner en DevSel = 0x10 (dirección de 7bits)
						//Poner En RegAddI2C = LSByte de Register485 (dirección del registro en el dispositivo I2C)
						//Poner en DataI2C = pdu_read.Data (código de acción)

						//Poner en Add485 = dirección propia 485
						//Poner en Register485 = mismo Register485 recibido
						//Poner en Comando485 =  mismo Command485 recibido
						//Poner en Data485 = 0x2BB4

						Command_Action = pdu_read.Data[0] & 0xFF;					//Cojo el campo Comando/Acción del LSByte de Data[0]

						pdu_I2C.DevSel = (pdu_read.Register >> 12) & 0x0F;			//I2C address of 'Generadora' (0x01)
						//pdu_I2C.RegAdd = 0x00D0;									//                                                           Memory slot of the I2C device for Comando/Acción operations
						pdu_I2C.Data_Length = 1/*(pdu_read.Data_Length) / 2*/;		//Data length of the Data to be written in BYTES
						pdu_I2C.Data[0] = Command_Action;

						#ifndef __SIMULATEGEN_ENABLED
							I2C_Communication(I2C_WRITE);
							while(!Flag_I2C_Comm_Completed);
						#endif

						memcpy(&ArrayGen[pdu_read.Register & 0xFF/*REG_GEN_COMMAND*/], &pdu_read.Data[0], pdu_read.Data_Length/2);	//Actualizo el array local de la RAM del MCU con los valores de la Generadora
						//ArrayGen[REG_GEN_COMMAND] = (signed short)pdu_read.Data[0];

						pdu_read.Data_Length = 4;									//Data_length in CHARS
						pdu_read.Data[0] = 0x2BB4;
						PDU_Send(&pdu_read);
						break;

					case WRITE:
						//Poner en DevSel = 0x10 (dirección de 7bits)
						//Poner En RegAddI2C = LSByte  de Register485
						//Poner en DataI2C = pdu_read.Data (datos que se van a escribir)

						//Poner en Add485 = dirección propia 485
						//Poner en Data485 = 0x2BB4
						//Poner en Register485 = mismo Register485 recibido
						//Poner en Comando485 =  mismo Command485 recibido

						pdu_I2C.DevSel = (pdu_read.Register >> 12) & 0x0F;			//I2C address of 'Generadora' (0x01)
						//pdu_I2C.RegAdd = pdu_read.Register & 0x0FFF;				//Memory slot of the I2C device to be written (0x0 + 3 nibbles !!)
						pdu_I2C.Data_Length = (pdu_read.Data_Length) / 2;			//Data length of the Data to be written IN BYTES

						for(k=0 , m=0 ; k < pdu_read.Data_Length/2 ; k=k+2 , m++)	//Copy Data to the I2C PDU in bytes
						{
							pdu_I2C.Data[k] = (pdu_read.Data[m]>>8) & 0xFF;	//CAMBIO AQUI
							pdu_I2C.Data[k + 1] = pdu_read.Data[m] & 0xFF;	//CAMBIO AQUI
						}

						#ifndef __SIMULATEGEN_ENABLED
							I2C_Communication(I2C_WRITE);
							while(!Flag_I2C_Comm_Completed);
						#endif

						memcpy(&ArrayGen[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);	//Actualizo el array local de la RAM del MCU con los valores de la Generadora

						pdu_read.Data_Length = 4;									//Data_length in CHARS
						pdu_read.Data[0] = 0x2BB4;
						PDU_Send(&pdu_read);
						break;

					case READ:
						//Poner en DevSel = 0x10 (dirección de 7bits)
						//Poner En RegAddI2C = LSByte de Register485
						//Poner en DataI2C = 0x2BB4

						//Poner en Add485 = dirección propia 485
						//Poner en Data485 = valor del registro solicitado
						//Poner en Register485 = mismo Register485 recibido
						//Poner en Comando485 =  mismo Command485 recibido

						pdu_I2C.DevSel = (pdu_read.Register >> 12) & 0x0F;			//I2C address of 'Generadora' (0x01)
						//pdu_I2C.RegAdd = pdu_read.Register & 0x0FFF;				//Memory slot of the I2C device to be read (0x0 + 3 nibbles !!)
						pdu_I2C.Data_Length = (pdu_read.Data_Length) / 2;			//Data length of the Data to be read IN BYTES


						#ifndef __SIMULATEGEN_ENABLED
							I2C_Communication(I2C_READ);
							while(!Flag_I2C_Comm_Completed);

							//UNCOMMENT THIS FOR REAL FUNCTIONING!!!:
							for(k=0 , m=0 ; k < pdu_read.Data_Length/2 ; k=k+2 , m++)	//Copy Data from PDU_I2C to PDU_485
							{
								pdu_read.Data[m] = (unsigned short)(pdu_I2C.Data[k]<<8) | (unsigned short)pdu_I2C.Data[k+1];
							}
							memcpy(&ArrayGen[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);	//Actualizo el array local de la RAM del MCU con los valores de la Generadora
						#endif

						#ifdef __SIMULATEGEN_ENABLED
							//DEBUG!!: READ FROM ARRAYGEN, NOT FROM ACTUAL GENERADORA
							memcpy(&pdu_read.Data[0], &ArrayGen[pdu_read.Register & 0xFF], pdu_read.Data_Length/2);
						#endif

						PDU_Send(&pdu_read);
						break;

					case RESET:
						_DINT();
						WDTCTL = WDTPW;
						while(1);
						break;
				}
			 }
		}

		//El LRC del frame recibido no está bien
		else //case FRAMEERROR
		{
		  pdu_read.Command = 0xFF;
		  pdu_read.Data[0] = 0x0000;		//NICO TODO: Codificar aquí el tipo de error
		  pdu_read.Data_Length = 4;			//Data_length in CHARS
		  PDU_Send(&pdu_read);
		}
  }
}




void PDU_Send (PDU_RS485 *lp)
{
  //char Buffer[UART_TX_SIZE_MAX] = {0};		//In CHARS
  char i,j;
  //char *ref;
  char LRC;
  //char add1;
  //ref = (char *)lp;

  //Header
  _UART_TxBuffer[0] = 0x3A; //:
   _UART_TxBuffer[1] = 0x34; //version 4

  //Address 2byte
  //add1 = (lp->Add>>12) & 0x0F;
  _UART_TxBuffer[2] = TABLA_HEX2ASCII[(lp->Add>>12) & 0x0F];
  _UART_TxBuffer[3] = TABLA_HEX2ASCII[(lp->Add>>8) & 0x0F];
  _UART_TxBuffer[4] = TABLA_HEX2ASCII[(lp->Add>>4) & 0x0F];
  _UART_TxBuffer[5] = TABLA_HEX2ASCII[lp->Add & 0x0F];

  //Frame number 1byte
  _UART_TxBuffer[6] = TABLA_HEX2ASCII[(lp->Frame_Number>>4) & 0x0F];
  _UART_TxBuffer[7] = TABLA_HEX2ASCII[lp->Frame_Number & 0x0F];

  //Command 1byte
  _UART_TxBuffer[8] = TABLA_HEX2ASCII[(lp->Command>>4) & 0x0F];
  _UART_TxBuffer[9] = TABLA_HEX2ASCII[lp->Command & 0x0F];

  //Register 2bytes
  _UART_TxBuffer[10] = TABLA_HEX2ASCII[(lp->Register>>12) & 0x0F];
  _UART_TxBuffer[11] = TABLA_HEX2ASCII[(lp->Register>>8) & 0x0F];
  _UART_TxBuffer[12] = TABLA_HEX2ASCII[(lp->Register>>4) & 0x0F];
  _UART_TxBuffer[13] = TABLA_HEX2ASCII[lp->Register & 0x0F];

  //Data Length 2bytes
  _UART_TxBuffer[14] = TABLA_HEX2ASCII[(lp->Data_Length>>4) & 0x0F];
  _UART_TxBuffer[15] = TABLA_HEX2ASCII[lp->Data_Length & 0x0F];

  //Data
  for (i=0 , j=16  ; i < pdu_read.Data_Length/4 ; i++ , j=j+4)
  {
	  _UART_TxBuffer[j] = TABLA_HEX2ASCII[(lp->Data[i]>>12) & 0x0F];
	  _UART_TxBuffer[j+1] = TABLA_HEX2ASCII[(lp->Data[i]>>8) & 0x0F];
	  _UART_TxBuffer[j+2] = TABLA_HEX2ASCII[(lp->Data[i]>>4) & 0x0F];
	  _UART_TxBuffer[j+3] = TABLA_HEX2ASCII[lp->Data[i] & 0x0F];
  }

  //LRC calculated in HEX
  LRC = 0;
  LRC = (((lp->Add)>>8) & 0xFF) ^ (lp->Add & 0xFF);
  LRC = (LRC) ^ (lp->Frame_Number);					//Nico: Sumamos (xor) todos los bits en HEX de la trama para calcular el LRC
  LRC = (LRC) ^ (lp->Command);
  LRC = (LRC) ^ (((lp->Register)>>8) & 0xFF);
  LRC = (LRC) ^ (lp->Register & 0xFF);
  LRC = (LRC) ^ (lp->Data_Length & 0xFF);

  for (i=0; i < pdu_read.Data_Length/4 ; i++)
  {
	  LRC = (LRC) ^ (((lp->Data[i])>>8) & 0xFF);
	  LRC = (LRC) ^ (lp->Data[i] & 0xFF);
  }

  //LRC 1byte
  _UART_TxBuffer[16 + pdu_read.Data_Length] = TABLA_HEX2ASCII[(LRC>>4) & 0x0F];
  _UART_TxBuffer[16 + pdu_read.Data_Length + 1] = TABLA_HEX2ASCII[LRC & 0x0F];

  //End Mark
  _UART_TxBuffer[16 + pdu_read.Data_Length + 2] = 0x0D;
  _UART_TxBuffer[16 + pdu_read.Data_Length + 3] = 0x0A;

  UART_TXSIZE_variable = (16 + pdu_read.Data_Length + 4);	//Tx frame length in CHARS

  #ifdef __RS485_ENABLED
	  UART_TxBuffer( (const char *)_UART_TxBuffer);
  #endif

  #ifdef __USB_ENABLED
	  //strcpy ( USB_Buffer_Tx , _UART_TxBuffer );
	  //USB_FlagCharTx = 1;

	  USB_Send( (const char *)_UART_TxBuffer);
  #endif
}






