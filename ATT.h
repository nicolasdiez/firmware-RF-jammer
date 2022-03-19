/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: ATT.h											*/
/********************************************************/

#ifndef _ATT_INCLUDE_HEADER_H_
#define _ATT_INCLUDE_HEADER_H_

//#define MAX_ATT 155

void ATT_Config (int ATTSelect);
signed long ATT_SetAttenuation (signed short bAtt, unsigned int ATTSelect);
void ATT_SetTotalAttenuation(signed short bAtt, unsigned int hpa);


#endif
