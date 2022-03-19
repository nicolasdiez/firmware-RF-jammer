/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: November 2015									*/
/* File: FlashMem.c										*/
/********************************************************/
#include <stdlib.h>
#include <string.h>
#include "HAL.h"
//#include "driverlib.h"
#include "FlashMem.h"

#ifdef	__MSP430F5638
	#include "msp430f5638.h"
#endif
#ifdef	__MSP430F5636
	#include "msp430f5636.h"
#endif

extern union register_entry ArrayTControl[TC_NUM_REGS];				//Nico: Cada slot de estos [] ocupa 2bytes
//extern union register_entry ArrayGen[GEN_NUM_REGS];


void FLASH_ReadParameters(void)
{
  memcpy(&ArrayTControl[REG_CODIGOMODULO1],(char *)INFO_SEGMENT_C, 8 );			//Cargamos código de módulo - mes fabricación - año fabricación - número serie
  memcpy(&ArrayTControl[REG_VERSION_HW],(char *)INFO_SEGMENT_B, 2 );			//Cargamos versión HW
  memcpy(&ArrayTControl[REG_VERSION_FW],(char *)INFO_SEGMENT_A, 2 );			//Cargamos versión FW
}

void FLASH_Write(void *address, void *buffer, int size)
{
  unsigned int i;								//Nico: Cambio a unsigned int
  unsigned char *pBuffer;
  unsigned char *pAddress;

  pBuffer=(unsigned char *)buffer;
  pAddress=(unsigned char *)address;

  FCTL3 = FWKEY + LOCKA;                        // Clear Lock bit + toggle segment A lock
  FCTL1 = FWKEY + WRT;                      	// Set WRT bit for write operation (PREGUNTAR: Dejamos BLKWRT a 0 para "byte-block write" ?)

  for (i=0; i < size; i++)
  {
    pAddress[i] = pBuffer[i];                   // Write value to flash
    //__delay_cycles(50);
  }

  FCTL1 = FWKEY;                            	// Clear WRT bit
  FCTL3 = FWKEY + LOCK + LOCKA;                 // Set LOCK bit + toggle segment A lock
  //_EINT();
}

void FLASH_ClearSegment(unsigned char *address)
{
  FCTL1 = FWPW + ERASE;                    // Set Erase bit
  FCTL3 = FWPW + LOCKA;                    // Clear Lock bit + toggle segment A lock

  *address = 0;                           // Dummy write to erase Flash segment

  FCTL1 = FWPW;                            // Clear WRT bit		//PREGUNTAR: y clear BLKWRT también no?
  FCTL3 = FWPW + LOCK + LOCKA;             // Set LOCK bit + toggle segment A lock
}
