/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: December 2015									*/
/* File: PDU.c											*/
/********************************************************/

#include <string.h>
#include <stdlib.h>
#include "HAL.h"
#include "PDU.h"
#include "uart.h"
#include "Power.h"
#include "USB.h"
#include "I2C.h"
#include "Acciones2.h"
#include "FlashMem.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"

#ifdef __WDT_ENABLED
	extern unsigned char signatureWDT;
#endif

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


#ifdef __FULL_DUPLEX
	#include <ti/sysbios/knl/Queue.h>
	extern Queue_Handle myQ;
#endif

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
extern union register_entry ArrayTControl[TC_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes
//extern union register_entry ArrayGen[GEN_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes

extern char _UART_RxBuffer[/*UART_RX_SIZE_MAX*/];
extern char _USB_RxBuffer[ UART_RX_SIZE_MAX/*+1*/]/* = {0}*/;
char _PDU_TxBuffer[UART_TX_SIZE_MAX] = {0};		//In CHARS
extern char UART_RXSIZE_variable;
extern char UART_TXSIZE_variable;
extern char USB_RXSIZE_variable;
PDU_RS485 pdu_read;

PDU_I2C pdu_I2C;
extern int Flag_I2C_Error;

char MaskEnclavamiento = 0;		//TODO: Hacer extern para usarla fuera
char MaskBorrarAlarmas = 0;
char MaskAutotest = 0;
char MaskAttenuator = 0;
int toI2C = 0;
char Command_Action;
extern const Semaphore_Handle semI2C;
extern const Semaphore_Handle semCdc0;
extern char Flag_UART_DataReceived;
extern int CardType;
extern char Testing_LEDs;

//---------------------------------------- FUCNTION METHODS ----------------------------------------
#ifdef __RTOS_ENABLED
void PDU_RTOS_Init(void)
{
	  //------------------- EVENT RS485 RECEIVED ----------------------------
	  Event_post(Event_PDUReceived, NO_RECEIVE);
	  Event_post(Event_USBTxChar, NO_TXCHAR);

	  //------------------- TASK CREATION ----------------------------
	  /* Create task*/
	  Error_Block eb;
	  Error_init(&eb);
	  Task_Params taskParams;
	  Task_Params_init(&taskParams);
	  taskParams.priority = 4/*25*/;					//Lowest priority: 0 , Highest priorit: 31
	  taskParams.stackSize = 640/*512*/;		 		//Task stack size in MAUs (MINIMUM ADDRESSABLE UNIT !)
	  tskhdl_RS485StatMach = Task_create (PDU_RTOS_Task, &taskParams, &eb);
	  if (tskhdl_RS485StatMach == NULL)
	  	System_abort("Task 'tskhdl_RS485StatMach' create failed");
}

void PDU_RTOS_Task(UArg arg0, UArg arg1/*void *p*/)
{
	  UInt posted;

	  while (1)
	  {
		#ifdef __WDT_ENABLED
		  signatureWDT |= SIGNATURE_TASK_PDURECEIVE;
		#endif

			posted = Event_pend(Event_PDUReceived, Event_Id_NONE/*RS485_RECEIVE*/, USB_RECEIVE+RS485_RECEIVE/*Event_Id_NONE*/, 5000/*bien 10000*//*ms*/);

			if (posted == 0)
				System_printf("Timeout expired for Event_pend() PDUReceived\n");

			if (posted & RS485_RECEIVE)
			{
				#ifdef __RS485_ENABLED

					if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_ARRANCANDO) != ST_ARRANCANDO )
						LED_3_POUT |= LED_3_MASK;

					Flag_UART_DataReceived = 0;
					PDU_Interpreta(UART_RXSIZE_variable);
					UART_ClearRxBuffer();			//OJO! Cuidado aqui con la prioridad de la Task que llama a PDU_Interpreta. No borrar buffers de Rx antes de que se ejecute esa tarea

					if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_ARRANCANDO) != ST_ARRANCANDO )
						LED_3_POUT &= ~LED_3_MASK;
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
	  }
}
#endif


void PDU_Interpreta(char UART_RXSIZE_variable2)
{
  char LRC_received;
  char LRC_calculated = 0;
  char j,k,m;
  unsigned short i;
  UInt posted;
  uint16_t wUsbEventMask;


  #ifdef __FULL_DUPLEX
  	  char* _UART_RxBuffer;
  	  if (!Queue_empty(myQ))
  		  _UART_RxBuffer = Queue_dequeue/*Queue_get*/(myQ);
  	  else
  		  return;
  #endif

  memset (pdu_read.Data, 0, PDU_DATASIZE);
  memset (&pdu_read.Data[0], 0, PDU_DATASIZE);

  //Init Data from PDU structs
  for (i=0 ; i<PDU_DATASIZE ; i++)
  {
	  pdu_read.Data[i] = 0x0000;
	  pdu_I2C.Data[i] = 0x00;
  }

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

  //Si la dirección de la trama recibida coincide con la cargada en el registro del micro (num serie XX) o es la de broadcast
  if ( (ArrayTControl[REG_DIRECCION_RS485].s_16 == pdu_read.Add) || (pdu_read.Add == 0xFFFF) )
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
			if (pdu_read.Command == WRITE)
			{
				for(k=0 , m=7 ; k < pdu_read.Data_Length/4 ; k++ , m=m+2)
				{
					pdu_read.Data[k] = (unsigned short)Buffer_recepcion[m]<<8 | (unsigned short)Buffer_recepcion[m+1];	//2bytes
				}
				LRC_received = Buffer_recepcion[7 + pdu_read.Data_Length/2];
			}
			else if ( (pdu_read.Command == READ) || (pdu_read.Command == COMMAND_ACTION) )
			{
				pdu_read.Data[0] = (unsigned short)Buffer_recepcion[7]<<8 | (unsigned short)Buffer_recepcion[8];		//2bytes
				LRC_received = Buffer_recepcion[9];
			}
		}

		//Marco si la trama va para un dispositivo I2C que cuelga del principal
		if( ((pdu_read.Register >> 12 & 0x0F) != 0) )
			toI2C = YES;
		else
			toI2C = NO;

		if( LRC_calculated == LRC_received )
		{
			 //caso de que la trama vaya al dispositivo principal (Tarjeta de Control)
			 if (toI2C == NO)
			 {
				switch (pdu_read.Command)
				{
					case COMMAND_ACTION:

						Command_Action = (char)((pdu_read.Data[0] >> 4) & 0x0F);		//When Command == 0x11 Data is 1bytes, MSByte is 0

						MaskEnclavamiento = pdu_read.Data[0] & 0x0F;		//bits: 0000HIJK (K=0 -> Enclavamiento etapa 1 OFF | K=1 -> Enclavamiento etapa 1 ON)
						MaskBorrarAlarmas = pdu_read.Data[0] & 0x0F;		// 0b0000 = Borrar alarmas NO activas / 0b0001 Borrar todas las alarmas
						MaskAutotest = pdu_read.Data[0] & 0x01;
						MaskAttenuator = pdu_read.Data[0] & 0x0F;
						int it;

						switch(Command_Action/*[i]*/)
						{
							///////////////////////////////////////////////////////////////
							case ACTION_ENCLAVAMIENTO:
								if ( ((MaskEnclavamiento & 0x01) == 0x01) && (Flags[BF].System_Stop == 0) )			//SHUT-DOWN HPA BF
								{
									POWER_System_Stop (BF);
									//__delay_cycles(2000000);
									//break;
								}
								else if( ((MaskEnclavamiento & 0x01) == 0x00) && (Flags[BF].System_Stop == 1) )		//START HPA BF
								{
									POWER_System_Start (BF);
									//__delay_cycles(2000000);
									//break;
								}

								if ( ((MaskEnclavamiento & 0x02) == 0x02) && (Flags[AF].System_Stop == 0) )			//SHUT-DOWN HPA AF
								{
									POWER_System_Stop (AF);
									//__delay_cycles(2000000);
									//break;
								}
								else if( ((MaskEnclavamiento & 0x02) == 0x00) && (Flags[AF].System_Stop == 1))		//START HPA AF
								{
									POWER_System_Start (AF);
									//__delay_cycles(2000000);
									//break;
								}

								break;

							case ACTION_BITE: 		//No implementado en Platinum
								break;

							case ACTION_CAMBIOPERFIL:	//No implementado en Platinum
								break;

							case ACTION_BORRARALARMAS:
								if (MaskBorrarAlarmas == 0 || MaskBorrarAlarmas == 1)	//Borrar de los registros NO RECONOCIDOS las alarmas que NO estén ACTIVAS
								{
									ArrayTControl[REG_TC_ALARMASNORECTC].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECTC].s_16) & ArrayTControl[REG_TC_ALARMASNORECTC].s_16 );
									ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 );
									ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 );
								}
								else if (MaskBorrarAlarmas == 2)	//BF
								{
									ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 );

									/*ArrayTControl[REG_TC_ALARMASNORECTC].s_16 = 0x0000;
									ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 = 0x0000;
									ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 = 0x0000;*/
								}
								else if (MaskBorrarAlarmas == 4)	//AF - Borrar de los registros NO RECONOCIDOS las alarmas que NO estén ACTIVAS
								{
									ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 &= ~( (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 ^ ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16) & ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 );
								}

								break;

							case ACTION_BORRAREMERGENCIA:	//No implementado en Platinum
								break;

							case ACTION_RESET:
								pdu_read.Data[0] = 0x2BB4;
								pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								pdu_read.Data_Length = 4;		//Data_length in CHARS
								PDU_Send(&pdu_read);
								__delay_cycles(100000);

								POWER_ControlledReset(0);
								break;

							case ACTION_UPLOADFW:
								//TODO: Al llegar aquí entrar en el segmento BSL del MCU
								break;

							case ACTION_TESTPREVIO:
								if (MaskAutotest == 0)
									ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_AUTOTEST;
								else if (MaskAutotest == 1)
								{
									POWER_HPA_Stop (BF);
									POWER_HPA_Stop (AF);
									ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_AUTOTEST;
								}

								break;

							case ACTION_CLEARFLASH:

								FLASH_ClearSegment((unsigned char *)INFO_SEGMENT_C);	//Clear INFO_C segment (#serie)
								FLASH_ClearSegment((unsigned char *)INFO_SEGMENT_B);	//Clear INFO_B segment (v HW)

								memcpy(&ArrayTControl[REG_CODIGOMODULO1]  , (char *)INFO_SEGMENT_C , 8 /*sizeof(UNIT_INFO)*/);			//Cargamos código de módulo - mes fabricación - año fabricación - número serie
								memcpy(&ArrayTControl[REG_VERSION_HW] 	, (char *)INFO_SEGMENT_B , 2 /*sizeof(UNIT_INFO_2)*/);			//Cargamos versión HW

								break;
							///////////////////////////////////////////////////////////////

							case ACTION_READREGSFROMGENERADORA:

								I2C_LoadParamsFromGeneradora(CardType);
								break;

							case ACTION_TEST_ATT1:

								POWER_System_Stop (CardType);

								if (CardType == BF)
								{
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V7_MASK;
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V6_MASK;
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V5_MASK;
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V4_MASK;
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V3_MASK;
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V2_MASK;
									ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V1_MASK;

									if (MaskAttenuator == 1)
										ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V5_MASK;
									if (MaskAttenuator == 2)
										ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V4_MASK;
									if (MaskAttenuator == 4)
										ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V3_MASK;
									if (MaskAttenuator == 8)
										ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V2_MASK;
									if (MaskAttenuator == 15)
										ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V1_MASK;
								}
								else if (CardType == AF)
								{
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V7_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V6_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V5_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V4_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V3_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V2_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V1_MASK;

									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V7_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V6_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V5_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V4_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V3_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V2_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V1_MASK;

									if (MaskAttenuator == 1)
										ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V5_MASK;
									if (MaskAttenuator == 2)
										ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V4_MASK;
									if (MaskAttenuator == 4)
										ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V3_MASK;
									if (MaskAttenuator == 8)
										ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V2_MASK;
									if (MaskAttenuator == 15)
										ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V1_MASK;
								}
								break;

							case ACTION_TEST_ATT2:

								if (CardType == AF)
								{
									POWER_System_Stop (CardType);

									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V7_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V6_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V5_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V4_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V3_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V2_MASK;
									ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V1_MASK;

									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V7_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V6_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V5_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V4_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V3_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V2_MASK;
									ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V1_MASK;

									if (MaskAttenuator == 1)
										ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V5_MASK;
									if (MaskAttenuator == 2)
										ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V4_MASK;
									if (MaskAttenuator == 4)
										ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V3_MASK;
									if (MaskAttenuator == 8)
										ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V2_MASK;
									if (MaskAttenuator == 15)
										ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V1_MASK;
								}
								break;

							case ACTION_TEST_LEDS:

								POWER_System_Stop (CardType);

								Testing_LEDs = 1; 			//cutre pero no hay tiempo para más
								LED_1_POUT &= ~LED_1_MASK;
								LED_2_POUT &= ~LED_2_MASK;
								LED_3_POUT &= ~LED_3_MASK;

								for (it=0 ; it < 30 ; it++)
								{
									LED_1_POUT ^= LED_1_MASK;
									LED_2_POUT ^= LED_2_MASK;
									LED_3_POUT ^= LED_3_MASK;
									__delay_cycles(12000000);	//wait 1/2 sec
								}
								Testing_LEDs = 0;

								break;
						}

						pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;		//Contestamos con la dirección 485 propia
						//pdu_read.Data[0] = 0x2BB4;
						pdu_read.Data_Length = 4;		//Data_length in CHARS
						PDU_Send(&pdu_read);
						break;

					case WRITE:

						//------------ Writing Serial Number 0x0045 8bytes for the 1st time
//						if( ((pdu_read.Register & 0xFF) == REG_CODIGOMODULO1) && ( (ArrayTControl[REG_CODIGOMODULO1].s_16 != 0xFFFF) || (ArrayTControl[REG_CODIGOMODULO2].s_16 != 0xFFFF) || (ArrayTControl[REG_MESANYO].s_16 != 0xFFFF) || (ArrayTControl[REG_NUMSERIE].s_16 != 0xFFFF) ) )
//						{
//							//ERROR
//							pdu_read.Command = 0xFF;
//							pdu_read.Add = ArrayTControl[REG_NUMSERIE].s_16;
//							pdu_read.Data[0] = 0x0002;
//							pdu_read.Data_Length = 4;
//							PDU_Send(&pdu_read);
//							break;
//						}
						if( ((pdu_read.Register & 0xFF) == REG_CODIGOMODULO1) || ((pdu_read.Register & 0xFF) == REG_CODIGOMODULO2) || ((pdu_read.Register & 0xFF) == REG_MESANYO) || ((pdu_read.Register & 0xFF) == REG_NUMSERIE) )
						{
							memcpy(&ArrayTControl[REG_CODIGOMODULO1]  , (char *)INFO_SEGMENT_C , 8 /*8bytes*/ );
							memcpy(&ArrayTControl[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);
							FLASH_ClearSegment((unsigned char *)INFO_SEGMENT_C);
							FLASH_Write((unsigned char *)INFO_SEGMENT_C, &ArrayTControl[REG_CODIGOMODULO1], 8 /*8bytes*/);			//Se escribe el num de serie válido por primera vez (Hay ya 0xFF en ese campo que es lo que se lee de la flash al arrancar)
						}
						//------------------------------------------------

						//------------ Writing HW version 0x0046 2bytes for the 1st time
//						if( ((pdu_read.Register & 0xFF) == REG_VERSION_HW) && (ArrayTControl[REG_VERSION_HW].s_16 != 0xFFFF) )
//						{
//							//ERROR
//							pdu_read.Command = 0xFF;
//							pdu_read.Add = ArrayTControl[REG_NUMSERIE].s_16;
//							pdu_read.Data[0] = 0x0003;
//							pdu_read.Data_Length = 4;
//							PDU_Send(&pdu_read);
//							break;
//						}
						if( ((pdu_read.Register & 0xFF) == REG_VERSION_HW) /*&& (ArrayTControl[REG_VERSION_HW].s_16 == 0xFFFF)*/ )
						{
							FLASH_Write((unsigned char *)INFO_SEGMENT_B, &pdu_read.Data[0], pdu_read.Data_Length/2 /*2bytes*/);			//Se escribe la version HW por primera vez (Hay ya 0xFF en ese campo que es lo que se lee de la flash al arrancar)
							memcpy(&ArrayTControl[REG_VERSION_HW]  , (char *)INFO_SEGMENT_B , 2 /*sizeof(UNIT_INFO_2)*/);
							//memcpy(&ArrayTControl[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);
						}
						//------------------------------------------------

						//Standard Register Write
						if( ((pdu_read.Register & 0xFF) != REG_CODIGOMODULO1) && ((pdu_read.Register & 0xFF) != REG_CODIGOMODULO2) && ((pdu_read.Register & 0xFF) != REG_MESANYO) && ((pdu_read.Register & 0xFF) != REG_NUMSERIE) && ((pdu_read.Register & 0xFF) != REG_VERSION_HW) )
							memcpy(&ArrayTControl[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);		//Escribo los datos que vienen en 'Data' en 'ArrayTControl'

						pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;		//Contestamos con la dirección 485 propia
						pdu_read.Data[0] = 0x2BB4;
						pdu_read.Data_Length = 4;		//Data_length in CHARS
						PDU_Send(&pdu_read);
						break;

					case READ:
						if(pdu_read.Data[0] == 0x2BB4)
						{
							memcpy(&pdu_read.Data[0], &ArrayTControl[pdu_read.Register & 0xFF].s_16, pdu_read.Data_Length/2);

							if (pdu_read.Add == 0xFFFF)	//Si es un broadcast para extraer la dirección 485 (TODO: Siempre contestar con la dirección 485 propia en Add)
							{
								pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
							}
						}
						else //case FRAMEERROR
						{
						  pdu_read.Command = 0xFF;
						  pdu_read.Data[0] = 0x0004;		//NICO TODO: Codificar aquí el tipo de error
						  pdu_read.Data_Length = 4;			//Data_length in CHARS
						  pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
						}

						PDU_Send(&pdu_read);
						break;

					case RESET:

						pdu_read.Data[0] = 0x2BB4;
						pdu_read.Data_Length = 4;		//Data_length in CHARS
						PDU_Send(&pdu_read);

						POWER_ControlledReset(0);
						break;
				}
			 }

			 //In case frame´s recipient is an I2C device
			 else if(toI2C == YES)
			 {
				switch (pdu_read.Command)
				{
					case COMMAND_ACTION:

						Semaphore_pend(semI2C, BIOS_WAIT_FOREVER);	//Sem Pend before config i2c PDU !!

							Command_Action = pdu_read.Data[0] & 0xFF;					//Cojo el campo Comando/Acción del LSByte de Data[0]

							pdu_I2C.DevSel = (pdu_read.Register >> 12) & 0x0F;			//I2C address of 'Generadora' (0x01)
							pdu_I2C.RegAdd = 0x00D0;									//Memory slot of the I2C device for Comando/Acción operations
							pdu_I2C.Data_Length = /*1*/(pdu_read.Data_Length) / 2;		//Data length of the Data to be written in BYTES
							pdu_I2C.Data[0] = 0x00;
							pdu_I2C.Data[1] = Command_Action;

							#ifndef __SIMULATEGEN_ENABLED

								I2C_Communication(I2C_WRITE);
								posted = Event_pend(Event_I2C , Event_Id_NONE ,I2C_COMM_FINISHED + I2C_TX_ERROR /*Event_Id_NONE*/, 1000);

								if ( (posted == 0) || (posted & I2C_TX_ERROR) )
								{
									pdu_read.Command = 0xFF;
									pdu_read.Data[0] = 0x0005;						//I2C Error Code
									pdu_read.Data_Length = 4;						//Data_length in CHARS
									pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								}
								else
								{
									pdu_read.Data_Length = 4;									//Data_length in CHARS
									//pdu_read.Data[0] = 0x2BB4;
									pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								}
							#endif

							//memcpy(&ArrayGen[pdu_read.Register & 0xFF/*REG_GEN_COMMAND*/], &pdu_read.Data[0], pdu_read.Data_Length/2);	//Actualizo el array local de la RAM del MCU con los valores de la Generadora

						Semaphore_post(semI2C);

						PDU_Send(&pdu_read);
						break;

					case WRITE:

						Semaphore_pend(semI2C, BIOS_WAIT_FOREVER);		//Sem Pend before config i2c PDU !!

							pdu_I2C.DevSel = (pdu_read.Register >> 12) & 0x0F;			//I2C address of 'Generadora' (0x01)
							pdu_I2C.RegAdd = pdu_read.Register & 0x0FFF;				//Memory slot of the I2C device to be written (0x0 + 3 nibbles !!)
							pdu_I2C.Data_Length = (pdu_read.Data_Length) / 2;			//Data length of the Data to be written IN BYTES

							for(k=0 , m=0 ; k < pdu_read.Data_Length/2 ; k=k+2 , m++)	//Parse Data to the I2C PDU in bytes
							{
								pdu_I2C.Data[k] = (pdu_read.Data[m]>>8) & 0xFF;
								pdu_I2C.Data[k+1] = pdu_read.Data[m] & 0xFF;
							}

							#ifndef __SIMULATEGEN_ENABLED
								I2C_Communication(I2C_WRITE);
								//while(!Flag_I2C_Comm_Completed);

								posted = Event_pend(Event_I2C , Event_Id_NONE ,I2C_COMM_FINISHED + I2C_TX_ERROR /*Event_Id_NONE*/, 1000);

								if ( (posted == 0) || (posted & I2C_TX_ERROR))
								{
									pdu_read.Command = 0xFF;
									pdu_read.Data[0] = 0x0005;						//I2C Error Code
									pdu_read.Data_Length = 4;						//Data_length in CHARS
									pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								}
								else
								{
									pdu_read.Data_Length = 4;									//Data_length in CHARS
									pdu_read.Data[0] = 0x2BB4;
									pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								}
							#endif

							//memcpy(&ArrayGen[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);	//Actualizo el array local de la RAM del MCU con los valores de la Generadora

						Semaphore_post(semI2C);

						PDU_Send(&pdu_read);
						break;

					case READ:

						Semaphore_pend(semI2C, BIOS_WAIT_FOREVER);		//Sem Pend before config i2c PDU !!

							pdu_I2C.DevSel = (pdu_read.Register >> 12) & 0x0F;			//I2C address of 'Generadora' (0x01)
							pdu_I2C.RegAdd = pdu_read.Register & 0x0FFF;				//Memory slot of the I2C device to be read (0x0 + 3 nibbles !!)
							pdu_I2C.Data_Length = (pdu_read.Data_Length) / 2;			//Data length of the Data to be read IN BYTES

							#ifndef __SIMULATEGEN_ENABLED
								I2C_Communication(I2C_READ);
								//while(!Flag_I2C_Comm_Completed);

								posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 1000/*ms*/);

								if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR))
								{
									pdu_read.Command = 0xFF;
									pdu_read.Data[0] = 0x0005;						//I2C Error Code
									pdu_read.Data_Length = 4;						//Data_length in CHARS
									pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								}
								else
								{
									//Build back RS485 PDU from I2C PDU
									for(k=0 , m=0 ; k < pdu_read.Data_Length/2 ; k=k+2 , m++)	//Copy Data from PDU_I2C to PDU_485
									{
										pdu_read.Data[m] = (unsigned short)(pdu_I2C.Data[k]<<8) | (unsigned short)pdu_I2C.Data[k+1];
									}

									pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
								}

								//memcpy(&ArrayGen[pdu_read.Register & 0xFF], &pdu_read.Data[0], pdu_read.Data_Length/2);	//Actualizo el array local de la RAM del MCU con los valores de la Generadora
							#endif

							#ifdef __SIMULATEGEN_ENABLED
								//DEBUG!!: READ FROM ARRAYGEN, NOT FROM ACTUAL GENERADORA
								//memcpy(&pdu_read.Data[0], &ArrayGen[pdu_read.Register & 0xFF], pdu_read.Data_Length/2);
							#endif

						Semaphore_post(semI2C);

						PDU_Send(&pdu_read);
						break;
				}
			 }
		}

		//ERROR - LRC del frame recibido es incorrecto
		else
		{
		  pdu_read.Command = 0xFF;
		  pdu_read.Data[0] = 0x0001;		//LRC error
		  pdu_read.Data_Length = 4;			//Data_length in CHARS
		  pdu_read.Add = ArrayTControl[REG_DIRECCION_RS485].s_16;
		  PDU_Send(&pdu_read);
		}
  }
  else
  {
	  UCA1IE |= UCRXIE;								//Enable UART RX
	  RS485_RXENABLE_POUT &= ~RS485_RXENABLE_MASK; 	//Enable RS485 transceiver (ACTIVE LOW!)

	  //-----------
	  //Enable USB reception
	  wUsbEventMask = USB_getEnabledEvents();
	  wUsbEventMask |= kUSB_dataReceivedEvent;
	  USB_setEnabledEvents (wUsbEventMask);
	  //---------
	  Event_post(Event_USB_Port, USB_FRAME_FINISHED);
  }
}




void PDU_Send (PDU_RS485 *lp)
{
  char i,j;
  char LRC;
  uint16_t wUsbEventMask;

  //Header
  _PDU_TxBuffer[0] = 0x3A; //:
  _PDU_TxBuffer[1] = 0x35; //version 5 (Sparta)

  //Address 2byte
  //add1 = (lp->Add>>12) & 0x0F;
  _PDU_TxBuffer[2] = TABLA_HEX2ASCII[(lp->Add>>12) & 0x0F];
  _PDU_TxBuffer[3] = TABLA_HEX2ASCII[(lp->Add>>8) & 0x0F];
  _PDU_TxBuffer[4] = TABLA_HEX2ASCII[(lp->Add>>4) & 0x0F];
  _PDU_TxBuffer[5] = TABLA_HEX2ASCII[lp->Add & 0x0F];

  //Frame number 1byte
  _PDU_TxBuffer[6] = TABLA_HEX2ASCII[(lp->Frame_Number>>4) & 0x0F];
  _PDU_TxBuffer[7] = TABLA_HEX2ASCII[lp->Frame_Number & 0x0F];

  //Command 1byte
  _PDU_TxBuffer[8] = TABLA_HEX2ASCII[(lp->Command>>4) & 0x0F];
  _PDU_TxBuffer[9] = TABLA_HEX2ASCII[lp->Command & 0x0F];

  //Register 2bytes
  _PDU_TxBuffer[10] = TABLA_HEX2ASCII[(lp->Register>>12) & 0x0F];
  _PDU_TxBuffer[11] = TABLA_HEX2ASCII[(lp->Register>>8) & 0x0F];
  _PDU_TxBuffer[12] = TABLA_HEX2ASCII[(lp->Register>>4) & 0x0F];
  _PDU_TxBuffer[13] = TABLA_HEX2ASCII[lp->Register & 0x0F];

  //Data Length 2bytes
  _PDU_TxBuffer[14] = TABLA_HEX2ASCII[(lp->Data_Length>>4) & 0x0F];
  _PDU_TxBuffer[15] = TABLA_HEX2ASCII[lp->Data_Length & 0x0F];

  //Data
  for (i=0 , j=16  ; i < pdu_read.Data_Length/4 ; i++ , j=j+4)
  {
	  _PDU_TxBuffer[j] = TABLA_HEX2ASCII[(lp->Data[i]>>12) & 0x0F];
	  _PDU_TxBuffer[j+1] = TABLA_HEX2ASCII[(lp->Data[i]>>8) & 0x0F];
	  _PDU_TxBuffer[j+2] = TABLA_HEX2ASCII[(lp->Data[i]>>4) & 0x0F];
	  _PDU_TxBuffer[j+3] = TABLA_HEX2ASCII[lp->Data[i] & 0x0F];
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
  _PDU_TxBuffer[16 + pdu_read.Data_Length] = TABLA_HEX2ASCII[(LRC>>4) & 0x0F];
  _PDU_TxBuffer[16 + pdu_read.Data_Length + 1] = TABLA_HEX2ASCII[LRC & 0x0F];

  //End Mark
  _PDU_TxBuffer[16 + pdu_read.Data_Length + 2] = 0x0D;
  _PDU_TxBuffer[16 + pdu_read.Data_Length + 3] = 0x0A;

  UART_TXSIZE_variable = (16 + pdu_read.Data_Length + 4);	//Tx frame length in CHARS

  #ifdef __RS485_ENABLED
	  UART_Send( (const char *)_PDU_TxBuffer);
  #endif

  #ifdef __USB_ENABLED
	  USB_Send( (const char *)_PDU_TxBuffer, (unsigned int) UART_TXSIZE_variable );

	  USBCDC_rejectData (CDC0_INTFNUM);

	  //Enable USB RX
	  wUsbEventMask = USB_getEnabledEvents();
	  wUsbEventMask |= kUSB_dataReceivedEvent;
	  USB_setEnabledEvents (wUsbEventMask);
	  //---------

	  USBCDC_rejectData (CDC0_INTFNUM);

	  Event_post (Event_USB_Port, USB_FRAME_FINISHED);	//el post aquí funciona bien (posible fallo en Start BFAF,Stop BFAF,Start BFAF,Start BFAF)
  #endif
}

