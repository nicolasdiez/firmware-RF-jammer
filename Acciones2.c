/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: Novemeber 2015									*/
/* File: Acciones2.c									*/
/********************************************************/

#include <string.h>
#include "HAL.h"
#include "ADC2.h"
//#include "Acciones.h"
#include "Acciones2.h"
#include "ATT.h"
#include "PDU.h"
#include "Ports.h"
#include "Timers.h"
#include "I2C.h"
#include <math.h>
#include "PowerController.h"
#include "Power.h"
#include "USB.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"

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
	#include <ti/sysbios/hal/Hwi.h>
	#include <xdc/runtime/Log.h>
	#include "I2C.h"
#endif



//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
extern FlagAlarms Flags[2];												//Flags[0] for BF & Flags[1] for AF
//extern unsigned char signatureWDT;
//extern PDU_I2C pdu_I2C;												//Main declaration in RS485.c
extern union register_entry ArrayTControl[TC_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes
//extern union register_entry ArrayGen[GEN_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes
int InitialConfig = 0;

extern unsigned int Trecovery[2]/*ºC*/;
int CardType = ERROR_CARD;

int timertest = 0;
//-------------------------------------------------------------------------------------------------


//---------------- TASK & EVENTS HANDLERS-----------------
Task_Handle tskhdl_Acciones;

//These parameters are loaded in RAM by default in case the I2C params generadora fail to load
void ACCIONES_Regs_Init(void)
{
	//ACCIONES_InitRegisters();

	//********* SET BF PARAMS *********
	Flags[BF].HPA_Stop = 1;
	Flags[BF].System_Stop = 1;
	ArrayTControl[REG_T_BF].s_16 = 40 /*ºC*/;

	ArrayTControl[REG_POTFWDOBJUSER_BF].s_16 = 460/* 10 x dBm*/;
	/*HACER ÉSTO EN EL INIT!!!*/ ArrayTControl[REG_POTFWDOBJREAL_BF].s_16 = ArrayTControl[REG_POTFWDOBJUSER_BF].s_16; //In case the FSM does not execute before power controller at start up
	ArrayTControl[REG_POTPREVIOOBJ_BF].s_16 = 0/*-50*/ /*dBm x 10*/;
	ArrayTControl[REG_SWR_BF].s_16 = 4;	/*DEBUGGING*/

	ArrayTControl[REG_UERRINFTEMP_BF].s_16= 0;
	ArrayTControl[REG_UWARINFTEMP_BF].s_16= 10;
	ArrayTControl[REG_UWARSUPTEMP_BF].s_16= 70;
	ArrayTControl[REG_UERRSUPTEMP_BF].s_16= 80/*48*/;	//Tª crítica
	Trecovery[BF] = /*67*/65 /*ºC*/;

	ArrayTControl[REG_UERRINFTENSION_BF].s_16= 0;
	ArrayTControl[REG_UWARINFTENSION_BF].s_16= 200;
	ArrayTControl[REG_UWARSUPTENSION_BF].s_16= 300;
	ArrayTControl[REG_UERRSUPTENSION_BF].s_16= 400;		//V crítico

	ArrayTControl[REG_UERRSWR_BF].s_16 = 80/*10xdB*/; 	//Error cuando haya menos de 6dBs de dif entre ambas potencias (RL)
	ArrayTControl[REG_UWARSWR_BF].s_16 = 100/*10xdB*/;

	ArrayTControl[REG_UERRPOTFWDINSUF_BF].s_16 = 200;
	ArrayTControl[REG_UWARPOTFWDINSUF_BF].s_16 = 300;

	ArrayTControl[REG_GAN2_BF].s_16 = 0/*mV*/;

	//Flags[BF].ONreciente = 1;

	//********* SET AF PARAMS *********
	Flags[AF].HPA_Stop = 1;
	Flags[AF].System_Stop = 1;
	ArrayTControl[REG_T_AF].s_16 = 41 /*ºC*/;
	ArrayTControl[REG_POTFWDOBJUSER_AF].s_16 = 440/* 10 x dBm*/;
	/*HACER ÉSTO EN EL INIT!!!*/ ArrayTControl[REG_POTFWDOBJREAL_AF].s_16 = ArrayTControl[REG_POTFWDOBJUSER_AF].s_16; //In case the FSM does not execute before power controller at start up
	ArrayTControl[REG_POTPREVIOOBJ_AF].s_16 = 120/*80*//*10xdBm*/;	//BEWARE!! HPA AF BROKEN WITH 13dBms INPUT !!!
	//ArrayTControl[REG_SWR_AF].s_16 = 4;							/*DEBUGGING*/

	ArrayTControl[REG_UERRINFTEMP_AF].s_16= 1;
	ArrayTControl[REG_UWARINFTEMP_AF].s_16= 11;
	ArrayTControl[REG_UWARSUPTEMP_AF].s_16= 71;
	ArrayTControl[REG_UERRSUPTEMP_AF].s_16= 81/*48*/;	//Tª crítica
	Trecovery[AF] = /*67*/65 /*ºC*/;

	ArrayTControl[REG_UERRINFTENSION_AF].s_16= 0;
	ArrayTControl[REG_UWARINFTENSION_AF].s_16= 210;
	ArrayTControl[REG_UWARSUPTENSION_AF].s_16= 330;
	ArrayTControl[REG_UERRSUPTENSION_AF].s_16= 350;		//V crítico

	ArrayTControl[REG_UERRSWR_AF].s_16 = 47/*10xdB*/;	//Error cuando haya menos de 6dBs de dif entre ambas potencias (RL)
	ArrayTControl[REG_UWARSWR_AF].s_16 = 50/*10xdB*/;

	ArrayTControl[REG_UWARPOTFWDINSUF_AF].s_16 = 210;
	ArrayTControl[REG_UERRPOTFWDINSUF_AF].s_16 = 310;

	//Flags[AF].ONreciente = 1;
}



//Task shot only once (on power-up)
void InitSystem (void)
{
	ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_INIT_PARAMS_GEN;
	ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_ARRANCANDO;
	LED_3_POUT |= LED_3_MASK;	//bus
	LED_1_POUT &= ~LED_1_MASK;	//alarms

	//Version FW Sparta (concordando con versiones SAP)
	ArrayTControl[REG_VERSION_FW].s_16 = 0x0000/*PRIMERA release v.0*/;

	ArrayTControl[REG_TIPOTARJETA].s_16 = ERROR_CARD;

	//ArrayTControl[REG_TC_PERFILACTIVO].s_16 |= 0x0001;
	ArrayTControl[REG_MUTE_DCDCs].s_16 = DEFAULT_MUTE_DCDCs;
	ArrayTControl[REG_MUTE_HPAs].s_16 = DEFAULT_MUTE_HPAs;
	Event_post(Event_USB_Port, USB_FRAME_FINISHED);


	Flags[BF].HPA_Stop = 1;
	Flags[BF].System_Stop = 1;
	ArrayTControl[REG_POTPREVIOOBJ_BF].s_16 = 10/*55*/ /*dBm x 10*/;	//con pruebas exhaus -> 55
	ArrayTControl[REG_T_BF].s_16 = 25 /*ºC*/;
	Trecovery[BF] = 65 /*ºC*/;

	Flags[AF].HPA_Stop = 1;
	Flags[AF].System_Stop = 1;
	ArrayTControl[REG_POTPREVIOOBJ_AF].s_16 = 120/*220*//*Platinum 170*//*10xdBm*/;	//!!!OJO!! TRUQUI PLATINUM subir a 170 para alcanzar POT MAX AF!!!!
	ArrayTControl[REG_T_AF].s_16 = 25 /*ºC*/;
	Trecovery[AF] = 65 /*ºC*/;

	//ACCIONES_Regs_Init();		//LOAD DEFAULT PARAMETERS, just in case!!

	//-----------------------------------------------------------
	char Result_ParamsGeneradora = ERROR;
	char Result_ReadyGeneradora = NOTREADY;

	while ( (Result_ParamsGeneradora == ERROR) || (Result_ReadyGeneradora == NOTREADY) || (Result_ReadyGeneradora == ALARMS) || (CardType == ERROR_CARD) )
	{
		//----------------------------------------------------------------------
		if ( CardType == ERROR_CARD )
		{
			CardType = ACCIONES_GetCardType();
			Task_sleep(1000/*ms*/);
		}
		if ( (CardType == BF) || (CardType == AF) )
			while(!(InitialConfig++))
				ACCIONES_CompoundRS485Address();
		//----------------------------------------------------------------------

		//----------------------------------------------------------------------
		Result_ReadyGeneradora = I2C_IsGeneradoraReady();

		if ( (Result_ParamsGeneradora == ERROR) && (Result_ReadyGeneradora == READY) )
			Result_ParamsGeneradora = I2C_LoadParamsFromGeneradoraDoubleCheck(CardType);
		else if ( Result_ReadyGeneradora == ALARMS )
			Result_ParamsGeneradora = I2C_LoadParamsFromGeneradoraDoubleCheck(CardType);
		else if (Result_ParamsGeneradora == OK)
			_NOP();					//Do nothing
		else
			Task_sleep(1000/*ms*/);
		//----------------------------------------------------------------------
	}

	// /*DEBUGGING!! --------->*/	ACCIONES_Regs_Init(); //	<--------DEBUGGING!!!
	ADC12_Config ();

	I2C_StartGeneradora ();
	POWER_System_Start (CardType);
	//-----------------------------------------------------------


	/*!!! HACER ÉSTO EN EL INIT!!!*/ ArrayTControl[REG_POTFWDOBJREAL_BF].s_16 = ArrayTControl[REG_POTFWDOBJUSER_BF].s_16 - 30 /*CAMBIO UCO(no sol)!!*/; 	//In case the FSM does not execute before power controller at start up
	/*!!! HACER ÉSTO EN EL INIT!!!*/ ArrayTControl[REG_POTFWDOBJREAL_AF].s_16 = ArrayTControl[REG_POTFWDOBJUSER_AF].s_16 - 30 /*CAMBIO UCO(no sol)!!*/; 	//In case the FSM does not execute before power controller at start up

	USBCDC_rejectData (CDC0_INTFNUM);		//Flush usb buffer just in case
	ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_ARRANCANDO;

	LED_3_POUT &= ~LED_3_MASK;	//bus
	LED_1_POUT &= ~LED_1_MASK;	//alarms
}


int ACCIONES_GetCardType ()
{
	P5DIR &= ~(1<<3);
	P5DIR &= ~(1<<4);

	char p53 = (P5IN & (1<<3));
	char p54 = (P5IN & (1<<4));

	if ( (p53 == 0) && (p54 == 0) )
	{
		ArrayTControl[REG_TIPOTARJETA].s_16 = AF;
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 &= ~CRITICAL_READ_CARDTYPE;
		return AF;	//AF
	}
	else if ( (p53 == (1<<3)) && (p54 == (1<<4)) )
	{
		ArrayTControl[REG_TIPOTARJETA].s_16 = BF;
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 &= ~CRITICAL_READ_CARDTYPE;
		return BF;	//BF
	}
	else
	{
		ArrayTControl[REG_TIPOTARJETA].s_16 = ERROR_CARD;
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_READ_CARDTYPE;
		return 2;	//Error
	}
}


void ACCIONES_CompoundRS485Address (void)
{
	ArrayTControl[REG_DIRECCION_RS485].s_16 = ( (ArrayTControl[REG_TIPOTARJETA].s_16 & 0x000F) << 12 ) | ( ArrayTControl[REG_NUMSERIE].s_16 & 0x0FFF );
	_NOP();
}

























void ACCIONES_InitRegisters (void)
{
	//************************* INIT REGS TARJETA CONTROL *************************
	ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 = DEFAULT_TC_ESTADOOPERATIVO;
	ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 = DEFAULT_TC_ALARMASACTIVASTC;
	ArrayTControl[REG_TC_ALARMASNORECTC].s_16 = DEFAULT_TC_ALARMASNORECTC;
	ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 = DEFAULT_TC_ALARMASACTIVASHPABF;
	ArrayTControl[REG_TC_ALARMASNORECHPABF].s_16 = DEFAULT_TC_ALARMASNORECHPABF;
	ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 = DEFAULT_TC_ALARMASACTIVASHPAAF;
	ArrayTControl[REG_TC_ALARMASNORECHPAAF].s_16 = DEFAULT_TC_ALARMASNORECHPAAF;
	ArrayTControl[REG_TC_PERFILACTIVO].s_16 = DEFAULT_TC_PERFILACTIVO;
	ArrayTControl[REG_POTFWDOBJREAL_BF].s_16 = DEFAULT_POTFWDOBJREAL_BF;
	ArrayTControl[REG_POTFWD_BF].s_16 = DEFAULT_POTFWD_BF;
	ArrayTControl[REG_POTRVS_BF].s_16 = DEFAULT_POTRVS_BF;
	ArrayTControl[REG_SWR_BF].s_16 = DEFAULT_SWR_BF;
	ArrayTControl[REG_T_BF].s_16 = DEFAULT_T_BF;
	ArrayTControl[REG_ATTTOT_BF].s_16 = DEFAULT_ATTTOT_BF;
	ArrayTControl[REG_GAN2_BF].s_16 = DEFAULT_GAN2_BF;
	ArrayTControl[REG_POT1PREVIO_BF].s_16 = DEFAULT_POT1PREVIO_BF;
	ArrayTControl[REG_POTFWDOBJREAL_AF].s_16 = DEFAULT_POTFWDOBJREAL_AF;
	ArrayTControl[REG_POTFWD_AF].s_16 = DEFAULT_POTFWD_AF;
	ArrayTControl[REG_POTRVS_AF].s_16 = DEFAULT_POTRVS_AF;
	ArrayTControl[REG_SWR_AF].s_16 = DEFAULT_SWR_AF;
	ArrayTControl[REG_T_AF].s_16 = DEFAULT_T_AF;
	ArrayTControl[REG_ATTTOT_AF].s_16 = DEFAULT_ATTTOT_AF;
	ArrayTControl[REG_ATT2_AF].s_16 = DEFAULT_ATT2_AF;
	ArrayTControl[REG_POT1PREVIO_AF].s_16 = DEFAULT_POT1PREVIO_AF;
	//ArrayTControl[REG_RELES_HPAs].s_16 = DEFAULT_RELES_HPAs;
	ArrayTControl[REG_MUTE_HPAs].s_16 = DEFAULT_MUTE_HPAs;
	ArrayTControl[REG_POTFWDOBJUSER_BF].s_16 = DEFAULT_POTFWDOBJUSER_BF;
	ArrayTControl[REG_UWARPOTFWDINSUF_BF].s_16 = DEFAULT_UWARPOTFWDINSUF_BF;
	ArrayTControl[REG_UERRPOTFWDINSUF_BF].s_16 = DEFAULT_UERRPOTFWDINSUF_BF;
	ArrayTControl[REG_UWARSWR_BF].s_16 = DEFAULT_UWARSWR_BF;
	ArrayTControl[REG_UERRSWR_BF].s_16 = DEFAULT_UERRSWR_BF;
	ArrayTControl[REG_UWARINFTEMP_BF].s_16 = DEFAULT_UWARINFTEMP_BF;
	ArrayTControl[REG_UERRINFTEMP_BF].s_16 = DEFAULT_UERRINFTEMP_BF;
	ArrayTControl[REG_UWARSUPTEMP_BF].s_16 = DEFAULT_UWARSUPTEMP_BF;
	ArrayTControl[REG_UERRSUPTEMP_BF].s_16 = DEFAULT_UERRSUPTEMP_BF;
	ArrayTControl[REG_POTFWDOBJUSER_AF].s_16 = DEFAULT_POTFWDOBJUSER_AF;
	ArrayTControl[REG_UWARPOTFWDINSUF_AF].s_16 = DEFAULT_UWARPOTFWDINSUF_AF;
	ArrayTControl[REG_UERRPOTFWDINSUF_AF].s_16 = DEFAULT_UERRPOTFWDINSUF_AF;
	ArrayTControl[REG_UWARSWR_AF].s_16 = DEFAULT_UWARSWR_AF;
	ArrayTControl[REG_UERRSWR_AF].s_16 = DEFAULT_UERRSWR_AF;
	ArrayTControl[REG_UWARINFTEMP_AF].s_16 = DEFAULT_UWARINFTEMP_AF;
	ArrayTControl[REG_UERRINFTEMP_AF].s_16 = DEFAULT_UERRINFTEMP_AF;
	ArrayTControl[REG_UWARSUPTEMP_AF].s_16 = DEFAULT_UWARSUPTEMP_AF;
	ArrayTControl[REG_UERRSUPTEMP_AF].s_16 = DEFAULT_UERRSUPTEMP_AF;
	ArrayTControl[REG_CODIGOMODULO1].s_16 = DEFAULT_CODIGOMODULO1;
	ArrayTControl[REG_CODIGOMODULO2].s_16 = DEFAULT_CODIGOMODULO2;
	ArrayTControl[REG_MESANYO].s_16 = DEFAULT_MESANYO;
	ArrayTControl[REG_NUMSERIE].s_16 = DEFAULT_NUMSERIE;
	ArrayTControl[REG_VERSION_HW].s_16 = DEFAULT_VERSION_HW;
	ArrayTControl[REG_VERSION_FW].s_16 = DEFAULT_VERSION_FW;
	ArrayTControl[REG_POTPREVIOOBJ_BF].s_16 = DEFAULT_POTPREVIOOBJ_BF;
	ArrayTControl[REG_POTPREVIOOBJ_AF].s_16 = DEFAULT_POTPREVIOOBJ_AF;

	//************************* INIT REGS GENERADORA *************************
//	ArrayGen[REG_GEN_ESTADOOPERATIVO].s_16 = DEFAULT_GEN_ESTADOOPERATIVO;
//	ArrayGen[REG_GEN_ALARMASACTIVAS].s_16 = DEFAULT_GEN_ALARMASACTIVAS;
//	ArrayGen[REG_GEN_ALARMASNOREC].s_16 = DEFAULT_GEN_ALARMASNOREC;
//	ArrayGen[REG_GEN_PERFILACTIVO].s_16 = DEFAULT_GEN_PERFILACTIVO;
//	ArrayGen[REG_GEN_MASCARAPERFILES].s_16 = DEFAULT_GEN_MASCARAPERFILES;
//	ArrayGen[REG_GEN_PROGCAMBIOPERFIL].s_16 = DEFAULT_GEN_PROGCAMBIOPERFIL;
//	ArrayGen[REG_GEN_CLAVECIFRADO].s_16 = DEFAULT_GEN_CLAVECIFRADO;
//	ArrayGen[REG_GEN_IDPLANINHIBICION].s_16 = DEFAULT_GEN_IDPLANINHIBICION;
//	ArrayGen[REG_GEN_TEMPDAC].s_16 = DEFAULT_GEN_TEMPDAC;
//	ArrayGen[REG_GEN_NUMSERIE].s_16 = DEFAULT_GEN_NUMSERIE;
//	ArrayGen[REG_GEN_VERSIONHW].s_16 = DEFAULT_GEN_VERSIONHW;
//	ArrayGen[REG_GEN_VERSIONFW].s_16 = DEFAULT_GEN_VERSIONFW;
//	ArrayGen[REG_GEN_LONGINFOUSUARIO].s_16 = DEFAULT_GEN_LONGINFOUSUARIO;
//	ArrayGen[REG_GEN_INFOUSUARIO].s_16 = DEFAULT_GEN_INFOUSUARIO;
//	ArrayGen[REG_GEN_LONGSEMILLA].s_16 = DEFAULT_GEN_LONGSEMILLA;
//	ArrayGen[REG_GEN_LRCSEMILLA].s_16 = DEFAULT_GEN_LRCSEMILLA;
//	ArrayGen[REG_GEN_COMANDO].s_16 = DEFAULT_GEN_COMANDO;
}
