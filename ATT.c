/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: November 2015									*/
/* File: ATT.c											*/
/********************************************************/
#include "HAL.h"
#include "ATT.h"
#include "ports.h"
#include "DAC.h"

#ifdef	__MSP430F5638
	#include "msp430f5638.h"
#endif
#ifdef	__MSP430F5636
	#include "msp430f5636.h"
#endif

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------
extern union register_entry ArrayTControl[TC_NUM_REGS];				//Nico: Cada slot de estos [] ocupa 2bytes
//extern signed short ATT_MIN_PREVIO[2];
signed short ATT_MIN_PREVIO[2] = {0};	//La Att mínima calculada al inicio de cada HPA_Start() para que nunca se supere la Pot_obj_prev en el previo (ArrayTControl[REG_POTPREVIOOBJ_BF])
float bAtt_Remainder[2] = {0};
extern char Flag_ControllerPrevio_ON[2];
extern int CardType;

//#ifdef IS_TARJETA_BF
	signed short mV_HPA_BF = 0;
	unsigned short mV_out = 0;
	int Flag_Gain_RaiseLower = LOWER;
//#endif

//---------------------------------------- FUCNTION METHODS ----------------------------------------
//Initialize Attenuator
void ATT_Config (int ATTSelect)
{
	if(ATTSelect == SEL_ATT_PREVIO_BF)
	{
		ATT_PREVIO_BF_DIR |= ATT_PREVIO_BF_V1_MASK + ATT_PREVIO_BF_V2_MASK + ATT_PREVIO_BF_V3_MASK + ATT_PREVIO_BF_V4_MASK + ATT_PREVIO_BF_V5_MASK + ATT_PREVIO_BF_V6_MASK + ATT_PREVIO_BF_V7_MASK;
		ATT_PREVIO_BF_DIR |= ATT_PREVIO_BF_LE_MASK;
		//ATT_SetAttenuation( bAtt , SEL_ATT_PREVIO_BF );
	}

	if(ATTSelect == SEL_ATT_PREVIO_AF)
	{
		ATT_PREVIO_AF_DIR |= ATT_PREVIO_AF_V1_MASK + ATT_PREVIO_AF_V2_MASK + ATT_PREVIO_AF_V3_MASK + ATT_PREVIO_AF_V4_MASK + ATT_PREVIO_AF_V5_MASK + ATT_PREVIO_AF_V6_MASK + ATT_PREVIO_AF_V7_MASK;
		ATT_PREVIO_AF_DIR |= ATT_PREVIO_AF_LE_MASK;
		//ATT_SetAttenuation( bAtt , SEL_ATT_PREVIO_AF );
	}

	if(ATTSelect == SEL_ATT_GANANCIA_AF)
	{
		ATT_GANANCIA_AF_DIR |= ATT_GANANCIA_AF_V1_MASK + ATT_GANANCIA_AF_V2_MASK + ATT_GANANCIA_AF_V3_MASK + ATT_GANANCIA_AF_V4_MASK + ATT_GANANCIA_AF_V5_MASK + ATT_GANANCIA_AF_V6_MASK + ATT_GANANCIA_AF_V7_MASK;
		ATT_GANANCIA_AF_DIR |= ATT_GANANCIA_AF_LE_MASK;
		//ATT_SetAttenuation( bAtt , SEL_ATT_GANANCIA_AF );
	}
}


//Set Attenuation
//OJO! Se le llama con un valor de bAtt completado con 0´s a la derecha hasta tener 4 cifras, NO a la izquierda. Ejem: 0.25dB -> bAtt = 25 | 25,5dB -> bAtt = 2550 | 13dB -> bAtt = 1300
signed long ATT_SetAttenuation (signed short bAtt, unsigned int ATTSelect)		//El valor bATT tiene que introducirse en formato XXYY siendo XX decenas y unidades, e YY décimas y centésimas. Ejem: 18,75dB --> 1875
{
	if(bAtt >= ATT_MAX)
		bAtt = ATT_MAX;

	if(bAtt <= ATT_MIN)
		bAtt = ATT_MIN;

	bAtt = bAtt/ATT_FACTORREAL;		//Paso a binario para que cuadre con las lineas de los atenuadores


	if (ATTSelect == SEL_ATT_PREVIO_BF)
	{
		//ArrayTControl[REG_ATT1_BF].s_16 = bAtt * ATT_FACTORREAL;	//NICO Actualizo el monitor de atenuación en formato 3125 = 31,25dB

		// 0.25 dB
		if ( bAtt & 0x01 )        	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V7_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V7_MASK;

		// 0.5 dB
		if ( (bAtt>>1) & 0x01 )   	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V6_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V6_MASK;

		// 1 dB
		if ( (bAtt>>2) & 0x01 )   	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V5_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V5_MASK;

		// 2 dB
		if ( (bAtt>>3) & 0x01 )   	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V4_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V4_MASK;

		// 4 dB
		if ( (bAtt>>4) & 0x01 )   	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V3_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V3_MASK;

		// 8 dB
		if ( (bAtt>>5) & 0x01 )   	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V2_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V2_MASK;

		// 16 dB
		if ( (bAtt>>6) & 0x01 )   	ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_V1_MASK;
		else                  		ATT_PREVIO_BF_OUT &= ~ATT_PREVIO_BF_V1_MASK;

		//Después de meter cada valor en cada linea habilitar el Latch Enable para darle el enable al atenuador
		ATT_PREVIO_BF_OUT |= ATT_PREVIO_BF_LE_MASK;
	}


	if(ATTSelect == SEL_ATT_PREVIO_AF)
	{
		// 0.25dB
		if ( bAtt & 0x01 )        	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V7_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V7_MASK;

		// 0.5dB
		if ( (bAtt>>1) & 0x01 )   	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V6_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V6_MASK;

		// 1dB
		if ( (bAtt>>2) & 0x01 )   	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V5_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V5_MASK;

		// 2dB
		if ( (bAtt>>3) & 0x01 )   	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V4_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V4_MASK;

		// 4dB
		if ( (bAtt>>4) & 0x01 )   	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V3_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V3_MASK;

		// 8dB
		if ( (bAtt>>5) & 0x01 )   	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V2_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V2_MASK;

		// 16dB
		if ( (bAtt>>6) & 0x01 )   	ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_V1_MASK;
		else                  		ATT_PREVIO_AF_OUT &= ~ATT_PREVIO_AF_V1_MASK;

		ATT_PREVIO_AF_OUT |= ATT_PREVIO_AF_LE_MASK;
	}

	else if(ATTSelect == SEL_ATT_GANANCIA_AF)
	{
		// 0.25dB
		if ( bAtt & 0x01 )        	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V7_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V7_MASK;

		// 0.5dB
		if ( (bAtt>>1) & 0x01 )   	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V6_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V6_MASK;

		// 1dB
		if ( (bAtt>>2) & 0x01 )   	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V5_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V5_MASK;

		// 2dB
		if ( (bAtt>>3) & 0x01 )   	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V4_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V4_MASK;

		// 4dB
		if ( (bAtt>>4) & 0x01 )   	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V3_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V3_MASK;

		// 8dB
		if ( (bAtt>>5) & 0x01 )   	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V2_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V2_MASK;

		// 16dB
		if ( (bAtt>>6) & 0x01 )   	ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_V1_MASK;
		else                  		ATT_GANANCIA_AF_OUT &= ~ATT_GANANCIA_AF_V1_MASK;

		ATT_GANANCIA_AF_OUT |= ATT_GANANCIA_AF_LE_MASK;
	}

	return bAtt;
}


void ATT_SetTotalAttenuation(signed short bAtt, unsigned int hpa)
{
		if (hpa == BF)
		{
			if ( (bAtt < 0) && (Flags[BF].ControllerPrevio_ON == 1) )
			{
				ATT_MIN_PREVIO[hpa] = 0;
				bAtt = 0;
			}

			mV_HPA_BF = ArrayTControl[REG_GAN2_BF].s_16;

			if (bAtt < ATT_MIN_PREVIO[hpa])
			{
				bAtt = ATT_MIN_PREVIO[hpa];
				if (Flags[BF].PrevioPotInsuf == 0)
					//ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 |= CRITICAL_FALLO_HPA;		//Se está queriendo quitar demasiada Att -> bajar Pobj_hpa y/o subir Pobj_previo
					_NOP();
			}
			else
				//ArrayTControl[REG_TC_ALARMASACTIVASHPABF].s_16 &= ~CRITICAL_FALLO_HPA;
				_NOP();


			if ( bAtt >= ArrayTControl[REG_ATTTOT_BF].s_16 )
				Flag_Gain_RaiseLower = LOWER; 				/*Lower Gain = Raise mV*/
			else if (bAtt < ArrayTControl[REG_ATTTOT_BF].s_16 )
				Flag_Gain_RaiseLower = RAISE; 				/*Raise Gain = Lower mV*/


			if ( bAtt <= 3175 )
			{
				mV_out = 0 /*mV*/;
				ATT_SetAttenuation( bAtt , SEL_ATT_PREVIO_BF );
			}
			else if ( bAtt > 3175 )
			{
				ATT_SetAttenuation( ATT_MAX , SEL_ATT_PREVIO_BF );
				bAtt_Remainder[BF] = bAtt - ATT_MAX;

				//******* !! DAC - 1dB = 25mV/12mV !! ******* (DEL ALA!)
				if (Flag_Gain_RaiseLower == LOWER)
					mV_out = mV_HPA_BF + /*Bien 25*//*12*/1*(bAtt_Remainder[BF]/100) ;
				else if (Flag_Gain_RaiseLower == RAISE)
					mV_out = mV_HPA_BF - /*Bien 25*//*12*/1*(bAtt_Remainder[BF]/100) ;

				//mV_out = mV_HPA_BF + 25*bAtt_Remainder;
				//mV_out = mV_out / 100;

				if (mV_out > 2300/*2250*/)	//Avoid non-linear zona del control de ganancia HPA BF
					mV_out = 2300/*2250*/;
				else if (mV_out <= 0)
					mV_out = 0;
			}

			DAC_SetGainControl ( mV_out /*mV*/ );

			ArrayTControl[REG_GAN2_BF].s_16 = mV_out/*mV*/;
			ArrayTControl[REG_ATTTOT_BF].s_16 = bAtt;
		}

		if (hpa == AF)
		{
			if ( (bAtt < 0) && (Flags[AF].ControllerPrevio_ON == 1))
			{
				ATT_MIN_PREVIO[hpa] = 0;
				bAtt = 0;
			}

			if(bAtt < ATT_MIN_PREVIO[hpa])
			{
				bAtt = ATT_MIN_PREVIO[hpa];
				if(Flags[AF].PrevioPotInsuf == 0)
					//ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 |= CRITICAL_FALLO_HPA;	//Se está queriendo quitar demasiada Att -> bajar Pobj_hpa y/o subir Pobj_previo
					_NOP();
			}
			else
				//ArrayTControl[REG_TC_ALARMASACTIVASHPAAF].s_16 &= ~CRITICAL_FALLO_HPA;
				_NOP();



			if ( bAtt <= 3175 )
			{
				//- 13/06/2016 - día de pruebas ganancia AF - Se cambia el orden de actuación de los atenuadores
				ATT_SetAttenuation( bAtt , SEL_ATT_GANANCIA_AF/*SEL_ATT_PREVIO_AF*/);
				ATT_SetAttenuation( ATT_MIN , /*SEL_ATT_GANANCIA_AF*/SEL_ATT_PREVIO_AF);
				bAtt_Remainder[AF] = ATT_MIN;
			}
			else if ( bAtt > 3175 )
			{
				ATT_SetAttenuation( ATT_MAX , SEL_ATT_GANANCIA_AF/*SEL_ATT_PREVIO_AF*/ );
				bAtt_Remainder[AF] = bAtt - ATT_MAX;

				if ( bAtt_Remainder[AF] >= ATT_MAX )
					ATT_SetAttenuation( ATT_MAX , /*SEL_ATT_GANANCIA_AF*/SEL_ATT_PREVIO_AF );
				else
					ATT_SetAttenuation( bAtt_Remainder[AF] , /*SEL_ATT_GANANCIA_AF*/ SEL_ATT_PREVIO_AF);
			}

			ArrayTControl[REG_ATTTOT_AF].s_16 = bAtt;
			ArrayTControl[REG_ATT2_AF].s_16 = bAtt_Remainder[AF];
		}
}
