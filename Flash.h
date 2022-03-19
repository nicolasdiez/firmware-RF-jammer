///**************************************************/
/* Author: Nicolás Díez Risueño							*/
///* Date: July 2014								*/
///* File: FlashConfig.h							*/
///**************************************************/
//#ifndef __FLASH_H
//#define __FLASH_H
//
////#define INFO_SEGMENT/*FLASH_INHIBITORUNIT_INFO_SEGMENT*/  		0x1000		//PREGUNTAR: qué hay en esta dirección de la flash?? -> se carga en fabricación, hay una estructura tipo _UNIT_INFO
////#define INFO_SEGMENT_B/*FLASH_SYSTEMPARAMETERS_INFO_SEGMENT*/ 	0x1080		//Nico: Aquí creo que hay una estructura tipo PARAM_INFO ,
//
//#define INFO_SEGMENT_A	0x1980
//#define INFO_SEGMENT_B	0x1900
//#define INFO_SEGMENT_C	0x1880
//
///*struct _UNIT_INFO
//{
//  char serial[16];
//  char model[4];
//  char cookie[2];
//};*/
//
//struct _UNIT_INFO
//{
//  char codigomodulo[4];			//Ojo! el byte mas significativo de estos 4 es un {0} !
//  char mesfabricacion;
//  char anyofabricacion;
//  char numeroserie[2];
//};
//
//struct _UNIT_INFO_2
//{
//	short HWversion;
//};
//
//struct _UNIT_INFO_3
//{
//	short FWversion;
//};
////NICO: Checkear que campos hay en el segmento 0x1080 de la flash
//struct _PARAM_INFO
//{
//  signed short perfil_activo;					//Número de perfil cargado. 16bits
//  //signed long ateobjetivo;
//  //signed long uswhall;
//  //signed long uiwhall;
//  //signed long usehall;
//  //signed long uiehall;
//  //signed long uswdcdc;
//  //signed long uiwdcdc;
//  //signed long usedcdc;
//  //signed long uiedcdc;
//  //signed long uwswr;
//  //signed long ueswr;
//  //signed long uwtemperatura;
//  //signed long uetemperatura;
//  //signed long uwpotincinsuf;
//  //signed long uepotincinsuf;
//
//  signed short potincobjetivo_HPA_BF;			//Potencia incicende Objetivo HPA Low Freq
//  signed short uwpotincinsuf_HPA_BF;			//Umbral Warning Potencia incidente insuficiente HPA Low Freq
//  signed short uepotincinsuf_HPA_BF;			//Umbral Error Crítico Potencia incidente insuficiente HPA Low Freq
//  signed short uwswr_HPA_BF;					//Umbral Warning SWR HPA Low Freq
//  signed short ueswr_HPA_BF;					//Umbral Error Crítico SWR HPA Low Freq
//  signed short uinfwtemperatura_HPA_BF;			//Umbral inferior Warning Temperatura HPA Low Freq
//  signed short uinfetemperatura_HPA_BF;			//Umbral inferior Error Crítico Temperatura HPA Low Freq
//  signed short usupwtemperatura_HPA_BF;			//Umbral superior Warning Temperatura HPA Low Freq
//  signed short usupetemperatura_HPA_BF;			//Umbral superior Error Crítico Temperatura HPA Low Freq
//
//  signed short potincobjetivo_HPA_AF;			//Potencia incicende Objetivo HPA High Freq
//  signed short uwpotincinsuf_HPA_AF;			//Umbral Warning Potencia incidente insuficiente HPA High Freq
//  signed short uepotincinsuf_HPA_AF;			//Umbral Error Crítico Potencia incidente insuficiente HPA High Freq
//  signed short uwswr_HPA_AF;					//Umbral Warning SWR HPA High Freq
//  signed short ueswr_HPA_AF;					//Umbral Error Crítico SWR HPA High Freq
//  signed short uinfwtemperatura_HPA_AF;			//Umbral inferior Warning Temperatura HPA High Freq
//  signed short uinfetemperatura_HPA_AF;			//Umbral inferior Error Crítico Temperatura HPA High Freq
//  signed short usupwtemperatura_HPA_AF;			//Umbral superior Warning Temperatura HPA High Freq
//  signed short usupetemperatura_HPA_AF;			//Umbral superior Error Crítico Temperatura HPA High Freq
//};
//
//typedef struct _UNIT_INFO  UNIT_INFO;
//typedef struct _UNIT_INFO_2  UNIT_INFO_2;
//typedef struct _UNIT_INFO_3  UNIT_INFO_3;
//typedef struct _PARAM_INFO PARAM_INFO;
//
//void FLASH_Write(void *address,void *buffer, int size);
//void FLASH_ClearSegment(unsigned char *address);
//void FLASH_ReadParameters(void);
//
//#endif
