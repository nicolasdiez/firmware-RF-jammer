/********************************************************/
/* Author: Nicolás Díez	Risueño							*/
/* Date: Feb 2016										*/
/* File: I2C.c											*/
/********************************************************/
#include <string.h>
#include "HAL.h"
#include "PDU.h"
#include "I2C.h"
#include "Power.h"
#include "Acciones2.h"

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

extern union register_entry ArrayTControl[TC_NUM_REGS];
unsigned int TxByteCtr = 0, RxByteCtr = 0;
unsigned int Tx_k = 0, Rx_k = 0;
unsigned int Rx = 0;
int countnumTries = 0;
int NumRetries = 2;
int DirCommunication = WRITE;
int GlobalDirCommunication = WRITE;
int Flag_I2C_Tx_Done = 0;
int Flag_I2C_Rx_Done = 0;
int Flag_I2C_Error = 0;
int Counter_StatusBusBusy = 0;

extern const Semaphore_Handle semI2C;
extern PDU_I2C pdu_I2C;		//Main declaration in RS485.c

//---------------------------------------- FUCNTION METHODS ----------------------------------------
void I2C_Config(void)
{
	P8SEL |= BIT5 + BIT6;					// Assign I2C pins to USCI_B1
	P8DIR |= BIT6 /*+ BIT5*/;				// SCL output
	UCB1CTL1 |= UCSWRST; 					// Enable SW reset (recommended procedure PRIOR to config)
	UCB1CTL0 = UCMODE_3 + UCSYNC + UCMST;  	// I2C mode + Sync mode + Master Mode
	UCB1CTL0 &= ~(UCA10 + UCSLA10);			// Own and Slave addresses are 7bit
	UCB1CTL1 = UCSSEL_2 + UCSWRST;          // Use SMCLK, keep SW reset
	UCB1BR0 = /*200*/240;                   // fSCL = SMCLK(24Mhz)/240 = ~100kHz		//OJO!! CAMBIAR SI SE CAMBIA MCLK/SMCLK !!!! (TAMBIÉN EN I2C_CheckBus(); )
	UCB1BR1 = 0;
	UCB1I2COA = 0x00;						//Has no influence in this case
	//UCB1I2CSA = 0x01;                   	// Slave Address is 0x10 (Generadora)
	UCB1CTL1 &= ~UCSWRST; 					// Clear SW reset, resume operation (recommended procedure AFTER config)
	//IE2 |= UCB0RXIE + UCB0TXIE;         	// Enable RX and TX interrupt
}


void I2C_Communication (int W_R)
{
	//memset (pdu_I2C.Data, 0, PDU_DATASIZE);

	UCB1I2CSA = pdu_I2C.DevSel;           		// Slave Address is 0x01 (Generadora)

	if (W_R == I2C_WRITE) //Transmit
	{
		GlobalDirCommunication = I2C_WRITE;

		Tx_k = 0;									//Iterator for pdu_I2C.Data[]
		TxByteCtr = pdu_I2C.Data_Length + /*1*/2;	//Number of Data bytes to Tx + 2 bytes (register address)
		RxByteCtr = 0;								//No reception bytes in Tx

		I2C_Transmit();

		Event_post(Event_I2C, I2C_COMM_FINISHED);
	}
	else if (W_R == I2C_READ) //Receive
	{
		GlobalDirCommunication = I2C_READ;

		Rx_k = 0;								//Iterator for pdu_I2C.Data[]
		TxByteCtr = /*1*/2;						//Before reception starts we need to Tx the Register Address (2 bytes)
		RxByteCtr = pdu_I2C.Data_Length;		//Num of bytes to Rx

		I2C_Transmit();
		__delay_cycles(20000/*bien = 10000*/);					//10000 ciclos a 24Mhz ////OJO!! CAMBIAR SI SE CAMBIA MCLK/SMCLK !!!!
		I2C_Receive();

		Event_post(Event_I2C, I2C_COMM_FINISHED);
	}
	return;
}

void I2C_Transmit(void)
{
	UInt posted;
	//DirCommunication = I2C_WRITE;
	countnumTries = 0;

	UCB1IE |= UCTXIE; 						// Enable TX interrupt (pag 965 x6xx family datasheet)
	UCB1IE |= UCNACKIE;						// Enable NACK ISR for re-tries
	UCB1IE |= UCSTPIE + UCSTTIE + UCALIE;
	//while (UCB1CTL1 & UCTXSTP);           // BEWARE while()!! Ensure stop condition got sent
	UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, generate start condition
	//while (UCB1CTL1 & UCTXSTT);           // BEWARE while()!! Start condition sent?


	posted = Event_pend(Event_I2C , I2C_TX_FINISHED , Event_Id_NONE, /*bien 1000*/1000/* x TickPeriod (x 1000us)*/);
	if (posted == 0)
		Event_post(Event_I2C, I2C_TX_ERROR);

	UCB1IE &= ~UCTXIE;
	UCB1IE &= ~UCNACKIE;
	//__delay_cycles(24000000);
	//__no_operation();

	_NOP();
}

void I2C_Receive(void)
{
	UInt posted;
	//DirCommunication = I2C_READ;
	countnumTries = 0;

	UCB1IE |= UCRXIE;				// Enable RX interrupt
	UCB1IE |= UCNACKIE;				// Enable NACK ISR for re-tries
	UCB1IE |= UCSTPIE + UCSTTIE + UCALIE;
	//while (UCB1CTL1 & UCTXSTP); 	// Ensure stop condition got sent		//Nico: espero a que el último STP mandado por la ISR del TX haya llegado
	UCB1CTL1 &= ~UCTR;              // I2C RX
	UCB1CTL1 |= UCTXSTT;            // I2C start condition
	//while (UCB1CTL1 & UCTXSTT);   // Start condition sent?

	posted = Event_pend(Event_I2C , I2C_RX_FINISHED , Event_Id_NONE, /*bien 1000*/1000/*ms*/);
	if (posted == 0)
		Event_post(Event_I2C, I2C_RX_ERROR);

	UCB1IE &= ~UCRXIE;				// Nico PREGUNTAR: Disable RX interrupt ???
	UCB1IE &= ~UCNACKIE;
}

//UCAxIE	RW
//UCAxIFG	RW
//UCAxIV	R


//************************************************ OPCION #2 ****************************************************

//#pragma vector = USCI_B1_VECTOR
//__interrupt void USCI_B1_ISR(void)
void I2C_ISR (/*UArg arg0*/void)
{
	_NOP();
	switch(__even_in_range(UCB1IV,12))
	{
		case  2:                            // Vector  2: ALIFG
			UCB1CTL1 |= UCTXSTP;
			_NOP();
			break;

	  	//NACK IFG
		case  4:
			UCB1CTL1 |= UCTXSTP;
			_NOP();
//			Flag_I2C_Tx_Done = 1;
//			Flag_I2C_Rx_Done = 1;
//			Flag_I2C_Error = 1;
//			Event_post(Event_I2C, I2C_ERROR);
//
//
//				  //METER GESTION DE SI ES RETRY DESDE LECTURA O DSD ESCRITURA
//				  if (countnumTries < NumRetries)
//			    {
//					  if (DirCommunication == WRITE)
//						  UCB1CTL1 |= UCTR + UCTXSTT;	//I2C TX, start condition
//					  else if(DirCommunication == READ)
//					  {
//							UCB1CTL1 &= ~UCTR;
//							UCB1CTL1 |= UCTXSTT;
//					  }
//			  		  countnumTries++;
//			    }
//			    else
//			    {
//			    	  UCB1CTL1 |= UCTXSTP;
//			  	  countnumTries = 0;
//			    }
			break;

		case  6:                                  	// Vector 6: STTIFG
		    UCB1IFG &= ~UCSTTIFG;
		    break;
		case  8: 									// Vector 8: STPIFG
		    UCB1IFG &= ~UCSTPIFG;
		    break;

		//MASTER LECTURA - (Master Receives Data)
		case 10:                           		 			// Vector 10: RXIFG (Data Received)

			RxByteCtr--;

			if (RxByteCtr)
			{
				pdu_I2C.Data[Rx_k++] = (unsigned char) UCB1RXBUF; 	// Get received byte
				if (RxByteCtr == 1)                     			// Only one byte left?
					UCB1CTL1 |= UCTXSTP;              				// Generate I2C stop condition
			}
			else
			{
				pdu_I2C.Data[Rx_k++] = (unsigned char) UCB1RXBUF; 	// Get final received byte,
				Event_post(Event_I2C, I2C_RX_FINISHED);
			}

			break;

		//MASTER ESCRITURA - (Master Transmits Data)
		case 12:										// Vector 12: TXIFG (Transmitting buffer empty)

			if (TxByteCtr)
			{
				if ( ((GlobalDirCommunication == I2C_READ) && (TxByteCtr == 2)) || ((GlobalDirCommunication == I2C_WRITE) && (TxByteCtr == (pdu_I2C.Data_Length + 2))) )
					UCB1TXBUF = (char)(pdu_I2C.RegAdd >> 8) & 0xFF;
				else if ( ((GlobalDirCommunication == I2C_READ) && (TxByteCtr == 1)) || ((GlobalDirCommunication == I2C_WRITE) && (TxByteCtr == (pdu_I2C.Data_Length + 1))) )
					UCB1TXBUF = (char)pdu_I2C.RegAdd & 0xFF;
				else if ( (GlobalDirCommunication == I2C_WRITE) && (TxByteCtr < pdu_I2C.Data_Length + 1) )
					UCB1TXBUF = pdu_I2C.Data[Tx_k++];

				TxByteCtr--;
			}
			//Last byte sent - generate I2C stop condition
			else
			{
				UCB1CTL1 |= UCTXSTP;
				UCB1IFG &= ~UCTXIFG; 							// Nico PREGUNTAR: Clear USCI_B0 TX int flag
				Event_post(Event_I2C, I2C_TX_FINISHED);
			}

			break;

		default:
			break;
	}
}
//****************************************************************************************************************

void I2C_CheckBus(void)	//This function is called by ISR_TimerA1 which runs every 100ms
{
	if (UCB1STAT & UCBBUSY)
		Counter_StatusBusBusy++;
	else
		Counter_StatusBusBusy = 0;

	if(Counter_StatusBusBusy >= 50/*ds*/)
	{
		//_DINT();
		//__disable_interrupt();
		//__bic_SR_register_on_exit(GIE); // Exit active CPU

		Counter_StatusBusBusy = 0;

		UCB1CTL1 |= UCSWRST;
		UCB1CTL0 &= ~UCMODE_3 + UCSYNC + UCMST;
		UCB1CTL1 &= ~UCSWRST;

		__delay_cycles(12000000);

		UCB1CTL1 |= UCSWRST;
		P8SEL |= BIT5 + BIT6;					// Assign I2C pins to USCI_B1
		P8DIR |= BIT6 /*+ BIT5*/;				// SCL output
		UCB1CTL1 |= UCSWRST; 					// Enable SW reset (recommended procedure PRIOR to config)
		UCB1CTL0 = UCMODE_3 + UCSYNC + UCMST;  	// I2C mode + Sync mode + Master Mode
		UCB1CTL0 &= ~(UCA10 + UCSLA10);			// Own and Slave addresses are 7bit
		UCB1CTL1 = UCSSEL_2 + UCSWRST;          // Use SMCLK, keep SW reset
		UCB1BR0 = /*200*/240;                    // fSCL = SMCLK(24Mhz)/240 = ~100kHz		//OJO!! CAMBIAR SI SE CAMBIA MCLK/SMCLK !!!!
		UCB1BR1 = 0;
		UCB1I2COA = 0x00;
		UCB1CTL1 &= ~UCSWRST;

		__delay_cycles(12000000);				//OJO!! CAMBIAR SI SE CAMBIA MCLK/SMCLK !!!!

		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_RESET_I2CBUS;

		//_EINT();
		//__enable_interrupt();
		//__bis_SR_register(GIE);	//Enable Interrupts
	}
	else
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 &= ~CRITICAL_RESET_I2CBUS;
}


int I2C_LoadParamsFromGeneradora(int HPA)
{
	//unsigned char Register;
	UInt posted;
	int i=0;
	char DataAux;
	int Flat_Result = OK;


	if (HPA == BF)
	{
		Semaphore_pend (semI2C, BIOS_WAIT_FOREVER);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0109 /*& 0x0FFF*/;	//Read reg 0x09 (2bytes)
		pdu_I2C.Data_Length = 2/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x012A /*& 0x0FFF*/;	//Read regs from 0x2A to 0x2F (6 x 2bytes each)
		pdu_I2C.Data_Length = 12/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0130 /*& 0x0FFF*/;	//Read regs from 0x30 to 0x36 (7 x 2bytes each)
		pdu_I2C.Data_Length = 14/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		//__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------

		Semaphore_post (semI2C);
	}

	else if (HPA == AF)
	{
		Semaphore_pend (semI2C, BIOS_WAIT_FOREVER);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0109 /*& 0x0FFF*/;	//Read reg 0x09 (2bytes)
		pdu_I2C.Data_Length = 2/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0137 /*& 0x0FFF*/;	//Read regs from 0x37 to 0x3C (6 x 2bytes each)
		pdu_I2C.Data_Length = 12/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x013D /*& 0x0FFF*/;	//Read regs from 0x3C to 0x43 (7 x 2bytes each)
		pdu_I2C.Data_Length = 14/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		//__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------
		Semaphore_post (semI2C);
	}

	else /*NO CARD TYPE*/
	{
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_INIT_PARAMS_GEN;
		ACCIONES_Regs_Init();		//Load preset parameters, just in case
		return 1;					//Error loading parameters!! ERROR!
	}



	if(Flat_Result == OK)
	{
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 &= ~CRITICAL_INIT_PARAMS_GEN;
		return 0;					//Parameters loaded successfully!! OK!
	}
	else
	{
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_INIT_PARAMS_GEN;
		ACCIONES_Regs_Init();		//Load preset parameters, just in case
		return 1;					//Error loading parameters!! ERROR!
	}

	//return 0;
}


int I2C_IsGeneradoraReady()
{
	UInt posted;
	char result = 0;
	unsigned short RxData;

	Semaphore_pend(semI2C, BIOS_WAIT_FOREVER);		//Sem Pend before config i2c PDU !!

	pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
	pdu_I2C.RegAdd = 0x0000;				//Read Status Register 0x0000
	pdu_I2C.Data_Length = 2;				//#bytes
	I2C_Communication (I2C_READ);

	posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/, 1000/*ms*/);

	if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR))
	{
		result = NOTREADY/*0*/;
	}
	else
	{
		RxData = (unsigned short)(pdu_I2C.Data[0]<<8) | (unsigned short)pdu_I2C.Data[1];		//CHECK BYTE ORDER!!

		//if( ( ((RxData & ST_GEN_APAGADO) == ST_GEN_APAGADO) || ((RxData & ST_GEN_TODOOK) == ST_GEN_TODOOK) ) && (RxData != 0xFFFF) )
		if( (RxData == ST_GEN_TODOOK) || (RxData == ST_GEN_APAGADO) )
		{
			result = READY/*0*/;
			_NOP();
		}
		else if ( ((RxData & ST_GEN_ALARMASCRITICAS) == ST_GEN_ALARMASCRITICAS) )
		{
			_NOP();
			result = ALARMS /*2*/;
		}
		else
			result = NOTREADY/*1*/;
	}

	Semaphore_post(semI2C);		//Free i2c module

	return result;
}



int I2C_StartGeneradora()
{
	UInt posted;
	int result = 0;
	//unsigned short RxData;

	Semaphore_pend(semI2C, BIOS_WAIT_FOREVER);		//Sem Pend before config i2c PDU !!

	pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
	pdu_I2C.RegAdd = 0x00D0;				//Command-Action Register
	pdu_I2C.Data_Length = 2;				//#bytes
	pdu_I2C.Data[0] = 0x00;
	pdu_I2C.Data[1] = 0x10;
	I2C_Communication (I2C_WRITE);

	posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_TX_ERROR /*+ I2C_TX_ERROR*/, 1000/*ms*/);

	if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR))
	{
		result = ERROR;
	}
	else
		result = OK;

	Semaphore_post(semI2C);		//Free i2c module

	return result;
}




//*********************************************************************************************************************
int I2C_LoadParamsFromGeneradoraDoubleCheck(int HPA)
{
	//unsigned char Register;
	UInt posted;
	int i=0;
	char DataAux;
	int Flat_Result = OK;


	if (HPA == BF)
	{
		Semaphore_pend (semI2C, BIOS_WAIT_FOREVER);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0109 /*& 0x0FFF*/;	//Read reg 0x09 (2bytes) - #PERFIL
		pdu_I2C.Data_Length = 2/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}
		__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------
		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x012A /*& 0x0FFF*/;	//Read regs from 0x2A to 0x2F (6 x 2bytes each)
		pdu_I2C.Data_Length = 12/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0130 /*& 0x0FFF*/;	//Read regs from 0x30 to 0x36 (7 x 2bytes each)
		pdu_I2C.Data_Length = 14/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x012A /*& 0x0FFF*/;	//Read regs from 0x2A to 0x2F (6 x 2bytes each)
		pdu_I2C.Data_Length = 12/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			//READ BF REGS BUT SAVE EM IN AF MEMORY SLOTS!!
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF) + 13], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0130 /*& 0x0FFF*/;	//Read regs from 0x30 to 0x36 (7 x 2bytes each)
		pdu_I2C.Data_Length = 14/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			//READ BF REGS BUT SAVE EM IN AF MEMORY SLOTS!!
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF) + 13], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------

		Semaphore_post (semI2C);


		if ( memcmp (&ArrayTControl[0x2A]/*BF REGS*/, &ArrayTControl[0x2A + 13]/*AF REGS*/, 26/*bytes*/) == 0)
			_NOP();
		else
			Flat_Result = ERROR;

	}


	else if (HPA == AF)
	{
		Semaphore_pend (semI2C, BIOS_WAIT_FOREVER);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0109 /*& 0x0FFF*/;	//Read reg 0x09 (2bytes) - #PERFIL
		pdu_I2C.Data_Length = 2/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}
		__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------
		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0137 /*& 0x0FFF*/;	//Read regs from 0x37 to 0x3C (6 x 2bytes each)
		pdu_I2C.Data_Length = 12/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x013D /*& 0x0FFF*/;	//Read regs from 0x3D to 0x43 (7 x 2bytes each)
		pdu_I2C.Data_Length = 14/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF)], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;					//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x0137 /*& 0x0FFF*/;	//Read regs from 0x37 to 0x3C (6 x 2bytes each)
		pdu_I2C.Data_Length = 12/*18*//*36*/;	//#bytes
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				//Data[0]Data[1] --> ArrayTControl[0] = Data[1]Data[0]
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			//READ AF REGS BUT SAVE EM IN BF MEMORY SLOTS!!
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF) - 13], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);

		//-------------------------------------------------------------------------------
		pdu_I2C.DevSel = 0x01;	//I2C address of 'Generadora' (0x01)
		pdu_I2C.RegAdd = 0x013D /*& 0x0FFF*/;	//Read regs from 0x3D to 0x43 (7 x 2bytes each)
		pdu_I2C.Data_Length = 14/*bytes*/;
		I2C_Communication(I2C_READ);

		posted = Event_pend(Event_I2C , Event_Id_NONE , I2C_COMM_FINISHED + I2C_RX_ERROR /*+ I2C_TX_ERROR*/ /*Event_Id_NONE*/, 12000/*ms*/);
		if ( (posted == 0) || (posted & I2C_RX_ERROR) || (posted & I2C_TX_ERROR) )
		{
			Flat_Result = ERROR;
		}
		else
		{
			//Turn over low and most significant bytes in every 16-bit slot
			for(i=0 ; i < pdu_I2C.Data_Length-1 ; i=i+2)
			{
				DataAux = pdu_I2C.Data[i];
				pdu_I2C.Data[i] = pdu_I2C.Data[i+1];
				pdu_I2C.Data[i+1] = DataAux;

				//If any 16-bit registrer equals to 0xFFFF report error
				if( (pdu_I2C.Data[i] == 0xFF) && (pdu_I2C.Data[i+1] == 0xFF) )
					Flat_Result = ERROR;
			}
			//READ AF REGS BUT SAVE EM IN BF MEMORY SLOTS!!
			memcpy(&ArrayTControl[(pdu_I2C.RegAdd & 0x00FF) - 13], &pdu_I2C.Data[0], pdu_I2C.Data_Length);
		}

		__delay_cycles(6000000/*12000000*//*24000000*/);
		//-------------------------------------------------------------------------------

		Semaphore_post (semI2C);

		if ( memcmp (&ArrayTControl[0x37]/*AF REGS*/, &ArrayTControl[0x37 - 13]/*BF REGS*/, 26/*bytes*/) == 0)
			_NOP();
		else
			Flat_Result = ERROR;
	}
	else/*NO CARD TYPE*/
	{
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_INIT_PARAMS_GEN;
		ACCIONES_Regs_Init();				//Load preset parameters, just in case
		return 1/*ERROR*/;					//Error loading parameters!! ERROR!
	}



	if(Flat_Result == OK)
	{
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 &= ~CRITICAL_INIT_PARAMS_GEN;
		return 0/*OK*/;					//Parameters loaded successfully!! OK!
	}
	else
	{
		ArrayTControl[REG_TC_ALARMASACTIVASTC].s_16 |= CRITICAL_INIT_PARAMS_GEN;
		ACCIONES_Regs_Init();				//Load preset parameters, just in case
		return 1/*ERROR*/;					//Error loading parameters!! ERROR!
	}

}
//*********************************************************************************************************************
