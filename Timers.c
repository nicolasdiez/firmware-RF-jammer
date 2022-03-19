/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: December 2015									*/
/* File: Timers.c										*/
/********************************************************/

#include <string.h>
#include "Timers.h"
#include "HAL.h"
#include "power.h"
#include "Acciones2.h"
#include "PowerController.h"
#include "I2C.h"

#ifdef	__MSP430F5638
	#include "msp430f5638.h"
#endif
#ifdef	__MSP430F5636
	#include "msp430f5636.h"
#endif

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
	//#include <ti/sysbios/hal/Hwi.h>
#endif

#ifdef __WDT_ENABLED
	extern unsigned char signatureWDT;
#endif

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
extern union register_entry ArrayTControl[TC_NUM_REGS];

unsigned int timerTicks = 0;
unsigned int timerTicks2 = 0;
char Testing_LEDs = 0;

extern unsigned char cyclesWDT;
extern unsigned char signatureWDT;
//extern unsigned char LedBusON;

//Variables for custom homemade timer
unsigned long Tinterval [8] = {0};
unsigned long Tref [8] = {0};
unsigned int TimerFlags [8] = {0};
unsigned short extension = 0;
unsigned long Tactual = 0;
unsigned long Tactual2 = 0;
unsigned long Tactual1 = 0;
extern int CardType;


//---------------------------------------- FUCNTION METHODS ----------------------------------------
//OJO! NO usar TA0 --> Lo usa el RTOS internamente para sus cálculos temporales.

//Usamos la ISR TB_0 para el control de firmas y kickear el WDT
void TIMER_B0_Config (void)
{
	System_printf("Config Timer_B and WatchDog: ");
	//------- Timer B is used to trigger the signature checking ISR -------
	TB0CCTL0 = CCIE;				//Capture-compare-0 INTERRUPT ENABLED
	TB0CCR0 = TIMERB_1_SECOND;		//In Compare mode: TBxCCRn holds the data for the comparison to the timer value in the Timer_B Register, TBR
	//TB0CTL |= MC0;				//(Starts the Timer) Mode Control 01b -> Up mode: Timer counts up to TBxCCR0
	TB0CTL = TBSSEL0 + TBCLR /*+ TBIE*/ + MC_1 + ID_3;	//ACLK, Reset TB, Up Mode, IE, ACLK/8

	//WDTCTL = (WDTCTL & 0x00FF & ~WDT_PLUS_HOLD) | WDT_ADLY_14s; //Inicializamos Watchdog

	//------- WDT is used in case the TB0-Check_WDT ISR fails -------
	WDTCTL = /*WDT_ARST_1000*/ (WDTPW + WDTCNTCL + WDTSSEL0 + WDTIS1 + WDTIS0); //pw, Watchdog mode, count clear, ACLK, 16s wdt interval;
	System_printf("DONE\n");
}

//Uso la ISR de Timer_A1 para el timer casero que se usa en las FSMs y para las llamadas a los...
//...controladores de potencia del previo y de salida
void TIMER_A1_Config()
{
//	TA2CCTL0 &= ~CCIE;									//Capture-compare-0 INTERRUPT ENABLED
//	TA2CCR0 = TIMERB_2_SECOND;							//In Compare mode: TBxCCRn holds the data for the comparison to the timer value in the Timer_B Register, TBR
//	TA2CTL = TASSEL0 + TACLR /*+ TBIE*/ + MC_1 + ID_3;	//ACLK, Reset TB, Up Mode, IE, ACLK/8

	//TA1CCTL0 &= ~CCIE;								//Capture-compare-0 INTERRUPT DISABLED
	TA1CCTL0 |= CCIE;
	TA1CCR0 = TIMERB_100_MSEC/*TIMERB_8_SECOND*/;		//In Compare mode: TBxCCRn holds the data for the comparison to the timer value in the Timer_B Register, TBR
	TA1CTL = TASSEL0 + TACLR + MC0 + ID_3;				//ACLK, Reset TB, Up Mode, IE, ACLK/8
}


//Runs every 1 second
void ISR_TimerB0/*WDT_ISR*/ (/*UArg arg0*/)
{
  timerTicks++;
  //timerTicks2++;

  WDTCTL = (WDTPW + WDTCNTCL + WDTSSEL0 + WDTIS1 + WDTIS0)/*WDTPW + WDTCNTCL*/;		//Kick watchdog with 16s at 32768hz

  //Check signature every 3secs
  if(timerTicks >= 1/*bien 3*//*TIMER_LEDS_1500_MILLS*/)	// 3 Segs
  {
    //Chequeamos firmas
	#ifdef __WDT_ENABLED
	  WDT_CheckWDT();
	#endif
    timerTicks = 0;
  }

}

//En Platinum Mossos -> Check cada 3 secs y 80 ciclos para WDT

#ifdef __WDT_ENABLED
void WDT_CheckWDT(void)
{
	  if ((signatureWDT & WDT_SIGNATURE_DISABLED) == 0)			//Si el WDT no está disabled
	  {
		if ((signatureWDT & WDT_SIGNATURE_MASK) == WDT_SIGNATURE_MASK)	//Si todos han firmado bien		//PREGUNTAR: Que significa la mask 0x3F ? ver notas cuaderno Nico
		{
		  signatureWDT &= ~WDT_SIGNATURE_MASK;
		  cyclesWDT= WDT_MAX_RESET_CYCLES;
		}
		else				//Si alguno ha fallado y no ha firmado
		{
		  cyclesWDT--;
		  if (cyclesWDT == 0)
		  {
			POWER_ControlledReset(0);
		  }
		}
	  }
	  else
	  {
		cyclesWDT= WDT_MAX_RESET_CYCLES;
	  }
}
#endif


//Runs every 100MS (TA1CCR0)
void ISR_TimerA1(void)
{
#ifdef __TIMERA1_ENABLED

	int i;
	unsigned long Inc_time = 0;

	extension++;	//En cada ISR se incrementa el Tactual en 65536

	Tactual1 = (unsigned long)extension<<16 /*| (unsigned long)TA1R*/;
	Tactual2 = (unsigned long)TA1R;
	Tactual = Tactual1 | Tactual2;

	for (i=0 ; i < 8 ; i++)
	{
		if (TimerFlags[i] == 1)
		{
			//Let some time pass, otherwise is not updating Tref and Tactual properly !!!
			_NOP();
			_NOP();

			if (Tref[i] >= Tactual)
				Inc_time = 0xFFFFFFFF - Tref[i] + Tactual;
			else
				Inc_time = Tactual - Tref[i];

			if ( Inc_time > Tinterval[i] )
				TimerFlags[i] = 0;
		}
	}
#endif

#ifdef __I2C_CHECKBUS_ENABLED
	I2C_CheckBus();
#endif

//------------ LEDS
	timerTicks2++;

	if (timerTicks2 >= 5/*ds*/)
	{
		if (Testing_LEDs == 0)
			BlinkLEDs();
		timerTicks2 = 0;
	}
//--------------------------------

#ifdef __POWERCONTROLLER_BF_ENABLED
	//**************************** POWER CONTROLLER BF ****************************
	//Si en NUM_TRIES_CONTROLLERPREVIO_BF vueltas del Previo Power Controller no se ha alcanzado la P_obj_previo damos un error y lo tipificamos
	if ( (Flags[BF].PowerPrevioController_counter >= NUM_TRIES_CONTROLLERPREVIO_BF) && (Flags[BF].ControllerPrevio_ON == 1) )
	{
		if (ArrayTControl[REG_ATTTOT_BF].s_16 == 0) //pot previo insuf
		{
			//ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 |= CRITICAL_POTPREVIO_INSUF;		//Not marked to avoid LED blinking for Pot. Prev. Insuf.
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POTPREVIO_EXCES;
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POT_PREVIO;
			//Flag_ControllerPrevio_ON[BF] = 0;													//Arrancamos HPA aunque sea con pot previo insuf
			Event_post(Event_PrevPowerControllerBF, PREVCONTROLLER_BF_FINISHED);
			Flags[BF].PrevioOK_StartHPA = 1; 													//From previo side the HPA can be started
			Flags[BF].PrevioPotInsuf = 1;
		}
		else if (ArrayTControl[REG_ATTTOT_BF].s_16 >= ATT_MAX)
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 |= CRITICAL_POTPREVIO_EXCES;			//From previo side the HPA can NOT be started
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POTPREVIO_INSUF;
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POT_PREVIO;
			Event_post(Event_PrevPowerControllerBF, PREVCONTROLLER_BF_POTPREVIO_EXCES);
			Flags[BF].PrevioOK_StartHPA = 0;
			Flags[BF].ControllerPrevio_ON = 0;
			Flags[BF].PrevioPotInsuf = 0;
		}
		else
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 |= CRITICAL_POT_PREVIO;				//Error genérico de Pot Previo. Do NOT start HPA
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POTPREVIO_INSUF;
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POTPREVIO_EXCES;
			Event_post(Event_PrevPowerControllerBF, PREVCONTROLLER_BF_PREVIOERROR);
			Flags[BF].PrevioOK_StartHPA = 0;
			Flags[BF].ControllerPrevio_ON = 0;
			Flags[BF].PrevioPotInsuf = 0;
		}
	}

	//HPA-BF OUTPUT POWER CONTROLLER
	if ( (Flags[BF].ControllerPrevio_ON == 0) && (Flags[BF].PrevioOK_StartHPA == 1) )
		ACCIONES_PowerControllerPD (BF);

	//PREVIO HPA-BF INPUT POWER CONTROLLER (opcion 2)
	if( (Flags[BF].ControllerPrevio_ON == 1) && (Flags[BF].PowerPrevioController_counter < NUM_TRIES_CONTROLLERPREVIO_BF ) )
	{
		if ( (ACCIONES_PowerPrevioControllerPD(BF) == 1) )
		{
			//potencia previo alcanzada: marcamos para apagar el previo power controller y seguir con el output power controller
			Event_post(Event_PrevPowerControllerBF, PREVCONTROLLER_BF_FINISHED);
			Flags[BF].PrevioOK_StartHPA = 1;
			Flags[BF].PrevioPotInsuf = 0;
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POT_PREVIO;
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POTPREVIO_EXCES;
			ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_POTPREVIO_INSUF;
		}
		else
		{
			Flags[BF].PowerPrevioController_counter++;
			Flags[BF].PrevioOK_StartHPA = 0;
		}
	}
#endif



#ifdef __POWERCONTROLLER_AF_ENABLED
	//**************************** POWER CONTROLLER AF ****************************
	//Si en NUM_TRIES_CONTROLLERPREVIO_AF vueltas del Previo Power Controller no se ha alcanzado la P_obj_previo damos un error y lo tipificamos
	if ( (Flags[AF].PowerPrevioController_counter >= NUM_TRIES_CONTROLLERPREVIO_AF) && (Flags[AF].ControllerPrevio_ON == 1))
	{
		if (ArrayTControl[REG_ATTTOT_AF].s_16 == 0) //pot previo insuf
		{
			//ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 |= CRITICAL_POTPREVIO_INSUF;	//Not marked to avoid LED blinking for Pot. Prev. Insuf.
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POT_PREVIO;
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POTPREVIO_EXCES;
			//Flag_ControllerPrevio_ON[AF] = 0;												//Arrancamos HPA aunque sea con pot previo insuf
			Event_post(Event_PrevPowerControllerAF, PREVCONTROLLER_AF_FINISHED);
			Flags[AF].PrevioOK_StartHPA = 1; 												//From previo side the HPA can be started
			Flags[AF].PrevioPotInsuf = 1;
		}
		else if (ArrayTControl[REG_ATTTOT_AF].s_16 >= 2*ATT_MAX)	//pot previo exces
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 |= CRITICAL_POTPREVIO_EXCES;		//From previo side the HPA can NOT be started
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POT_PREVIO;
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POTPREVIO_INSUF;
			Event_post(Event_PrevPowerControllerAF, PREVCONTROLLER_AF_POTPREVIO_EXCES);
			Flags[AF].PrevioOK_StartHPA = 0;
			Flags[AF].ControllerPrevio_ON = 0;
			Flags[AF].PrevioPotInsuf = 0;
		}
		else
		{
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 |= CRITICAL_POT_PREVIO;			//Error genérico de Pot Previo. Do NOT start HPA
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POTPREVIO_EXCES;
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POTPREVIO_INSUF;
			Event_post(Event_PrevPowerControllerAF, PREVCONTROLLER_AF_PREVIOERROR);
			Flags[AF].PrevioOK_StartHPA = 0;
			Flags[AF].ControllerPrevio_ON = 0;
			Flags[AF].PrevioPotInsuf = 0;
		}
	}

	//HPA-AF OUTPUT POWER CONTROLLER
	if ( (Flags[AF].ControllerPrevio_ON == 0) && (Flags[AF].PrevioOK_StartHPA == 1) )
		ACCIONES_PowerControllerPD (AF);

	//PREVIO HPA-AF INPUT POWER CONTROLLER (opcion 2)
	if( (Flags[AF].ControllerPrevio_ON == 1) && (Flags[AF].PowerPrevioController_counter < NUM_TRIES_CONTROLLERPREVIO_AF ) )
	{
		if ( (ACCIONES_PowerPrevioControllerPD(AF) == 1) )
		{
			//Flag_ControllerPrevio_ON[AF] = 0;	//potencia previo alcanzada: marcamos para apagar el previo power controller y seguir con el output power controller
			Event_post(Event_PrevPowerControllerAF, PREVCONTROLLER_AF_FINISHED);
			Flags[AF].PrevioOK_StartHPA = 1;
			Flags[AF].PrevioPotInsuf = 0;
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POT_PREVIO;
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POTPREVIO_EXCES;
			ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_POTPREVIO_INSUF;
		}
		else
		{
			Flags[AF].PowerPrevioController_counter++;
			Flags[AF].PrevioOK_StartHPA = 0;
		}
	}
#endif

}


//NOTA! POR EJEMPLO, UN t_interval DE (50*65535) SERÍA UN TOTAL DE 50 VUELTAS DE LA ISR TA1, SI SALTA CADA 100MS SERÍAN 50*100ms = 5s
void StartTimer (unsigned long t_interval , int numTimer)
{
	//MAXIMO INTERVALO TEMPORAL CON ISR TA1 CADA 100MS = (2^32/65535)vueltas * 100ms = 6553segundos!!
	Tinterval [numTimer] = t_interval;
	Tref [numTimer] = Tactual /*((unsigned long)extension<<16) | TA1R*/;
	TimerFlags [numTimer] = 1;
}


void initClocksNico(void)
{
	//Config XT2 port
	P7SEL |= 0x06;						//Select peripheral functionality por XTAL2 pin port
	P7DIR &= ~0x04;						//P7.2 (XT2IN) as input
	P7DIR |= 0x08;						//P7.3 (XT2OUT) as output

	//Config XT2 regs - XT2 On
	UCSCTL6 |= XT2DRIVE_3;				//XT2 max drive capability
	UCSCTL6	&= ~XT2BYPASS;				//XT2 sourced from external xtal
	UCSCTL6 &= ~XT2OFF;					//XT2 On (24Mhz)

	//Config XT1 regs - XT1 Off
//	UCSCTL6 |= XT1DRIVE_3;				//XT1 max drive capability
//	UCSCTL6	&= ~XTS;					//XT1 Low Freq Mode
//	UCSCTL6	&= ~XT1BYPASS;				//XT1 sourced from external xtal
	UCSCTL6 |= XT1OFF;					//XT1 Off

	//Inicializar FLL con REFOCLK
	UCSCTL3 |= SELREF_2 + FLLREFDIV_0;	//FLL <- REFOCLK , f(FLLREFCLK)/1

	//Inicializar ACLK con REFOCLK
	UCSCTL4 |= SELA_2;					//ACLK <- REFOCLK
	UCSCTL5 |= DIVA_0;					//f(ACLK)/1

	//Inicializar MCLK y SMCLK a 24Mhz
	UCSCTL4 |= SELM_5/*SELM_5*/;		//MCLK <- XT2
	UCSCTL4 |= SELS_5/*SELS_5*/;		//SMCLK <- XT2
	UCSCTL5 |= DIVM_0/*DIVM_0*/;		//f(MCLK)/1
	UCSCTL5 |= DIVS_0/*DIVS_0*/;		//f(SMCLK)/1

	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);		// Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;                    			// Clear fault flags
	}while (SFRIFG1 & OFIFG);                   		// Test oscillator fault flag
}


void BlinkLEDs(void)
{
	if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_ARRANCANDO) != ST_ARRANCANDO )
	{
		//  ALARMS LED
		if ( (Flags[CardType].HPA_Stop == 0) && (Flags[CardType].System_Stop == 0))
		{
			if( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & (ST_CRITICAL_CONTROL ^ ST_CRITICAL_BF ^ ST_CRITICAL_AF) ) != 0x0000 )
				LED_1_POUT ^= LED_1_MASK;	//alarms led toggle
			else if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & (ST_WARNING_CONTROL ^ ST_WARNING_BF ^ ST_WARNING_AF) ) != 0x0000 )
				LED_1_POUT |= LED_1_MASK;	//alarms led on
			else
				LED_1_POUT &= ~LED_1_MASK;	//alarms led off
		}
		else if ( (Flags[CardType].HPA_Stop == 1) && (Flags[CardType].System_Stop == 0) )
			LED_1_POUT ^= LED_1_MASK;	//alarms led toggle. If HPA off but System on --> critical alarm active
		else
			LED_1_POUT &= ~LED_1_MASK;

		//  ON/OFF Status LED
		if ( (Flags[CardType].HPA_Stop == 0) && (Flags[CardType].System_Stop == 0) )
			LED_2_POUT |= LED_2_MASK;
		else
			LED_2_POUT ^= LED_2_MASK;	//on-off led toggle

//		//	ON/OFF Status LED
//		if ( (Flags[CardType].HPA_Stop == 1) && (Flags[CardType].System_Stop == 0) )
//			LED_2_POUT ^= LED_2_MASK;	//on-off led toggle
//		else if ( (Flags[CardType].HPA_Stop == 0) && (Flags[CardType].System_Stop == 0) )
//			LED_2_POUT |= LED_2_MASK;
//		else if ( (Flags[CardType].HPA_Stop == 1) && (Flags[CardType].System_Stop == 1) )
//			LED_2_POUT &= ~LED_2_MASK;
	}
	else if ( (ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_ARRANCANDO) == ST_ARRANCANDO )
	{
		LED_1_POUT ^= LED_1_MASK;	//alarms led
		LED_3_POUT ^= LED_3_MASK;	//bus led

		LED_2_POUT ^= LED_2_MASK;	//on-off led toggle
	}
}
