/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: Novemeber 2015									*/
/* File: Alarms.c										*/
/********************************************************/

#include "HAL.h"
#include "Alarms.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB.h"

extern FlagAlarms Flags[2];
extern union register_entry ArrayTControl[TC_NUM_REGS];
extern int CardType;
extern unsigned char signatureWDT;
extern char Flag_USBAPI_DataReceived;
extern char Flag_UART_DataReceived;

unsigned short reg_swr;
unsigned short reg_t;
unsigned short reg_potfwd;
unsigned short reg_tension;
unsigned short reg_uerrpotfwdinsuf;
unsigned short reg_uerrswr;
unsigned short reg_uerrinftemp;
unsigned short reg_uerrsuptemp;
unsigned short reg_alarmasact;
unsigned short reg_uwarswr;
unsigned short reg_uwarpotfwdinsuf;
unsigned short reg_uwarinftemp;
unsigned short reg_uwarsuptemp;
unsigned short reg_uwarinftension;
unsigned short reg_uwarsuptension;
unsigned short reg_uerrinftension;
unsigned short reg_uerrsuptension;

//-------------------------------------------------------------------------------------------------


void ALARMS_MarcarBFAF(UArg arg0, UArg arg1)
{
	while(1)
	{
		#ifdef __ALARMASMARCAR_BFAF_ENABLED

		#ifdef __WDT_ENABLED
		  signatureWDT |= SIGNATURE_TASK_MARCARBFAF;
		#endif
		//AUX_2_POUT |= AUX_2_MASK; 	//SET pin 9.5

//		  if (CardType == AF)
//			  ALARMS_MarcarAlarmas_Pot_SWR_Temp_Volt(AF);		//cambio orden bf y af!!  día uco (no sol)!
//		  else if (CardType == BF)
//			  ALARMS_MarcarAlarmas_Pot_SWR_Temp_Volt(BF);

		  ALARMS_MarcarAlarmas_Pot_SWR_Temp_Volt(CardType);

		  ALARMS_MarcarEstadoOperativo();

		//AUX_2_POUT &= ~AUX_2_MASK; 	//CLEAR pin 9.5

		#endif

		#ifdef __POST_USBRXEVENTS_IN_TASK
			if (Flag_USBAPI_DataReceived == 1)
				Semaphore_post(semCdc0);	//No marco semáforo en USBCDC_handleDataReceived, marco una variable y en ésta Task marco el sem
			else
				USBCDC_rejectData (CDC0_INTFNUM);
		#endif

		Task_sleep(/*1000*//*300*//*200*/100/*50*/ /*ms*/);
		//Task_yield();	//Yields CPU to another task with equal priority
	}
}


void ALARMS_MarcarAlarmas_Pot_SWR_Temp_Volt (unsigned int hpa)
{
	if (hpa == BF)
	{
	    reg_swr = REG_SWR_BF;
	    reg_potfwd = REG_POTFWD_BF;
	    reg_t = REG_T_BF;
	    reg_tension = REG_TENSION_BF;
	    reg_uerrswr = REG_UERRSWR_BF;
	    reg_uerrpotfwdinsuf = REG_UERRPOTFWDINSUF_BF;
	    reg_uerrinftemp = REG_UERRINFTEMP_BF;
	    reg_uerrsuptemp = REG_UERRSUPTEMP_BF;
	    reg_alarmasact = REG_TC_ALARMASACTIVASHPABF;
	    reg_uwarsuptemp = REG_UWARSUPTEMP_BF;
	    reg_uwarpotfwdinsuf = REG_UWARPOTFWDINSUF_BF;
	    reg_uwarinftemp = REG_UWARINFTEMP_BF;
	    reg_uwarswr = REG_UWARSWR_BF;
	    reg_uwarinftension = REG_UWARINFTENSION_BF;
	    reg_uwarsuptension = REG_UWARSUPTENSION_BF;
	    reg_uerrinftension = REG_UERRINFTENSION_BF;
	    reg_uerrsuptension = REG_UERRSUPTENSION_BF;
	}
	else if (hpa == AF)
	{
	    reg_swr = REG_SWR_AF;
	    reg_potfwd = REG_POTFWD_AF;
	    reg_t = REG_T_AF;
	    reg_tension = REG_TENSION_AF;
	    reg_potfwd = REG_POTFWD_AF;
	    reg_uerrswr = REG_UERRSWR_AF;
	    reg_uerrpotfwdinsuf = REG_UERRPOTFWDINSUF_AF;
	    reg_uerrinftemp = REG_UERRINFTEMP_AF;
	    reg_uerrsuptemp = REG_UERRSUPTEMP_AF;
	    reg_alarmasact = REG_TC_ALARMASACTIVASHPAAF;
	    reg_uwarswr = REG_UWARSWR_AF;
	    reg_uwarpotfwdinsuf = REG_UWARPOTFWDINSUF_AF;
	    reg_uwarinftemp = REG_UWARINFTEMP_AF;
	    reg_uwarsuptemp = REG_UWARSUPTEMP_AF;
	    reg_uwarinftension = REG_UWARINFTENSION_AF;
	    reg_uwarsuptension = REG_UWARSUPTENSION_AF;
	    reg_uerrinftension = REG_UERRINFTENSION_AF;
	    reg_uerrsuptension = REG_UERRSUPTENSION_AF;
	}



	if ( /*(Flags[hpa].System_Stop == 0) &&*/ Flags[hpa].HPA_Stop == 0 )
	{
			//*************************************************** TEMPERATURA & SWR **************************************************
			// TEMPERATURA CRÍTICA
			if( ((ArrayTControl[reg_t].s_16 < ArrayTControl[reg_uerrinftemp].s_16) || (ArrayTControl[reg_t].s_16 > ArrayTControl[reg_uerrsuptemp].s_16)) )
			{
				Flags[hpa].ErrorTEMP++;
				Flags[hpa].OKTEMP=0;
				Flags[hpa].WarTEMP=0;

				if(Flags[hpa].ErrorTEMP>=/*30*/20)
				{
					ArrayTControl[reg_alarmasact].s_16 |= CRITICAL_T;
					ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_T;
				}
			}
			// TEMPERATURA WARNING
			else if( ((ArrayTControl[reg_t].s_16 < ArrayTControl[reg_uwarinftemp].s_16) || (ArrayTControl[reg_t].s_16 > ArrayTControl[reg_uwarsuptemp].s_16)) )
			{
				Flags[hpa].WarTEMP++;
				Flags[hpa].OKTEMP=0;
				Flags[hpa].ErrorTEMP=0;

				if(Flags[hpa].WarTEMP>=/*30*/40)
				{
					ArrayTControl[reg_alarmasact].s_16 |= WARNING_T;
					ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_T;
				}
			}
			// TEMPERATURA OK
			else
			{
				Flags[hpa].OKTEMP++;
				Flags[hpa].ErrorTEMP=0;
				Flags[hpa].WarTEMP=0;

				if(Flags[hpa].OKTEMP>=0/*30*//*60*/)
				{
					ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_T;
					ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_T;
				}
			}


			if(Flags[hpa].ONreciente == 0 || Flags[hpa].ONreciente == 1)
			{
				//SWR CRITICAL - Condición invertida porque es diferencia de dBms no es SWR real -> Alarma salta si SWR < SWR_UMBRAL_ERROR
				if((ArrayTControl[reg_swr].s_16 < ArrayTControl[reg_uerrswr].s_16) && (ArrayTControl[reg_potfwd].s_16 > /*bien 100*/ /*uco*/300))	//cambio día uco (únicamente ÉSTO soluciona SOLO 1er arranque hasta 1ª protección swr)
				{
					Flags[hpa].ErrorSWR++;
					Flags[hpa].OKSWR=0;
					Flags[hpa].WarSWR=0;

					if (Flags[hpa].ErrorSWR >= 0)	//Cambio el if de marcar alarma respecto a v3
					{
						ArrayTControl[reg_alarmasact].s_16 |= CRITICAL_SWR;
						ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_SWR;
					}
				}
				//SWR WARNING
				else if((ArrayTControl[reg_swr].s_16 < ArrayTControl[reg_uwarswr].s_16) && (ArrayTControl[reg_potfwd].s_16 > /*bien 100*/ /*uco*/300))
				{
					Flags[hpa].WarSWR++;
					Flags[hpa].ErrorSWR=0;
					Flags[hpa].OKSWR=0;

					if (Flags[hpa].WarSWR >= 0)	//Cambio el if de marcar alarma respecto a v3
					{
						ArrayTControl[reg_alarmasact].s_16 |= WARNING_SWR;
						ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_SWR;
					}
				}
				//SWR OK
				else if( (ArrayTControl[reg_swr].s_16 >= ArrayTControl[reg_uerrswr].s_16) && (ArrayTControl[reg_potfwd].s_16 > /*bien 100*/ 100))
				{
					Flags[hpa].OKSWR++;
					Flags[hpa].ErrorSWR=0;
					Flags[hpa].WarSWR=0;

					if (Flags[hpa].OKSWR >= 0)	//Cambio el if de marcar alarma respecto a v3
					{
						ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_SWR;
						ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_SWR;
					}
				}
			 }
			//***********************************************************************************************************************************

			//*************************************************** POTENCIA INCIDENTE & TENSION **************************************************
			//if( (MUTE_HPAX_POUT & MUTE_HPAX_MASK) != MUTE_HPAX_MASK/*Flags[hpa].StandBy == 0*/)
			if( (Flags[hpa].System_Stop == 0) || (Flags[hpa].System_Stop == 1) )
			{
					//POTENCIA FWD CRITICAL
					if(ArrayTControl[reg_potfwd].s_16 < ArrayTControl[reg_uerrpotfwdinsuf].s_16)
					{
						Flags[hpa].ErrorPII++;
						Flags[hpa].OKPII=0;
						Flags[hpa].WarPII=0;

						if (Flags[hpa].ErrorPII >= 0)	//Cambio el if de marcar alarma respecto a v3
						{
							ArrayTControl[reg_alarmasact].s_16 |= CRITICAL_POT_INS;
							ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_POT_INS;
						}
					}
					//POTENCIA FWD WARNING
					else if(ArrayTControl[reg_potfwd].s_16 < ArrayTControl[reg_uwarpotfwdinsuf].s_16)
					{
						Flags[hpa].WarPII++;
						Flags[hpa].OKPII=0;
						Flags[hpa].ErrorPII=0;

						if (Flags[hpa].WarPII >= 0)		//Cambio el if de marcar alarma respecto a v3
						{
							ArrayTControl[reg_alarmasact].s_16 |= WARNING_POT_INS;
							ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_POT_INS;
						}
					}
					//POTENCIA FWD OK
					else
					{
						Flags[hpa].OKPII++;
						Flags[hpa].ErrorPII=0;
						Flags[hpa].WarPII=0;

						if (Flags[hpa].OKPII >= 0)		//Cambio el if de marcar alarma respecto a v3
						{
							ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_POT_INS;
							ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_POT_INS;
						}
					}



					//TENSIÓN CRITICAL
					if( ((ArrayTControl[reg_tension].s_16 < ArrayTControl[reg_uerrinftension].s_16) || (ArrayTControl[reg_tension].s_16 > ArrayTControl[reg_uerrsuptension].s_16)) )
					{
						Flags[hpa].ErrorTension++;
						Flags[hpa].OKTension=0;
						Flags[hpa].WarTension=0;

						if (Flags[hpa].ErrorTension >= 50)	//Cambio el if de marcar alarma respecto a v3
						{
							ArrayTControl[reg_alarmasact].s_16 |= CRITICAL_TENSION;
							ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_TENSION;
						}
					}
					//TENSIÓN WARNING
					else if( ((ArrayTControl[reg_tension].s_16 < ArrayTControl[reg_uwarinftension].s_16) || (ArrayTControl[reg_tension].s_16 > ArrayTControl[reg_uwarsuptension].s_16)) )
					{
						Flags[hpa].WarTension++;
						Flags[hpa].OKTension=0;
						Flags[hpa].ErrorTension=0;

						if (Flags[hpa].WarTension >= 50)		//Cambio el if de marcar alarma respecto a v3
						{
							ArrayTControl[reg_alarmasact].s_16 |= WARNING_TENSION;
							ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_TENSION;
						}
					}
					//TENSIÓN OK
					else
					{
						Flags[hpa].OKTension++;
						Flags[hpa].ErrorTension=0;
						Flags[hpa].WarTension=0;

						if (Flags[hpa].OKTension >= 50)		//Cambio el if de marcar alarma respecto a v3
						{
							ArrayTControl[reg_alarmasact].s_16 &= ~CRITICAL_TENSION;
							ArrayTControl[reg_alarmasact].s_16 &= ~WARNING_TENSION;
						}
					}
			}
			//***********************************************************************************************************************************
	}
	else	//if HPA is OFF
	{
		ArrayTControl[reg_alarmasact].s_16 &= ~(CRITICAL_POTPREVIO_EXCES ^ CRITICAL_POTPREVIO_INSUF ^ CRITICAL_POT_PREVIO ^ CRITICAL_T ^ CRITICAL_SWR ^ CRITICAL_POT_INS ^ CRITICAL_TENSION);
		ArrayTControl[reg_alarmasact].s_16 &= ~(WARNING_T ^ WARNING_SWR ^ WARNING_POT_INS ^ WARNING_TENSION);
	}
	//***********************************************************************************************************************************



}


void ALARMS_MarcarEstadoOperativo (void)
{
	//**************** HPA BF Alarms ****************
	if((ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & SI_CRITICAL) != 0x0000)			//BF Critical alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_CRITICAL_BF;
	else
	    ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_CRITICAL_BF;

	if((ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & SI_WARNING) != 0x0000)				//BF Warning alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_WARNING_BF;
	else
	    ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_WARNING_BF;

	//Actualizar el registro histórico de alarmas BF
	ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 |= ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16;


	//**************** HPA AF Alarms ****************
	if((ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & SI_CRITICAL) != 0x0000)			//AF Critical alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_CRITICAL_AF;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_CRITICAL_AF;

	if((ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & SI_WARNING) != 0x0000)				//AF Warning alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_WARNING_AF;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_WARNING_AF;

	//Actualizar el registro histórico de alarmas AF
	ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 |= ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16;


	//**************** CONTROL CARD Alarms ****************
	if((ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 & SI_CRITICAL) != 0x0000)			//CONTROL Critical alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_CRITICAL_CONTROL;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_CRITICAL_CONTROL;

	if((ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 & SI_WARNING) != 0x0000)			//CONTROL Warning alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_WARNING_CONTROL;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_WARNING_CONTROL;

	//Actualizar el registro histórico de alarmas TC
	ArrayTControl[REG_TC_ALARMASNORECTC].s_16 |= ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16;

	//******************************************************
	if( (Flags[BF].System_Stop == 1) && (Flags[AF].System_Stop == 1) )
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_STANDBY_USER;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_STANDBY_USER;

	//******************************************************
	if( (Flags[BF].HPA_Stop == 1) && (Flags[AF].HPA_Stop == 1) )
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_STANDBY_HPA;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_STANDBY_HPA;

	//*****************************************************
	if( ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & (ST_CRITICAL_CONTROL ^ ST_WARNING_CONTROL ^ ST_CRITICAL_BF ^ ST_CRITICAL_AF ^ ST_WARNING_BF ^ ST_WARNING_AF) ) == 0x0000) && (Flags[CardType].HPA_Stop == 0) )
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |=ST_OK;
	else
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_OK;

	//*****************************************************
}
