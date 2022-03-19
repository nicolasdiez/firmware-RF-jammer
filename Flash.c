/********************************************************/
/* Author: Nicolás Díez (Based on Firmware UA V1-5_AP)	*/
/* Date: July 2014										*/
/* File: FlashConfig.c									*/
/********************************************************/

//#include <cross_studio_io.h>
//#include <ctl_api.h>
#include <stdlib.h>
#include <string.h>
#include "HAL.h"
#include "Flash.h"
#include "msp430f5636.h"

#ifdef __DEBUG
#include "debug.h"
#endif

//unsigned char soyBF=0;

//extern union registro ArrayTControl[0x20];
//unsigned char atenuacion_temporal=0;

//////////////////////////////
/*typedef struct
{
  char StandBy, ONreciente;			//PREGUNTAR: Qué significan estos parámetros? Necesito descripción.
  union register_entry ArrayTControl[0x106F];
  char ATT;
  char ATTTEMPCOUNTER;
  char WarSWR, ErrorSWR, OKSWR, WarPII, ErrorPII, OKPII, WarTEMP, ErrorTEMP, OKTEMP, ErrorATETEMP;
  char CriticalSWR, CriticalTEMP;
  char MaxATETEMP;
  long TEMPanterior;
}parametros;
extern parametros param[3];*/
//////////////////////////////

extern union register_entry ArrayTControl[TC_NUM_REGS];				//Nico: Cada slot de estos [] ocupa 2bytes
extern union register_entry ArrayGen[GEN_NUM_REGS];


void FLASH_ClearSegment(unsigned char *address) //__monitor		//PREGUNTAR: Este __monitor es algo del debugger que viene del CrossWorks?
{
  FCTL1 = FWPW + ERASE;                    // Set Erase bit
  FCTL3 = FWPW;                            // Clear Lock bit

  *address = 0;                           // Dummy write to erase Flash segment

  FCTL1 = FWPW;                            // Clear WRT bit		//PREGUNTAR: y clear BLKWRT también no?
  FCTL3 = FWPW + LOCK;                     // Set LOCK bit
}


void FLASH_Write(void *address, void *buffer, int size) //__monitor
{
  unsigned int i;								//Nico: Cambio a unsigned int
  unsigned char *pBuffer;
  unsigned char *pAddress;

  pBuffer=(unsigned char *)buffer;
  pAddress=(unsigned char *)address;

  FCTL3 = FWKEY;                            	// Clear Lock bit
  FCTL1 = FWKEY + WRT;                      	// Set WRT bit for write operation (PREGUNTAR: Dejamos BLKWRT a 0 para "byte-block write" ?)

  for (i=0; i < size; i++)
    pAddress[i] = pBuffer[i];                   // Write value to flash

  FCTL1 = FWKEY;                            	// Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     	// Set LOCK bit
  //_EINT();
}


void FLASH_READParameters(void)
{
// Hay que habilitarlo otra vez

  //int i = 0;
  //CARGAMOS EL NUMERO DE SERIE
  //for (i=0;i<3;i++)
      //memcpy(&ArrayTControl[0x1B],(char *)INFO_SEGMENT,sizeof(INFO_SEGMENT)); //Nos saltamos la cookie	//PREGUNTAR:Este INFO_SEGMENT para que?? hay una struct _UNIT_INFO

  /*if (ArrayTControl[0x1B].s_32 == 0x32333030)
    soyBF=1;
  else
    soyBF=0;*/

	  memcpy(&ArrayTControl[REG_CODIGOMODULO1],(char *)INFO_SEGMENT_A,sizeof(UNIT_INFO));			//Cargamos código de módulo - mes fabricación - año fabricación - número serie
	  memcpy(&ArrayTControl[REG_VERSION_HW],(char *)INFO_SEGMENT_B,sizeof(UNIT_INFO_2));			//Cargamos versión HW
	  memcpy(&ArrayTControl[REG_VERSION_FW],(char *)INFO_SEGMENT_C,sizeof(UNIT_INFO_3));			//Cargamos versión FW
}


