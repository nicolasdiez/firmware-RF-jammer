/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: Power.c										*/
/********************************************************/

#include <string.h>
#include "HAL.h"
#include "Power.h"
#include <string.h>
#include "HAL.h"
//#include "Acciones2.h"
#include "FSM.h"
#include "ATT.h"
#include "PowerController.h"

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
extern union register_entry ArrayTControl[TC_NUM_REGS];				//Nico: Cada slot de estos [] ocupa 2bytes
//extern union register_entry ArrayGen[GEN_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2byteschar Flag_System_Stop[2] = {1,1};

//extern char Flag_System_Stop[2];
//extern char Flag_HPA_Stop[2];
extern char Flag_ControllerPrevio_ON[2];
//extern unsigned short PowerPrevioController_counter[2];
extern signed short ATT_MIN_PREVIO[2];	//La Att mínima calculada al inicio para que haya la Pot necesaria en el previo (ArrayTControl[REG_POTPREVIOOBJ_BF])

extern int EstadoTemp[2];
extern int EstadoSWR[2];
extern int CardType;

//------------------------------------------- FUCNTION METHODS ------------------------------------------
void POWER_System_Stop (int HPA)
{
	if ( (HPA == BF) && (CardType == BF) )
	{
		//Flag_ControllerPrevio_ON[BF] = 0;  	//Por si acaso sigue rulando el controller previo
		Flags[BF].System_Stop = 1;
		POWER_HPA_Stop (BF);
		EstadoTemp[BF] = ESTADO_0;
		EstadoSWR[BF] = ESTADO_0;
	}
	else if ( (HPA == AF) && (CardType == AF) )
	{
		//Flag_ControllerPrevio_ON[AF] = 0;
		Flags[AF].System_Stop = 1;
		POWER_HPA_Stop (AF);
		EstadoTemp[AF] = ESTADO_0;
		EstadoSWR[AF] = ESTADO_0;
	}

	if ( (Flags[BF].System_Stop == 1) && (Flags[AF].System_Stop == 1) )
	{
		//Turn off fans (NOT in Sparta)
		//AUX_1_POUT &= ~AUX_1_MASK;
		//AUX_2_POUT &= ~AUX_2_MASK;
		//AUX_3_POUT &= ~AUX_3_MASK;
	}
}

void POWER_HPA_Stop (int HPA)
{
	if (HPA == BF)
	{
		if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_AUTOTEST) != ST_AUTOTEST )
		{
			MUTE_HPA1_POUT |= MUTE_HPA1_MASK;
			MUTE_DCDC_BF_POUT |= MUTE_DCDC_BF_MASK;
			ArrayTControl[REG_MUTE_HPAs].s_16 |= 0x0001;
			ArrayTControl[REG_MUTE_DCDCs].s_16 |= 0x0001;
			Flags[BF].HPA_Stop = 1;
			Flags[BF].ONreciente = 1;
			Flags[BF].PrevioOK_StartHPA = 0;
			LED_2_POUT &= ~LED_2_MASK;				//On-Off LED - Sync LEDs
			LED_1_POUT &= ~LED_1_MASK;				//Alarms LED - Sync LEDs
		}
	}
	else if (HPA == AF)
	{
		if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_AUTOTEST) != ST_AUTOTEST )
		{
			MUTE_HPA2_POUT |= MUTE_HPA2_MASK;
			//MUTE_DCDC_AF_POUT |= MUTE_DCDC_AF_MASK;
			MUTE_DCDC_BF_POUT |= MUTE_DCDC_BF_MASK;
			ArrayTControl[REG_MUTE_HPAs].s_16 |= 0x0002;
			ArrayTControl[REG_MUTE_DCDCs].s_16 |= 0x0002;
			Flags[AF].HPA_Stop = 1;
			Flags[AF].ONreciente = 1;
			Flags[AF].PrevioOK_StartHPA = 0;
			LED_2_POUT &= ~LED_2_MASK;				//On-Off LED - Sync LEDs
			LED_1_POUT &= ~LED_1_MASK;				//Alarms LED - Sync LEDs
		}
	}
}


void POWER_System_Start (int HPA)
{
	//Estudiar si meter aquí I2C_Config(), UART_Config()...

	if( (HPA == BF) && (CardType == BF) )
	{
		if ( POWER_HPA_Start (BF) == ERROR)
		{
			POWER_HPA_Stop (BF);
			Flags[BF].System_Stop = 1;			//if system manages to start in HPA_Start() this flag will be set down
		}
		else
			Flags[BF].System_Stop = 0;							//Consider system ON only if HPA on

	}
	else if( (HPA == AF) && (CardType == AF) )
	{
		if ( POWER_HPA_Start (AF) == ERROR)	//HPA failed to start
		{
			POWER_HPA_Stop (AF);
			Flags[BF].System_Stop = 1;	//if system manages to start in HPA_Start() this flag is set down
		}
		else	//HPA Started OK
			Flags[AF].System_Stop = 0;						//Consider system ON only if HPA on
	}

	if ( (Flags[BF].System_Stop == 0) || (Flags[AF].System_Stop == 0) )
	{
		 //Turn on fans (NOT in Sparta)
		 //AUX_1_POUT |= AUX_1_MASK;
		 //AUX_2_POUT |= AUX_2_MASK;
		 //AUX_3_POUT |= AUX_3_MASK;
	}
}


/*void*/int POWER_HPA_Start (int HPA)
{
	UInt posted;

	if (HPA == BF)
	{
		Flags[BF].ControllerPrevio_ON = 1;
		Flags[BF].PowerPrevioController_counter = 0;

		posted = Event_pend(Event_PrevPowerControllerBF, Event_Id_NONE, PREVCONTROLLER_BF_FINISHED + PREVCONTROLLER_BF_POTPREVIO_EXCES + PREVCONTROLLER_BF_PREVIOERROR, 10000/*ms*/);

		if (posted & PREVCONTROLLER_BF_FINISHED)
		{
			if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_AUTOTEST) != ST_AUTOTEST )
			{
				Flags[BF].ControllerPrevio_ON = 0;
				ATT_SetTotalAttenuation (ATT_MAX , BF);

				MUTE_DCDC_BF_POUT &= ~MUTE_DCDC_BF_MASK;
				MUTE_HPA1_POUT &= ~MUTE_HPA1_MASK;

				Flags[BF].HPA_Stop = 0;									//Flag used to run ACCIONES_Actuar
				//Flags[BF].System_Stop = 0;							//Consider system ON only if HPA on
				ArrayTControl[REG_MUTE_HPAs].s_16 &= ~0x0001;
				ArrayTControl[REG_MUTE_DCDCs].s_16 &= ~0x0001;
				ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_SWR;
				ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_T;
				return OK;
			}
			else
			{
				//Flags[BF].System_Stop = 1;	//si no se encendió la HPA considero system off
				//POWER_HPA_Stop (BF);
				Flags[BF].ControllerPrevio_ON = 0;
				return ERROR;
			}
		}
		if (posted & PREVCONTROLLER_BF_POTPREVIO_EXCES)
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 |= CRITICAL_POTPREVIO_EXCES;
			//Flags[BF].System_Stop = 1;
			//POWER_HPA_Stop (BF);
			Flags[BF].ControllerPrevio_ON = 0;
			return ERROR;
		}
		if (posted & PREVCONTROLLER_BF_PREVIOERROR)
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 |= CRITICAL_POT_PREVIO;
			//Flags[BF].System_Stop = 1;
			//POWER_HPA_Stop (BF);
			Flags[BF].ControllerPrevio_ON = 0;
			return ERROR;
		}
		if (posted == 0)
		{
			_NOP();	//Timeout. Do nothing.
			//Flags[BF].System_Stop = 1;
			//POWER_HPA_Stop (BF);
			Flags[BF].ControllerPrevio_ON = 0;
			return ERROR;
		}

		Flags[BF].ONreciente = 0;
	}

	else if (HPA == AF)
	{
		Flags[AF].ControllerPrevio_ON = 1;
		Flags[AF].PowerPrevioController_counter = 0;

		posted = Event_pend(Event_PrevPowerControllerAF, Event_Id_NONE, PREVCONTROLLER_AF_FINISHED + PREVCONTROLLER_AF_POTPREVIO_EXCES + PREVCONTROLLER_AF_PREVIOERROR, 10000/*ms*/);

		if (posted & PREVCONTROLLER_AF_FINISHED)
		{
			if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_AUTOTEST) != ST_AUTOTEST )
			{
				Flags[AF].ControllerPrevio_ON = 0;
				ATT_SetTotalAttenuation (2*ATT_MAX , AF);

				//MUTE_DCDC_AF_POUT &= ~MUTE_DCDC_AF_MASK;
				MUTE_DCDC_BF_POUT &= ~MUTE_DCDC_BF_MASK;		//BF and AF share DCDC mute pin
				MUTE_HPA2_POUT &= ~MUTE_HPA2_MASK;

				Flags[AF].HPA_Stop = 0;							//Flag used to run ACCIONES_Actuar
				//Flags[AF].System_Stop = 0;						//Consider system ON only if HPA on
				ArrayTControl[REG_MUTE_HPAs].s_16 &= ~0x0002;
				ArrayTControl[REG_MUTE_DCDCs].s_16 &= ~0x0002;
				ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_SWR;	//OJO! TRUQUI! (cambio día UCO (únicamente ésto soluciona SÓLO rearme swr en perfil v3))
				ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_T;
				return OK;
			}
			else
			{
				//Flags[AF].System_Stop = 1;	//si no se encendió la HPA considero system off
				//POWER_HPA_Stop (AF);
				Flags[AF].ControllerPrevio_ON = 0;
				return ERROR;
			}
		}
		if (posted & PREVCONTROLLER_AF_POTPREVIO_EXCES)
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 |= CRITICAL_POTPREVIO_EXCES;
			//Flags[AF].System_Stop = 1;
			//POWER_HPA_Stop (AF);
			Flags[AF].ControllerPrevio_ON = 0;
			return ERROR;
		}
		if (posted & PREVCONTROLLER_AF_PREVIOERROR)
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 |= CRITICAL_POT_PREVIO;
			//Flags[AF].System_Stop = 1;
			//POWER_HPA_Stop (AF);
			Flags[AF].ControllerPrevio_ON = 0;
			return ERROR;
		}
		if (posted == 0)
		{
			_NOP();	//Timeout. Do nothing.
			//Flags[AF].System_Stop = 1;
			//POWER_HPA_Stop (AF);
			Flags[AF].ControllerPrevio_ON = 0;
			return ERROR;
		}

		Flags[AF].ONreciente = 0;
	}
}



void POWER_ControlledReset(unsigned char type)
{
  //-->ctl_timeout_wait( ctl_get_current_time() +  MACRO_TIME_MILS(1000) );		// 1Seg
  //__delay_cycles(24000000);
  _DINT();
  WDTCTL= 0x1234/*WDTPW*/;	//Illegal PW write
  while(1);
}
