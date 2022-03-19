/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: Novemeber 2015									*/
/* File: FSM.c										*/
/********************************************************/

#include "HAL.h"
#include "Alarms.h"
#include "Power.h"
#include "Timers.h"
#include "FSM.h"


//FSMs Parameters
//signed short ATT_MIN_PREVIO[2] = {0};	//La Att mínima que calculada al inicio para que nunca se supere la Pot_obj_prev en el previo (ArrayTControl[REG_POTPREVIOOBJ_BF])
signed int Attextra[2] = {0};
int EstadoTemp[2] = {ESTADO_0};
int EstadoSWR[2] = {ESTADO_0};
int Apagar_HPA_T[2] = {0};				//Flag de coordinacion de apagado entre FSM T y FSM SWR
int Apagar_HPA_SWR[2] = {0};			//Flag de coordinacion de apagado entre FSM T y FSM SWR
int PotObjAux_T[2] = {0};				//Potencia objetivo real de coordinación entre FSM T y FSM SWR
int PotObjAux_SWR[2] = {0};				//Potencia objetivo real de coordinación entre FSM T y FSM SWR
unsigned int Trecovery[2] = {0,0}/*ºC*/;
extern FlagAlarms Flags[2];
extern union register_entry ArrayTControl[TC_NUM_REGS];
extern int CardType;
extern unsigned int TimerFlags [8];
extern unsigned char signatureWDT;


void FSM_ActuarBF (UArg arg0, UArg arg1)
{
	#ifdef __AUTOSTARTSYSTEM_BF_ENABLED
		ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_AUTOTEST;
		POWER_System_Start(BF);	//OJO! Si dejo esto aqui y no alcanza P_previo_obj no pasará de aquí el PC por el while() de HPA_Start
		__delay_cycles(12000000);
		//POWER_System_Start(BF);
	#endif

	Task_sleep(1000/*5000*//*ms*/);	//Let some time pass for ADC12 to update prior to Acting
	unsigned short SleepTime = 1500/*ms*/;

	#ifdef __FSM_BF_ENABLED
	_NOP();

	///*DEBUGGING*/ StartTimer (70 /*vueltas de TA1R*/ * INCREMENTO_Tactual_CADA_ISR/*interval*/ , 0 /*#Timer*/);	//50 vueltas de TA1 por lo que crece el contador en cada vuelta (65535)

	//Set the FSM starting power to the first user introduced power
	PotObjAux_T[BF] = ArrayTControl[REG_POTFWDOBJUSER_BF].s_16 /*ESTE REG NO SE PUEDE ESCRIBIR!*/;
	PotObjAux_SWR[BF] =ArrayTControl[REG_POTFWDOBJUSER_BF].s_16 /*ESTE REG NO SE PUEDE ESCRIBIR!*/;

	while(1)
	{
		#ifdef __WDT_ENABLED
		signatureWDT |= SIGNATURE_TASK_ACTUARBF;
		#endif

		if ( (CardType == BF) /*&& ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_ARRANCANDO) != ST_ARRANCANDO)*/ )
		{
			//AUX_2_POUT |= AUX_2_MASK; //SET pin 9.5

			SleepTime = /*1000*/600/*ms*/;

			if (Flags[BF].System_Stop == 0)
			{
				_NOP();
				_NOP();

				//******************************* TEMPERATURE FSM *******************************
				switch (EstadoTemp[BF])
				{
					case ESTADO_0:	//T < T_critica

						Apagar_HPA_T[BF] = 0;
						PotObjAux_T[BF] = ArrayTControl[REG_POTFWDOBJUSER_BF].s_16;  //Set back la Pot Obj introducida por el usuario

						if( (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & CRITICAL_T) == CRITICAL_T )
							EstadoTemp[BF] = ESTADO_1;
						break;

					case ESTADO_1:	//T > T_critica

						Apagar_HPA_T[BF] = 0;

						if( TimerFlags [1 /*num Timer*/] == 0 )
						{
							if( (Attextra[BF] > LIMIT_ATT_TEMP_BF) /*&& (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & CRITICAL_T == CRITICAL_T)*/ )
								EstadoTemp[BF] = ESTADO_2;
							else if( (ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & CRITICAL_T) != CRITICAL_T )
								EstadoTemp[BF] = ESTADO_2;
							else
							{
								Attextra[BF] += 30/*3dB*/;	//Accumulated extra attenuation
								PotObjAux_T[BF] = PotObjAux_T[BF] - 30/*dB*/;

								if( PotObjAux_T[BF]  < 300/*mín 30dBm*/ )
									PotObjAux_T[BF] = 300;

								StartTimer (250/*25seg*/ * INCREMENTO_Tactual_CADA_ISR, 1 /*num Timer*/);
							}
						}
						break;

					case ESTADO_2:	//T > T_Recovery

						Apagar_HPA_T[BF] = 1;
						Attextra[BF] = 0;

						if (ArrayTControl[REG_T_BF].s_16 < Trecovery[BF])
							EstadoTemp[BF] = ESTADO_0;
						break;
				}
				//************************************************************************************


				//******************************* SWR(Return Loss) FSM *******************************
				switch (EstadoSWR[BF])
				{
					case ESTADO_0:	//RL > RL_Critica

						Apagar_HPA_SWR[BF] = 0;
						PotObjAux_SWR[BF] = ArrayTControl[REG_POTFWDOBJUSER_BF].s_16;  //Set back la Pot Obj introducida por el usuario

						if( ((ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & CRITICAL_SWR) == CRITICAL_SWR) )
						{
							EstadoSWR[BF] = ESTADO_1;
							Apagar_HPA_SWR[BF] = 1;	//Por seguridad de la HPA
						}
						break;

					case ESTADO_1:	//RL < RL_Critica

						Apagar_HPA_SWR[BF] = 1;

						if (TimerFlags [/*numTimer:*/2] == 0)
						{
							EstadoSWR[BF] = ESTADO_2;
							StartTimer (150 * INCREMENTO_Tactual_CADA_ISR, 2 /*num Timer*/);	//15seg
						}
						else
							_NOP();

						break;

					case ESTADO_2:	//RL < RL_Critica

						if(Apagar_HPA_SWR[BF] == 0)
						{
							if( ((ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & CRITICAL_SWR) != CRITICAL_SWR) )
								EstadoSWR[BF] = ESTADO_0;
							else if ( ((ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 & CRITICAL_SWR) == CRITICAL_SWR) )
							{
								EstadoSWR[BF] = ESTADO_1;
								Apagar_HPA_SWR[BF] = 1;	//Apago aquí directamente por seguridad de la HPA!!
							}
						}
						else
						{
							Apagar_HPA_SWR[BF] = 0;
							PotObjAux_SWR[BF] = 310/*10xdBm*/;
							SleepTime = /*500*/ 300 /*ms*/;
						}
						break;
				}
				//************************************************************************************

				//******************************** SET MUTE & POT_OBJ ********************************
				//Set HPA BF on/off according to decisions of Temp and SWR state machines
				if ( ((Apagar_HPA_T[BF] | Apagar_HPA_SWR[BF]) == 1) && (Flags[BF].HPA_Stop == 0) )
					POWER_HPA_Stop (BF);			//HPA Stop
				else if ( ((Apagar_HPA_T[BF] | Apagar_HPA_SWR[BF]) == 0) && (Flags[BF].HPA_Stop == 1) )
					POWER_HPA_Start (BF);		//HPA Start

				//Set minimum Potencia Objetivo according to decisions of Temp/SWR FSMs
				if (PotObjAux_T[BF] > PotObjAux_SWR[BF])
					ArrayTControl[REG_POTFWDOBJREAL_BF].s_16 = PotObjAux_SWR[BF]; /*POTFWDOBJ REAL -> ESTE ES EL QUE USA EL POWERCONTROLLER*/
				else
					ArrayTControl[REG_POTFWDOBJREAL_BF].s_16 = PotObjAux_T[BF]; /*POTFWDOBJ REAL -> ESTE ES EL QUE USA EL POWERCONTROLLER*/
				//************************************************************************************
			}
		}
		//Task_yield();
		//AUX_2_POUT &= ~AUX_2_MASK; 		//CLEAR pin 9.5
		Task_sleep(SleepTime/*ms*/);		//Tick period is set in CFG Clock = 1000us
	}
	#endif
}




void FSM_ActuarAF (UArg arg0, UArg arg1)
{
	#ifdef __AUTOSTARTSYSTEM_AF_ENABLED
		//ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_AUTOTEST;
		//POWER_System_Start(AF);	//OJO! Si dejo esto aqui y no alcanza P_previo_obj no pasará de aquí el PC por el while() de HPA_Start
	#endif

	Task_sleep (1000/*5000*//*ms*/);	//Let some time pass for ADC12 to update prior to Acting
	unsigned short SleepTime = 1500/*ms*/;

	#ifdef __FSM_AF_ENABLED
	_NOP();

	//Set the FSM starting power to the first user introduced power
	PotObjAux_T[AF] = ArrayTControl[REG_POTFWDOBJUSER_AF].s_16 /*ESTE REG NO SE PUEDE ESCRIBIR!*/;
	PotObjAux_SWR[AF] = ArrayTControl[REG_POTFWDOBJUSER_AF].s_16 /*ESTE REG NO SE PUEDE ESCRIBIR!*/;

	while(1)
	{
		#ifdef __WDT_ENABLED
		signatureWDT |= SIGNATURE_TASK_ACTUARAF;
		#endif

		if ( (CardType == AF) /*&& ((ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 & ST_ARRANCANDO) != ST_ARRANCANDO)*/ )
		{
			//AUX_2_POUT |= AUX_2_MASK; //SET pin 9.5

			SleepTime = /*1000*/600/*ms*/;

			if (Flags[AF].System_Stop == 0)
			{
				//******************************* TEMPERATURE FSM *******************************
				switch (EstadoTemp[AF])
				{
					case ESTADO_0:	//T < T_critica

						Apagar_HPA_T[AF] = 0;
						PotObjAux_T[AF] = ArrayTControl[REG_POTFWDOBJUSER_AF].s_16;  //Set back la Pot Obj introducida por el usuario

						if( (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & CRITICAL_T) == CRITICAL_T )
							EstadoTemp[AF] = ESTADO_1;
						break;

					case ESTADO_1:	//T > T_critica

						Apagar_HPA_T[AF] = 0;

						if( TimerFlags [3 /*num Timer*/] == 0 )
						{
							if( (Attextra[AF] > LIMIT_ATT_TEMP_AF) /*&& (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & CRITICAL_T == CRITICAL_T)*/ )
								EstadoTemp[AF] = ESTADO_2;
							else if( (ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & CRITICAL_T) != CRITICAL_T )
								EstadoTemp[AF] = ESTADO_2;
							else
							{
								Attextra[AF] += 30/*3dB*/;	//Accumulated extra attenuation
								PotObjAux_T[AF] = PotObjAux_T[AF] - 30/*dB*/;

								if( PotObjAux_T[AF]  < 300/*mín 30dBm*/ )
									PotObjAux_T[AF] = 300;

								StartTimer (250 * INCREMENTO_Tactual_CADA_ISR, 3 /*num Timer*/);	//25seg
							}
						}
						break;

					case ESTADO_2:	//T > T_Recovery

						Apagar_HPA_T[AF] = 1;
						Attextra[AF] = 0;

						if (ArrayTControl[REG_T_AF].s_16 < Trecovery[AF])
							EstadoTemp[AF] = ESTADO_0;
						break;
				}
				//************************************************************************************

				//******************************* SWR(Return Loss) FSM *******************************
				switch (EstadoSWR[AF])
				{
					case ESTADO_0:	//RL > RL_Critica

						Apagar_HPA_SWR[AF] = 0;
						PotObjAux_SWR[AF] = ArrayTControl[REG_POTFWDOBJUSER_AF].s_16;  //Set back la Pot Obj introducida por el usuario

						if( ((ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & CRITICAL_SWR) == CRITICAL_SWR) )
						{
							EstadoSWR[AF] = ESTADO_1;
							Apagar_HPA_SWR[AF] = 1;	//Por seguridad de la HPA
						}
						break;

					case ESTADO_1:	//RL < RL_Critica

						Apagar_HPA_SWR[AF] = 1;

						if (TimerFlags [/*numTimer:*/4] == 0)
						{
							EstadoSWR[AF] = ESTADO_2;
							StartTimer (150 * INCREMENTO_Tactual_CADA_ISR, 4 /*num Timer*/);	//15seg
						}
						else
							_NOP();

						break;

					case ESTADO_2:	//RL < RL_Critica

						if (Apagar_HPA_SWR[AF] == 0)
						{
							//__delay_cycles(48000000);
							//Task_sleep (125/*ms*/);	//consider time raising if doesnt recover
							if( ((ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & CRITICAL_SWR) != CRITICAL_SWR) )
								EstadoSWR[AF] = ESTADO_0;
							else if ( ((ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 & CRITICAL_SWR) == CRITICAL_SWR) )
							{
								EstadoSWR[AF] = ESTADO_1;
								Apagar_HPA_SWR[AF] = 1;	//Apago aquí directamente por seguridad de la HPA!!
							}
						}
						else
						{
							Apagar_HPA_SWR[AF] = 0;

							//PotObjAux_SWR[AF] = 310/*10xdBm*/;						//esto bien antes de uco!
							//SleepTime = /*500*/ /*Ok 400*/ 450 /*Mal 300*/ /*ms*/;	//esto bien antes de uco!

							PotObjAux_SWR[AF] = 430/*10xdBm*/;									// OJO!!! !!!!!!CAMBIO AQUÍ día uco(no sol)!!!!!!!
							SleepTime = /*500*/ /*Ok 400*/ 700 /*bien 450*/ /*Mal 300*/ /*ms*/;	// OJO!!! !!!!!!CAMBIO AQUÍ día uco(no sol)!!!!!!!
						}

						break;
				}
				//************************************************************************************


				//******************************** SET MUTE & POT_OBJ ********************************
				//Set HPA AF on/off according to decisions of Temp and SWR state machines
				if ( ((Apagar_HPA_T[AF] | Apagar_HPA_SWR[AF]) == 1) && (Flags[AF].HPA_Stop == 0) )
					POWER_HPA_Stop (AF);			//HPA Stop
				else if ( ((Apagar_HPA_T[AF] | Apagar_HPA_SWR[AF]) == 0) && (Flags[AF].HPA_Stop == 1) )
					POWER_HPA_Start (AF);			//HPA Start

				//Set minimum Potencia Objetivo according to decisions of Temp/SWR FSMs
				if(PotObjAux_T[AF] > PotObjAux_SWR[AF])
					ArrayTControl[REG_POTFWDOBJREAL_AF].s_16 = PotObjAux_SWR[AF]; /*POTFWDOBJ REAL -> ESTE ES EL QUE USA EL POWERCONTROLLER*/
				else
					ArrayTControl[REG_POTFWDOBJREAL_AF].s_16 = PotObjAux_T[AF]; /*POTFWDOBJ REAL -> ESTE ES EL QUE USA EL POWERCONTROLLER*/
				//************************************************************************************
			}
		}

		//Task_yield();
		//AUX_2_POUT &= ~AUX_2_MASK; 		//CLEAR pin 9.5
		Task_sleep(SleepTime/*ms*/);		//Tick period is set in CFG Clock = 1000us
	}

	#endif
}
