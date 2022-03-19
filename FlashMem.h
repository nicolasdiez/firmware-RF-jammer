/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: February 2014									*/
/* File: FlashMem.h										*/
/********************************************************/
#ifndef FLASHMEM_H_
#define FLASHMEM_H_

#define INFO_SEGMENT_A	0x1980
#define INFO_SEGMENT_B	0x1900
#define INFO_SEGMENT_C	0x1880

struct _UNIT_INFO
{
  char codigomodulo[4];			//Ojo! el byte mas significativo de estos 4 es un {0} !
  char mesfabricacion;
  char anyofabricacion;
  char numeroserie[2];
};
struct _UNIT_INFO_2
{
	short HWversion;
};

struct _UNIT_INFO_3
{
	short FWversion;
};

typedef struct _UNIT_INFO  UNIT_INFO;
typedef struct _UNIT_INFO_2  UNIT_INFO_2;
typedef struct _UNIT_INFO_3  UNIT_INFO_3;

void FLASH_ReadParameters(void);
void FLASH_Write(void *address,void *buffer, int size);
void FLASH_ClearSegment(unsigned char *address);


#endif
