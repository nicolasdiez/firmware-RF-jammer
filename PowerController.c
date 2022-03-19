/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: Nov 2015										*/
/* File: PowerController.c								*/
/********************************************************/

#include <string.h>
#include "HAL.h"
#include "Acciones2.h"
#include "ATT.h"
#include "PowerController.h"
#include "Power.h"

extern union register_entry ArrayTControl[TC_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes
//extern union register_entry ArrayGen[GEN_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2byteschar

extern signed short ATT_MIN_PREVIO[2];	//La Att mínima calculada al inicio para que la Pot en previo no exceda la Pobjprev (ArrayTControl[REG_POTPREVIOOBJ_BF])
//********************************************************************

unsigned short ErrorAnt = 0;
signed short PotError = 0;
signed short PotIncError = 0;
signed short PotObjetivo = 0;
signed short PotActual = 0;
signed long Att1 = 0;
signed long Att2 = 0;
signed long Att3 = 0;
signed long Att = 0;

unsigned short reg_potfwd2;
unsigned short reg_potprevioobj2;
unsigned short reg_potfwdobjreal2;
unsigned short reg_potpreviofwd2;
unsigned short reg_alarmasactivas;
unsigned short reg_monitorAtt;
unsigned short ATTSelect;
unsigned short mask_critical_potprevexces2;
signed short PotPrevio_Umbral;
unsigned short Temperatura;

void ACCIONES_PowerControllerPD (unsigned int hpa)
{
	//int PowerReached;
	const short k_desplazamiento = 5;
	const short k1 = 28/*valor ant = 28*/;	//k1 vale entre 1 y 2^5 (k1_desplazamiento) -> multiplico por k1, divido por 2^k_desplazamiento
	const short k2 = /*2*//*5*/0;

	if (hpa == BF)
	{
		reg_potfwd2 = REG_POTFWD_BF;
		reg_potfwdobjreal2 = REG_POTFWDOBJREAL_BF/*REG_POTFWDOBJ_BF*/;
		reg_potpreviofwd2 = REG_POT1PREVIO_BF;
		reg_potprevioobj2 = REG_POTPREVIOOBJ_BF;
		reg_alarmasactivas = REG_TC_ALARMASACTIVASHPABF;
		reg_monitorAtt = REG_ATTTOT_BF;
		ATTSelect = SEL_ATT_PREVIO_BF;
		Temperatura= REG_T_BF;
		//mask_critical_potprevexces2 = CRITICAL_POTPREVIO_EXCES_BF;
	}
	else if (hpa == AF)
	{
		reg_potfwd2 = REG_POTFWD_AF;
		reg_potfwdobjreal2 = REG_POTFWDOBJREAL_AF/*REG_POTFWDOBJ_AF*/;
		reg_potpreviofwd2 = REG_POT1PREVIO_AF;
		reg_potprevioobj2 = REG_POTPREVIOOBJ_AF;
		reg_alarmasactivas = REG_TC_ALARMASACTIVASHPAAF;
		reg_monitorAtt = REG_ATTTOT_AF;
		ATTSelect = SEL_ATT_PREVIO_AF;
		Temperatura= REG_T_AF;
		//mask_critical_potprevexces2 = CRITICAL_POTPREVIO_EXCES_AF;
	}


	if (Flags[hpa].HPA_Stop == 0)
	{
		PotPrevio_Umbral = ArrayTControl[/*0x4B*/reg_potprevioobj2].s_16 + POTPREVIO_MAXADMITTED_ERROR /*2.5dB*/ /*(0.25*abs(ArrayTControl[reg_potprevioobj2].s_16) )*/;

		//If previo power is above its 125% threshold re-start HPAs
		if( (ArrayTControl[/*0x11*/reg_potpreviofwd2].s_16 > PotPrevio_Umbral) && (Flags[BF].ONreciente == 0) )
		{
			//POWER_HPA_Stop (hpa);
			//__delay_cycles(24000000);
			//POWER_HPA_Start (hpa);	//EVENT_PEND CAN`T BE CALLED FROM OUTSIDE A TASK!!
			//__delay_cycles(24000000);

			if(hpa == BF)
				ATT_SetTotalAttenuation (ATT_MAX , BF);
			else if (hpa == AF)
				ATT_SetTotalAttenuation (2*ATT_MAX , AF);

			ArrayTControl[reg_alarmasactivas].s_16 |= CRITICAL_POTPREVIO_EXCES;
		}
		else
			ArrayTControl[reg_alarmasactivas].s_16 &= ~CRITICAL_POTPREVIO_EXCES;

		PotActual = ArrayTControl[reg_potfwd2].s_16;
		if (ArrayTControl[Temperatura].s_16 < 0)
			PotActual += (ArrayTControl[Temperatura].s_16 * 10);
		PotObjetivo = ArrayTControl[/*0x0A*/reg_potfwdobjreal2].s_16 /*POTFWDOBJ REAL -> NUEVO REG*/;

		if( (PotActual < (/*0.95 * */PotObjetivo)) || (PotActual > (/*1.05 * */PotObjetivo)) )
		{
			PotError = (PotActual - PotObjetivo);
			PotIncError = (PotError - ErrorAnt);

			Att1 = PotError * k1;
			Att1 = 10 * Att1;
			Att1 = (Att1 >> k_desplazamiento);

			Att2 = PotIncError * k2;
			Att2 = 10 * Att2;
			Att2 = (Att2 >> k_desplazamiento);

			Att/*dBx100*/ = Att1 + Att2;

			//ATT_SetAttenuation( ArrayTControl[reg_monitorAtt].s_16 + Att , ATTSelect);
			ATT_SetTotalAttenuation( ArrayTControl[reg_monitorAtt].s_16 + Att,  hpa);

			ErrorAnt = PotError;
		}
	}
}



unsigned short PrevioErrorAnt = 0;
signed short PrevioPotError = 0;
signed short PrevioPotIncError = 0;
signed short PrevioPotObjetivo = 0;
signed short PrevioPotActual = 0;
signed long PrevioAtt1 = 0;
signed long PrevioAtt2 = 0;
signed long PrevioAtt3 = 0;
signed long PrevioAtt = 0;

int ACCIONES_PowerPrevioControllerPD (unsigned int hpa)
{
	unsigned short reg_potpreviofwd;
	unsigned short reg_potprevioobj;
	unsigned short reg_monitorAtt;
	//unsigned short ATTSelect;
	char PrevioPowerReached;
	const short k_desplazamiento_previo = 5;
	const short k1_previo = 28/*20*/;	//k1 vale entre 1 y 2^5 (k1_desplazamiento) -> multiplico por k1, divido por 2^k_desplazamiento
	const short k2_previo = 0;

	if (hpa == BF)
	{
		reg_potpreviofwd = REG_POT1PREVIO_BF;
		reg_monitorAtt = REG_ATTTOT_BF;
		reg_potprevioobj = REG_POTPREVIOOBJ_BF;
		ATTSelect = SEL_ATT_PREVIO_BF;
	}
	else if (hpa == AF)
	{
		reg_potpreviofwd = REG_POT1PREVIO_AF;
		reg_monitorAtt = REG_ATTTOT_AF;
		reg_potprevioobj = REG_POTPREVIOOBJ_AF;
		ATTSelect = SEL_ATT_PREVIO_AF;
	}

	PrevioPotActual/*10xdBm*/ = ArrayTControl[reg_potpreviofwd].s_16;
	PrevioPotObjetivo/*10xdBm*/ = ArrayTControl[reg_potprevioobj].s_16;

	if( (PrevioPotActual < ( PrevioPotObjetivo - POTPREVIO_CONTROLLER_RANGOERROR/*0.7dB*/ )) || (PrevioPotActual > (PrevioPotObjetivo + POTPREVIO_CONTROLLER_RANGOERROR/*0.7dB*/)) )
	{
		PrevioPotError = (PrevioPotActual - PrevioPotObjetivo);
		PrevioPotIncError = (PrevioPotError - PrevioErrorAnt);

		PrevioAtt1 = PrevioPotError * k1_previo;
		PrevioAtt1 = 10 * PrevioAtt1;
		PrevioAtt1 = (PrevioAtt1 >> k_desplazamiento_previo);

		PrevioAtt2 = PrevioPotIncError * k2_previo;
		PrevioAtt2 = 10 * PrevioAtt2;
		PrevioAtt2 = (PrevioAtt2 >> k_desplazamiento_previo);

		PrevioAtt /*100xdB*/ = PrevioAtt1 + PrevioAtt2;
		//Att = k1 * (10*PotError) + k2 * 10 * (PotError - ErrorAnt);


		ATT_MIN_PREVIO[hpa] = ArrayTControl[reg_monitorAtt].s_16 + PrevioAtt;

		//ATT_SetAttenuation( ArrayTControl[reg_monitorAtt].s_16 + PrevioAtt , ATTSelect);
		ATT_SetTotalAttenuation( ArrayTControl[reg_monitorAtt].s_16 + PrevioAtt,  hpa);

		PrevioErrorAnt = PrevioPotError;
	}
	else
		return PrevioPowerReached = 1;

	return PrevioPowerReached = 0;
}
