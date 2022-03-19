/********************************************************/
/* Author: Nicolás Díez							 		*/
/* Date: July 2014										*/
/* File: Acciones.c										*/
/********************************************************/

//#include <ctl_api.h>
//#include "ctl.h"
//#include <cross_studio_io.h>
#include <string.h>
#include "HAL.h"
#include "ADC.h"
#include "Acciones.h"
#include "ATT.h"
#include "Ports.h"
//#include "EstadoOperativo.h"
//#include "FlashConfig.h"

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

//void RTOS_Acciones_TaskInit(void);
//void RTOS_Acciones(UArg arg0, UArg arg1/*void *p*/);
//#include "stdlib.h"

#ifdef __DEBUG
//-->#include "debug.h"
#endif



//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
//-->CTL_TASK_t AccionesTask;
//-->extern CTL_EVENT_SET_t AdcAllSampled;

unsigned long MaskAlarm[2] = {0};					//MaskAlarm[0] for BF - MaskAlarm[1] for AF
extern FlagAlarms Flags[2];							//Flags[0] for BF & Flags[1] for AF
unsigned char contador_arranque = 10;
extern unsigned char signatureWDT;
//extern PARAM_INFO param_flash;
extern unsigned int Flag_I2C_Comm_Completed;

//#pragma location=0x15940
//union register_entry ArrayTControl[TC_NUM_REGS];			//Nico: Cada slot de estos [0x004A] ocupa 2bytes

//#pragma location=0x16500
//union register_entry ArrayGen[GEN_NUM_REGS];				//Nico: Cada slot de estos [0x00D0] ocupa 2bytes

extern PDU_I2C pdu_I2C;		//Main declaration in RS485.c

extern union register_entry ArrayTControl[TC_NUM_REGS];				//Nico: Cada slot de estos [] ocupa 2bytes
extern union register_entry ArrayGen[GEN_NUM_REGS];					//Nico: Cada slot de estos [] ocupa 2bytes

//unsigned AccionesStack[1+ACCIONESSTACKSIZE+1] = {0};
//parameters param[2];
//extern unsigned char soyBF;
//extern unsigned char atenuacion_temporal;
//-------------------------------------------------------------------------------------------------

//---------------- TASK & EVENTS HANDLERS-----------------
Task_Handle tskhdl_Acciones;
//extern Event_Handle Event_ADC;



//---------------------------------------- FUCNTION METHODS ----------------------------------------
void ACCIONES_RTOS_Init(void)
{
  //------------------- EVENT ADC SAMPLE ----------------------------
  /*Error_Block eb;
  Error_init(&eb);
  if ( !(Event_ADC = Event_create(NULL, &eb)) )
	  System_abort("Event ADC create failed");
  Event_post(Event_ADC, NO_ALLSAMPLED);*/

  //------------------- TASK CREATION ----------------------------
  /* Create task*/
//  Error_Block eb;
//  Error_init(&eb);
//  Task_Params taskParams;
//  Task_Params_init(&taskParams);
//  taskParams.priority = 7/*1*/;					//Lowest priority: 0 , Highest priorit: 31
//  taskParams.stackSize = 384 /*384*/;	 	// Task stack size in MAUs (MINIMUM ADDRESSABLE UNIT !)
//  tskhdl_Acciones = Task_create (ACCIONES_RTOS_Task, &taskParams, &eb);
//  if (tskhdl_Acciones == NULL)
//	  System_abort("Task 'Acciones' create failed");

  //-----------------------------------------------------------
  //memset(AccionesStack, 0xcd, sizeof(AccionesStack));					//PREGUNTAR: Por que se escribe 0xcd en todo el array AccionesStack??
  //AccionesStack[0]=AccionesStack[1+ACCIONESSTACKSIZE]=0xfacefeed; 	// put marker values at the words before/after the stack	//PREGUNTAR: Por qué se marca con ese valor?

  //ctl_events_init initializes the event set "AdcAllSampled" with the "NO_ALLSAMPLED" value
  //-->ctl_events_init(&AdcAllSampled,NO_ALLSAMPLED);

  //ctl_task_run takes a pointer in AccionesTask to the CTL_TASK_t structure that represents the task
  //-->ctl_task_run(&AccionesTask, 10, Acciones, 0, "AC", ACCIONESSTACKSIZE, AccionesStack+1, CALLSTACKSIZE); // Modem Driver


	//Debugging
	ArrayTControl[REG_UERRINFTEMP_BF].s_16= 0;
	ArrayTControl[REG_UWARINFTEMP_BF].s_16= 10;
	ArrayTControl[REG_UWARSUPTEMP_BF].s_16= 75;
	ArrayTControl[REG_UERRSUPTEMP_BF].s_16= 80;
	Flags[BF].ONreciente = 1;
}



void ACCIONES_RTOS_Task(UArg arg0, UArg arg1/*void *p*/)
{
  int hpa = BF;
  int ATTSelect = SEL_ATT_PREVIO_BF;

  #ifdef __DEBUG_CROSSWORKS
  	  //-->debug_printf("Tarea Acciones\n");
  #endif

  while (1)
  {
	#ifdef __WDT_ENABLED
	  //Log_info0("Signing Watchdog: Task Acciones");
	  signatureWDT |= SIGNATURE_TASK_ACCIONES;   // Firmamos
	  _NOP();
	#endif


	  //Nico: Si se termina de samplear y convertir todo el ADC (ctl_events_wait devuelve !=0 si se produce el evento)
	  //ctl_events_wait waits for ALLSAMPLED to be set (value 1) in the event set pointed to by AdcAllSampled
	  //-->if(ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR, &AdcAllSampled, ALLSAMPLED, 1, ctl_get_current_time()+ MACRO_TIME_MILS(1000)) != 0)
	  //-->{
	  if( Event_pend(Event_ADC, ALLSAMPLED, Event_Id_NONE, 100000/*MACRO_TIME_MILS(1000)*/) != 0)
	  {
		  //Se han leido todos los canales. Marco alarmas y actuo en consecuencia.
		  //ACCIONES_MarcarAlarmas();

		  //ACCIONES_LoadParamsFromGeneradora();
		  ACCIONES_MarcarAlarmas_PotSWRT(BF);
		  //ACCIONES_MarcarAlarmas_PotSWRT(AF);
		  ACCIONES_MarcarEstadoOperativo();
		  ACCIONES_Actuar(BF);
		  //ACCIONES_Actuar(AF);
		  //ACCIONES_AdjustFwdPower(BF);
		  //ACCIONES_AdjustFwdPower(AF);

		  /*if(Flags[hpa].ONreciente == 1) // && (StandBy == 0))		//PREGUNTAR: Qué implica ONreciente?
		  {
			  contador_arranque--;									//PREGUNTAR: Qué es contador_arranque? Porque se procede así?

			  if(contador_arranque == 0)
			  {
				  ATT_SetAttenuation(0, ATTSelect);
				  Flags[hpa].ONreciente=0;
			  }
			  else if(contador_arranque < 5)
				  ATT_SetAttenuation(80, ATTSelect);   							//PREGUNTAR: porque se atenúa 80 aquí?
		  }*/
	  //-->}
	  }
  }
}


void ACCIONES_MarcarAlarmas_PotSWRT (unsigned int hpa)
{
    unsigned short reg_swr;
    unsigned short reg_uerrswr;
    unsigned short reg_potfwd;
    unsigned short reg_uwarswr;
    unsigned short reg_uerrpotfwdinsuf;
    unsigned short reg_uwarpotfwdinsuf;
    unsigned short reg_uwarinftemp;
    unsigned short reg_t;
    unsigned short reg_uwarsuptemp;
    unsigned short reg_uerrinftemp;
    unsigned short reg_uerrsuptemp;
    unsigned short ATTSelect;

	if (hpa == BF)
	{
	    reg_swr = REG_SWR_BF;
	    reg_uerrswr = REG_UERRSWR_BF;
	    reg_potfwd = REG_POTFWD_BF;
	    reg_uwarswr = REG_UWARSWR_BF;
	    reg_uerrpotfwdinsuf = REG_UERRPOTFWDINSUF_BF;
	    reg_uwarpotfwdinsuf = REG_UWARPOTFWDINSUF_BF;
	    reg_uwarinftemp = REG_UWARINFTEMP_BF;
	    reg_t = REG_T_BF;
	    reg_uwarsuptemp = REG_UWARSUPTEMP_BF;
	    reg_uerrinftemp = REG_UERRINFTEMP_BF;
	    reg_uerrsuptemp = REG_UERRSUPTEMP_BF;
	    ATTSelect = SEL_ATT_PREVIO_BF;
	}
	else if (hpa == AF)
	{
	    reg_swr = REG_SWR_AF;
	    reg_uerrswr = REG_UERRSWR_AF;
	    reg_potfwd = REG_POTFWD_AF;
	    reg_uwarswr = REG_UWARSWR_AF;
	    reg_potfwd = REG_POTFWD_AF;
	    reg_uerrpotfwdinsuf = REG_UERRPOTFWDINSUF_AF;
	    reg_uwarpotfwdinsuf = REG_UWARPOTFWDINSUF_AF;
	    reg_uwarinftemp = REG_UWARINFTEMP_AF;
	    reg_t = REG_T_AF;
	    reg_uwarsuptemp = REG_UWARSUPTEMP_AF;
	    reg_uerrinftemp = REG_UERRINFTEMP_AF;
	    reg_uerrsuptemp = REG_UERRSUPTEMP_AF;
	    ATTSelect = SEL_ATT_PREVIO_AF;
	}

	//int i = 0;
	if(Flags[hpa].ONreciente == 0)  // Si SW ON y llevan encendidos un rato
	{
	    //SWR CRITICAL - Condición invertida -> Alarma salta si SWR < SWR_UMBRAL_ERROR
		if((ArrayTControl[reg_swr].s_16 < ArrayTControl[reg_uerrswr].s_16) && (ArrayTControl[reg_potfwd].s_16 > 300))     // Marco alarma critica de la relacion de SWR=FWD-RVS // (FWD > 300(30dBm)) ->
	    {									//PREGUNTAR: Porque condicion de Pot incidente > 30dBm y no se usa umbral warning SWR? -> Mínima Pot incidente(?)
	          Flags[hpa].WarSWR=0;
	          Flags[hpa].OKSWR=0;
	          //COMPRUEBA DIEZ VECES			//PREGUNTAR: Por que check 10 veces??
	          if(Flags[hpa].ErrorSWR<10)
	        	  Flags[hpa].ErrorSWR++;
	          else
	    	  {
	        	  MaskAlarm[hpa] &= ~WARNING_SWR;
	        	  MaskAlarm[hpa] |= CRITICAL_SWR;
	        	  //ATENUAMOS AL MAXIMO PARA PROTEGER EL EQUIPO
	        	  ATT_SetAttenuation(ATT_MAX, ATTSelect);
	        	  //ATT_SetAttenuation (signed long bAtt, int i, int ATTSelect);

	        	  //COMO HEMOS CAMBIADO LA ATENUACION, HACEMOS QUE EL SENSOR HALL Y EL SENSOR DE TEMPERATURA ESPEREN UN TIEMPO PARA QUE PUEDAN ACTUALIZAR SU VALOR
	        	  if(Flags[hpa].CriticalSWR<1)
	        	  {
	        		  //WarHALL=0;
	        		  Flags[hpa].WarTEMP=0;
	        	  }
	        	  Flags[hpa].CriticalSWR=1;
	    	  }
	    }

		//SWR WARNING - Condición invertida (!! NICO CAMBIO EL ORDER DE LOS IF´S WARNING Y ERROR)
		else if((ArrayTControl[reg_swr].s_16 < ArrayTControl[reg_uwarswr].s_16) && (ArrayTControl[reg_potfwd].s_16 > 300))	// Marco alarma warning de la relacion de SWR=FWD-RVS
	    {
	          Flags[hpa].ErrorSWR=0;
	          Flags[hpa].OKSWR=0;
	          //COMPRUEBA DIEZ VECES
	          if(Flags[hpa].WarSWR<10)
	        	  Flags[hpa].WarSWR++;
	          else
	          {
	        	  MaskAlarm[hpa] &= ~CRITICAL_SWR;
	        	  MaskAlarm[hpa] |= WARNING_SWR;
	        	  Flags[hpa].CriticalSWR=0;
	          }
	    }

	    //SWR OK - Condición invertida
	    else if(ArrayTControl[reg_swr].s_16 >= ArrayTControl[reg_uwarswr].s_16)
	    {
	          Flags[hpa].WarSWR=0;
	          Flags[hpa].ErrorSWR=0;
	          //COMPRUEBA DIEZ VECES
	          if(Flags[hpa].OKSWR<10)
	        	  Flags[hpa].OKSWR++;
	          else
	          {
	        	  MaskAlarm[hpa] &= ~WARNING_SWR;
	        	  MaskAlarm[hpa] &= ~CRITICAL_SWR;
	        	  Flags[hpa].CriticalSWR=0;
	          }
	    }


	    if(Flags[hpa].StandBy == 0)					//PREGUNTAR: Que implica el flag StandBy ?? -> Es == 0 cuando NO está enclavado (etapas encendidas)
	    {
	    	  //FWD CRITICAL - Potencia incidente insuficiente
	    	  if(ArrayTControl[reg_potfwd].s_16 < ArrayTControl[reg_uerrpotfwdinsuf].s_16)				// Marco alarma critica de Potencia Incidente Insuficiente
	    	  {
	    		  Flags[hpa].WarPII=0;
	    		  Flags[hpa].OKPII=0;
	    		  //COMPRUEBA DIEZ VECES
	    		  if(Flags[hpa].ErrorPII<10)
	    		  Flags[hpa].ErrorPII++;
	    		  else
	    		  {
	    			  MaskAlarm[hpa] &= ~WARNING_POT_INS;
	    			  MaskAlarm[hpa] |= CRITICAL_POT_INS;
	    		  }
	    	  }

	    	  //FWD WARNING - Potencia incidente insuficiente  (!! NICO CAMBIO EL ORDER DE LOS IF´S WARNING Y ERROR)
	    	  else if(ArrayTControl[reg_potfwd].s_16 < ArrayTControl[reg_uwarpotfwdinsuf].s_16)  		// Marco alarma warning de Potencia Incidente Insuficiente
			  {
				  Flags[hpa].ErrorPII=0;
				  Flags[hpa].OKPII=0;
				  //COMPRUEBA DIEZ VECES
				  if(Flags[hpa].WarPII<10)
				  Flags[hpa].WarPII++;
				  else
				  {
					  MaskAlarm[hpa] &= ~CRITICAL_POT_INS;
					  MaskAlarm[hpa] |= WARNING_POT_INS;
				  }
			  }

			  //FWD OK
			  else										//PREGUNTAR: Por que no comprobamos que es mayor que Pot inci insuf?
			  {
				  Flags[hpa].WarPII=0;
				  Flags[hpa].ErrorPII=0;
				  //COMPRUEBA DIEZ VECES
				  if(Flags[hpa].OKPII<10)
					  Flags[hpa].OKPII++;
				  else
				  {
					  MaskAlarm[hpa] &= ~WARNING_POT_INS;
					  MaskAlarm[hpa] &= ~CRITICAL_POT_INS;
				  }
			  }
	    }

	    //Nico: Si (param[i].StandBy == 1)			//PREGUNTAR: el flag StandBy para que se usa? -> Es == 1 cuando está ENCLAVADO (etapas apagadas)
	    else
	    {
	    	  MaskAlarm[hpa] &= ~WARNING_POT_INS;
	    	  MaskAlarm[hpa] &= ~CRITICAL_POT_INS;
	    }
	 }


	// TEMPERATUA CRÍTICA - Marco alarma critica de TEMPERATURA
	if( (ArrayTControl[reg_t].s_16 < ArrayTControl[reg_uerrinftemp].s_16) || (ArrayTControl[reg_t].s_16 > ArrayTControl[reg_uerrsuptemp].s_16) )
	{
	    Flags[hpa].WarTEMP=0;
	    Flags[hpa].OKTEMP=0;
	    Flags[hpa].ErrorTEMP++;
	    //COMPRUEBA 60 VECES
	    if(Flags[hpa].ErrorTEMP>=60)
	    {
	    	MaskAlarm[hpa] &= ~WARNING_T;
	    	MaskAlarm[hpa] |= CRITICAL_T;
	    }
	}

	// TEMPERATUA WARNING - Marco alarma warning de Temperatura (!! NICO CAMBIO EL ORDER DE LOS IF´S WARNING Y ERROR)
	else if( (ArrayTControl[reg_t].s_16 < ArrayTControl[reg_uwarinftemp].s_16) || (ArrayTControl[reg_t].s_16 > ArrayTControl[reg_uwarsuptemp].s_16) ) 			//PREGUNTAR: No habría que check que T<T_critica?
	{
	    Flags[hpa].ErrorTEMP=0;
	    Flags[hpa].OKTEMP=0;
	    Flags[hpa].WarTEMP++;
	    //COMPRUEBA 60 VECES
	    if(Flags[hpa].WarTEMP>=60)
	    {
	    	MaskAlarm[hpa] &= ~CRITICAL_T;
	    	MaskAlarm[hpa] |= WARNING_T;
	    }
	}

	// TEMPERATUA OK
	else
	{
        Flags[hpa].WarTEMP=0;
        Flags[hpa].ErrorTEMP=0;
        Flags[hpa].OKTEMP++;
        //COMPRUEBA 60 VECES
        if(Flags[hpa].OKTEMP>=60)
        {
        	MaskAlarm[hpa] &= ~WARNING_T;
        	MaskAlarm[hpa] &= ~CRITICAL_T;
        }
	}
}



//////// Marcar el status de Estado Operativo
void ACCIONES_MarcarEstadoOperativo (void)
{
	// Mark HPA BF Alarms
	int hpa = BF;
	if((MaskAlarm[hpa] & SI_CRITICAL) != 0x0000)			//Critical alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_CRITICAL_BF;
	else
	    ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_CRITICAL_BF;

	if((MaskAlarm[hpa] & SI_WARNING) != 0x0000)				//Warning alarms
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_WARNING_BF;
	else
	    ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~ST_WARNING_BF;


	// Mark HPA AF Alarms
	hpa = AF;
	if((MaskAlarm[hpa] & SI_CRITICAL) != 0x0000)			//Critical alarms
		ACCIONES_SetEstadoOperativo(ST_CRITICAL_AF);
	else
		ACCIONES_ClearEstadoOperativo(ST_CRITICAL_AF);

	if((MaskAlarm[hpa] & SI_WARNING) != 0x0000)				//Warning alarms
		ACCIONES_SetEstadoOperativo(ST_WARNING_AF);
	else
		ACCIONES_ClearEstadoOperativo(ST_WARNING_AF);
}


void ACCIONES_AdjustFwdPower(unsigned int hpa)
{
	unsigned short reg_potfwd;
	unsigned short reg_potfwdobj;
	unsigned short reg_monitorAtt;
	unsigned short ATTSelect;

	if (hpa == BF)
	{
		reg_potfwd = REG_POTFWD_BF;
		reg_potfwdobj = REG_POTFWDOBJ_BF;
		reg_monitorAtt = REG_ATT1_BF;
		ATTSelect = SEL_ATT_PREVIO_BF;
	}
	else if (hpa == AF)
	{
		reg_potfwd = REG_POTFWD_AF;
		reg_potfwdobj = REG_POTFWDOBJ_AF;
		reg_monitorAtt = REG_ATT1_AF;
		ATTSelect = SEL_ATT_PREVIO_AF;
	}

	//ATTSelect = SEL_ATT_PREVIO_BF, SEL_ATT_PREVIO_AF, SEL_ATT_GANANCIA_AF

	if ( ArrayTControl[reg_potfwd].s_16 <= (0.95*ArrayTControl[reg_potfwdobj].s_16) )
	{
		while( ArrayTControl[reg_potfwd].s_16 <= (0.95*ArrayTControl[reg_potfwdobj].s_16) )
		{
			ATT_SetAttenuation( ArrayTControl[reg_monitorAtt].s_16 - 0100/*01,00 dB*/ , ATTSelect);
		}
	}

	else if( ArrayTControl[reg_potfwd].s_16 >= (1.05*ArrayTControl[reg_potfwdobj].s_16) )
	{
		while( ArrayTControl[reg_potfwd].s_16 <= (1.05*ArrayTControl[reg_potfwdobj].s_16) )
		{
			ATT_SetAttenuation( ArrayTControl[reg_monitorAtt].s_16 + 0100/*01,00 dB*/ , ATTSelect);
		}
	}
}


void ACCIONES_Actuar (int hpa)
{
  int ATTSelect;
  unsigned short MUTE_HPAX_POUT;
  unsigned short MUTE_HPAX_MASK;
  unsigned short reg_Att_1;
  unsigned short reg_Tempe;
  unsigned short reg_UESupTemp;
  unsigned short reg_UWSupTemp;

  if (hpa == BF)
  {
	  MUTE_HPAX_POUT = MUTE_HPA1_POUT;
	  MUTE_HPAX_MASK = MUTE_HPA1_MASK;
	  ATTSelect = SEL_ATT_PREVIO_BF;
	  reg_Att_1 = REG_ATT1_BF;
	  reg_Tempe = REG_T_BF;
	  reg_UESupTemp =  REG_UERRSUPTEMP_BF;
	  reg_UWSupTemp = REG_UWARSUPTEMP_BF;
  }
  else if (hpa == AF)
  {
	  MUTE_HPAX_POUT = MUTE_HPA2_POUT;
	  MUTE_HPAX_MASK = MUTE_HPA2_MASK;
	  ATTSelect = SEL_ATT_PREVIO_AF;
	  reg_Att_1 = REG_ATT1_AF;
	  reg_Tempe = REG_T_AF;
	  reg_UESupTemp =  REG_UERRSUPTEMP_AF;
	  reg_UWSupTemp = REG_UWARSUPTEMP_AF;
  }



  //MIRAMOS SI LA ETAPA DE POTENCIA ESTÁ ENCENDIDA (Switch cerrado a 0) Y LA TEMPERATURA DE LA ETAPA DE POTENCIA DA ALARMAS CRITICAS
  if(((MUTE_HPAX_POUT & MUTE_HPAX_MASK) != MUTE_HPAX_MASK) && ((MaskAlarm[hpa] & CRITICAL_T) == CRITICAL_T) && (Flags[hpa].ErrorTEMP>=60))		//OJO si SW_POT_OUT==1 etapa APAGADA !
  {
		Flags[hpa].ErrorTEMP=0;
		MUTE_HPAX_POUT |= MUTE_HPAX_MASK;	//Nico: Meto esto directamente para probar

		/*
		//ESPERAMOS UN TIEMPO ANTES DE APAGAR PARA VER SI ATENUANDO AL MAXIMO EL ERROR DESAPARECE
		if(Flags[hpa].ErrorATETEMP<1)
		{
			  //ATT_SetAttenuation(ATTMAX, i);
			  ATT_SetAttenuation ( ATT_MAX, ATTSelect );
			  Flags[hpa].ErrorATETEMP++;
			  Flags[hpa].CriticalTEMP=1;		//Nico: Creo que este flag no se usa para nada ahora mismo
		}
		else
		{
			  //TENEMOS UN ERROR. APAGAMOS LAS ETAPAS
			  MUTE_HPAX_POUT |= MUTE_HPAX_MASK;						//PREGUNTAR: por que se apaga poniendo a 1 esos registros? SW_POT_OUT activo bajo? -> Se abren los switch
			  //SW_DCDC_OUT |= SW_DCDC_MASK;
		}
		Flags[hpa].ATTTEMPCOUNTER = ArrayTControl[reg_Att_1].s_16;		//Nico Creo que este flag no sirve para nada
		*/
  }




  //MIRAMOS SI LA ETAPA DE POTENCIA ESTÁ ENCENDIDA Y LA TEMPERATURA DE LA ETAPA DE POTENCIA DA ALARMAS WARNING
  else if(((MUTE_HPAX_POUT & MUTE_HPAX_MASK) != MUTE_HPAX_MASK) && ((MaskAlarm[hpa] & WARNING_T) == WARNING_T) && (Flags[hpa].WarTEMP>=60))
  {
		//ATENUAMOS
		if(ArrayTControl[reg_Tempe].s_16 >= Flags[hpa].TEMPanterior)
		{
		  if(ArrayTControl[reg_Att_1].s_16 == ATT_MAX)
		  {
			  //ATENUACION MAXIMA, APAGAMOS
			  MUTE_HPAX_POUT |= MUTE_HPAX_MASK;
			  //SW_DCDC_OUT |= SW_DCDC_MASK;
			  Flags[hpa].MaxATETEMP=1;
		  }
		  else
		  {
			  ATT_SetAttenuation( ArrayTControl[reg_Att_1].s_16 + Flags[hpa].ATT, hpa );	//AUMENTAMOS LA ATENUACION EN 3 dB
		  }
		}

		else if( (ArrayTControl[reg_Tempe].s_16 <= (Flags[hpa].TEMPanterior - ((ArrayTControl[reg_UESupTemp].s_16 - ArrayTControl[reg_UWSupTemp].s_16)/5))) /*&& ((ATTTEMPCOUNTER-ATT)>=ATTHALLCOUNTER)*/ && (Flags[hpa].CriticalSWR==0) )
		{
			ATT_SetAttenuation(ArrayTControl[reg_Att_1].s_16 - Flags[hpa].ATT, hpa);		//DISMINUIMOS ATENUACION EN 3 dB
		}
		Flags[hpa].TEMPanterior = ArrayTControl[reg_Tempe].s_16;		//GUARDAMOS EL VALOR DEL SENSOR DE TEMPERATURA PARA COMPARARLO CON EL QUE TENGAMOS LA SIGUIENTE VEZ QUE ENTREMOS AQUI
		Flags[hpa].ATTTEMPCOUNTER = ArrayTControl[reg_Att_1].s_16;		//GUARDAMOS EL VALOR DE LA ATENUACION QUE TENEMOS PARA EL SENSOR DE TEMPERATURA PARA COMPARARLA CON LA DEL SENSOR HALL
  }




  //SI LA TEMPERATURA DA UN VALOR IGUAL O MENOR QUE SU UMBRAL DE WARNING, DESAPARECE SU ERROR CRITICO DE ATENUACION
  if((Flags[hpa].MaxATETEMP>=1) && (Flags[hpa].OKTEMP>=60))
     Flags[hpa].MaxATETEMP=0;




  //SI LA TEMPERATURA DE LA ETAPA DE POTENCIA NO TIENE ERRORES Y NO TENEMOS ERROR DE MAXIMA ATENUACION
  if(((MaskAlarm[hpa] & CRITICAL_T) != CRITICAL_T) && ((Flags[hpa].OKTEMP>=60) || (Flags[hpa].WarTEMP>=60)))
  {
     if(Flags[hpa].OKTEMP>=60)
    	 Flags[hpa].OKTEMP=0;
     else
    	 Flags[hpa].WarTEMP=0;

     Flags[hpa].CriticalTEMP=0;

     if(/*(MaxATEHALL==0) &&*/ (Flags[hpa].MaxATETEMP==0))
     {
    	 Flags[hpa].ErrorATETEMP=0;

    	 //SI LA CORRIENTE NO TIENE WARNING, RESETEAMOS LOS VALORES QUE USAMOS PARA CONTROLAR LA ATENUACION
    	 if((MaskAlarm[hpa] & WARNING_T) != WARNING_T)
    	 {
    		 Flags[hpa].TEMPanterior=0;
    		 Flags[hpa].ATTTEMPCOUNTER=0;
    	 }
    	 //SI LA CORRIENTE NO TIENE ERROR, ENCENDEMOS LAS ETAPAS
    	 if(/*(CriticalHALL==0) &&*/ (Flags[hpa].StandBy==0))
    	 {
    		 MUTE_HPAX_POUT &= ~MUTE_HPAX_MASK;
    		 //SW_DCDC_OUT &= ~SW_DCDC_MASK;

    		 //SI LA TEMPERATURA NO TIENE WARNING, LA CORRIENTE ESTA POR DEBAJO DE SU UMBRAL SUPERIOR DE WARNING Y LA ROE NO TIENE ERROR, PONEMOS AL MINIMO LA ATENUACION
    		 if(((MaskAlarm[hpa] & WARNING_T) != WARNING_T) /*&& (ArrayTControl[0x05].s_32 <= ArrayTControl[0x0C].s_32)*/ && (Flags[hpa].CriticalSWR==0)) //NICO! Comento la condición de registros 0x05 y 0x0C !!
    			 ATT_SetAttenuation(ATT_MIN , ATTSelect);
         }
     }
  }
}



int ACCIONES_LoadParamsFromGeneradora()
{
	unsigned short Register;
	pdu_I2C.DevSel = 0x01;										//I2C address of 'Generadora' (0x01)

	for(Register = 0x0100 ; Register <= 0x014A ; Register++)
	{
		Flag_I2C_Comm_Completed = 0;
		pdu_I2C.RegAdd = Register & 0x0FFF;					//Memory slot of the I2C device to be read (0x0 + 3 nibbles !!)

		if(Register == 0x0142)
			pdu_I2C.Data_Length = 8;							//Data length of the Data to be read IN BYTES
		else
			pdu_I2C.Data_Length = 2;

		I2C_Communication(I2C_READ);
		while(!Flag_I2C_Comm_Completed);

		memcpy(&ArrayGen[Register - 0x100], &pdu_I2C.Data[0], pdu_I2C.Data_Length);		//Actualizo el array local de la RAM del MCU con los valores de la Generadora
	}

	return 0;
}


void ACCIONES_SetEstadoOperativo(unsigned short Status)
{
	ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= Status;
}

void ACCIONES_ClearEstadoOperativo(unsigned short Status)
{
	ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 &= ~Status;
}


void ACCIONES_MarcarAlarmas (void)
{
	Log_info0("Setting Alarms: BF");
	ACCIONES_MarcarAlarmas_PotSWRT (BF);
    Log_info0("Setting Alarms: AF");
    ACCIONES_MarcarAlarmas_PotSWRT (AF);

    Log_info0("Setting Alarms System Status: BF & AF");
    ACCIONES_MarcarEstadoOperativo();

    //ONreciente = 0;

    //Turn ON RED LED if any critical or warning alarms
    if( ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_CRITICAL_BF) == ST_CRITICAL_BF) || ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_WARNING_BF) == ST_WARNING_BF) || ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_CRITICAL_AF) == ST_CRITICAL_AF) || ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_WARNING_AF) == ST_WARNING_AF) )
    		LED2_POUT |= LED2_MASK;		// Rojo encendido
    else
    		LED2_POUT &= ~LED2_MASK;	// Si no hay ninguna alarma apago el led rojo

    Log_info0("Acting according to alarms: BF");
    ACCIONES_Actuar(BF);
    Log_info0("Acting according to alarms: AF");
    ACCIONES_Actuar(AF);

    ACCIONES_AdjustFwdPower(BF);
    ACCIONES_AdjustFwdPower(AF);
}

/*
//Initialize the Registers from the Flash memory
void RegInit(int i)
{
  //Nico: Volcar info de la flash a param_flash
  ArrayTControl[0x0000].s_16 = ST_ARRANCANDO;
  memcpy(&param_flash, (char *)INFO_SEGMENT_B, sizeof(param_flash));


  //Perfil activo [0x0009]
  if(param_flash.perfil_activo != 0x00FF)
    memcpy(&ArrayTControl[0x0009], &param_flash.perfil_activo,2);
  else
    ArrayTControl[0x0009].s_16 = 0xFF;

  //Umbral Warning Potencia Incidente Insuficiente HPA BF [0x002A]
  if(param_flash.uwpotincinsuf_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x002A], &param_flash.uwpotincinsuf_HPA_BF,2);
  else
    ArrayTControl[0x002A].s_16 = 0xFF;

  //Umbral Error Critico Potencia Incidente Insuficiente HPA BF [0x002B]
  if(param_flash.uepotincinsuf_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x002B], &param_flash.uepotincinsuf_HPA_BF,2);
  else
    ArrayTControl[0x002B].s_16 = 0xFF;

  //Umbral Warning SWR HPA BF [0x002C]
  if(param_flash.uwswr_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x002C], &param_flash.uwswr_HPA_BF,2);
  else
    ArrayTControl[0x002C].s_16 = 0xFF;

  //Umbral Error Critico SWR HPA BF [0x002D]
  if(param_flash.ueswr_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x002D], &param_flash.ueswr_HPA_BF,2);
  else
    ArrayTControl[0x002D].s_16 = 0xFF;

  //Umbral Inferior Warning Temperatura HPA BF [0x002E]
  if(param_flash.uinfwtemperatura_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x002E], &param_flash.uinfwtemperatura_HPA_BF,2);
  else
    ArrayTControl[0x002E].s_16 = 0xFF;

  //Umbral Inferior Error Critico Temperatura HPA BF [0x002F]
  if(param_flash.uinfetemperatura_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x002F], &param_flash.uinfetemperatura_HPA_BF,2);
  else
    ArrayTControl[0x002F].s_16 = 0xFF;

  //Umbral Superior Warning Temperatura HPA BF [0x0030]
  if(param_flash.usupwtemperatura_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x0030], &param_flash.usupwtemperatura_HPA_BF,2);
  else
    ArrayTControl[0x0030].s_16 = 0xFF;

  //Umbral Superior Error Critico Temperatura HPA BF [0x0031]
  if(param_flash.usupetemperatura_HPA_BF != 0x00FF)
    memcpy(&ArrayTControl[0x0031], &param_flash.usupetemperatura_HPA_BF,2);
  else
    ArrayTControl[0x0031].s_16 = 0xFF;

  //Umbral Warning Potencia Incidente Insuficiente HPA AF [0x0032]
  if(param_flash.uwpotincinsuf_HPA_AF != 0x00FF)
      memcpy(&ArrayTControl[0x0032], &param_flash.uwpotincinsuf_HPA_AF,2);
    else
      ArrayTControl[0x0032].s_16 = 0xFF;

  //Umbral Error Critico Potencia Incidente Insuficiente HPA AF [0x0033]
  if(param_flash.uepotincinsuf_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0033], &param_flash.uepotincinsuf_HPA_AF,2);
  else
    ArrayTControl[0x0033].s_16 = 0xFF;

  //Umbral Warning SWR HPA AF [0x0034]
  if(param_flash.uwswr_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0034], &param_flash.uwswr_HPA_AF,2);
  else
    ArrayTControl[0x0034].s_16 = 0xFF;

  //Umbral Error Critico SWR HPA AF [0x0035]
  if(param_flash.ueswr_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0035], &param_flash.ueswr_HPA_AF,2);
  else
    ArrayTControl[0x0035].s_16 = 0xFF;

  //Umbral Inferior Warning Temperatura HPA AF [0x0036]
  if(param_flash.uinfwtemperatura_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0036], &param_flash.uinfwtemperatura_HPA_AF,2);
  else
    ArrayTControl[0x0036].s_16 = 0xFF;

  //Umbral Inferior Error Critico Temperatura HPA AF [0x0037]
  if(param_flash.uinfetemperatura_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0037], &param_flash.uinfetemperatura_HPA_AF,2);
  else
    ArrayTControl[0x0037].s_16 = 0xFF;

  //Umbral Superior Warning Temperatura HPA AF [0x0038]
  if(param_flash.usupwtemperatura_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0038], &param_flash.usupwtemperatura_HPA_AF,2);
  else
    ArrayTControl[0x0038].s_16 = 0xFF;

  //Umbral Superior Error Critico Temperatura HPA AF [0x0039]
  if(param_flash.usupetemperatura_HPA_AF != 0x00FF)
    memcpy(&ArrayTControl[0x0039], &param_flash.usupetemperatura_HPA_AF,2);
  else
    ArrayTControl[0x0039].s_16 = 0xFF;
}
*/
