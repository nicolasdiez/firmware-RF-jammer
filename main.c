/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: main.c											*/
/********************************************************/
#include <string.h>
#include "driverlib.h"
#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"

#include <ti/sysbios/BIOS.h>   // BIOS includes
#include <ti/sysbios/knl/Event.h>
#include <xdc/runtime/Error.h>

#include "HAL.h"
#include "USB.h"
#include "PDU.h"
#include "UART.h"
#include "I2C.h"
//#include "WDT.h"
#include "Timers.h"
#include "Ports.h"
#include "Power.h"
//#include "Acciones.h"
#include "Acciones2.h"
#include "ADC2.h"
#include "ATT.h"
#include "DAC.h"
#include "FlashMem.h"

// Global flags set by events
volatile uint8_t bCDCDataReceived_event = FALSE;  // Flag set by event handler to indicate data has been received into USB buffer


#ifdef __WDT_ENABLED
	unsigned char signatureWDT = 0;
	unsigned char cyclesWDT = 0;
#endif

union register_entry ArrayTControl[TC_NUM_REGS] = {0};
//union register_entry ArrayGen[GEN_NUM_REGS] = {0};

extern PDU_I2C pdu_I2C;							//Main declaration in RS485.c

#pragma location=INFO_SEGMENT_A
unsigned short FWversion /*= 1*/;
//int CardType = ERROR_CARD;



//FW PORTATIL CAMBIOS HECHOS EN ADC.C, ACCIONES.C, PORTS.C, ATT.C, DAC.C, .CFG, RS485.c, USB_TEST.c
void main(void)
{
  ArrayTControl[REG_TC_ESTADOOPERATIVO].s_16 |= ST_ARRANCANDO;	//Flag taken out after finishing InitSystem()

  WDT_A_hold(WDT_A_BASE); // Stop watchdog timer (Muevo esta instruccion desde USB_Config() aquí)
  //ACCIONES_InitRegisters();

  //-------------------------------------------------------------------------------
  //WDTCTL = WDTPW + WDTHOLD;		// Stop WDT

  //Config Clock (OJO Algunas de estas instrucciones son inútiles porque las pisa luego el initClocks() del USB)
  //UCSCTL4 = SELA_5 + SELM_5;		// ACLK and MCLK Source = XT2CLK
  //UCSCTL5 = DIVA_3;				// ACLK source divider = f(ACLK)/8
  //UCSCTL6 |= XTS;					// XT1 Mode Select = HF mode	(OJO ACTIVANDO EL HF MODE EL USB NO FUNCIONA!!)
  //UCSCTL6 |= XCAP_3;            	// Set Value of Internal load cap
  //UCSCTL6 &= ~(XT1OFF);			// XT1 On

  //UCSCTL4 |= SELS_5;

  // Loop until XT1 fault flag is cleared
  do
  {
	  UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);	// Clear XT2,XT1,DCO fault flags
	  SFRIFG1 &= ~OFIFG;                    		// Clear fault flags
  }while (SFRIFG1 & OFIFG);                   		// Test oscillator fault flag


  //--> BCSCTL2 |= SELM1 + SELM0;                     // Select the MCLK = LFXT1 (safe)
  //--> BCSCTL1 |= XTS + DIVA_3;                      // Select mode for LFXT1 = HF mode. Select ACLK divider = /8.         ACLK * 8 = LFXT1 = HF XTAL
  //-------------------------------------------------------------------------------

  #ifdef __WDT_ENABLED
  	  signatureWDT = 0x00;
	  cyclesWDT = WDT_MAX_RESET_CYCLES_INITIAL;
  #endif

//   /*COMMENT LINE FOR FINAL VERSION*/ FLASH_ClearSegment((unsigned char *)INFO_SEGMENT_B);	//DEBUG!! Clear INFO_B segment (v HW)
//   /*COMMENT LINE FOR FINAL VERSION*/ FLASH_ClearSegment((unsigned char *)INFO_SEGMENT_C);	//DEBUG!! Clear INFO_C segment (#serie)


  FLASH_ReadParameters();	//Cambio Sparta
//  memcpy(&ArrayTControl[REG_CODIGOMODULO1]  , (char *)INFO_SEGMENT_C , 8 /*sizeof(UNIT_INFO)*/);			//Cargamos código de módulo - mes fabricación - año fabricación - número serie
//  memcpy(&ArrayTControl[REG_VERSION_HW] 	, (char *)INFO_SEGMENT_B , 2 /*sizeof(UNIT_INFO_2)*/);			//Cargamos versión HW
//  memcpy(&ArrayTControl[REG_VERSION_FW] 	, (char *)INFO_SEGMENT_A , 2 /*sizeof(UNIT_INFO_3)*/);			//Cargamos versión FW

  Ports_Init();
  DAC_Config();
  I2C_Config();

  //initClocksNico();		//Init all CLK signals. MCLK=SMCLK=XT2=24MHz; ACLK=FLL=REFO=32kHz
  PDU_RTOS_Init();			//OJO! Es común a USB y RS485
  USB_Config();				//OJO! Iniciliza los relojes de TODO el sistema

  #ifdef __RS485_ENABLED
  	  //initClocks(24000000/*8000000*/);   	//MCLK=SMCLK=FLL=8MHz; ACLK=REFO=32kHz
	  UART_Config();
  	  UART_RTOS_Init();						//For UART Timeout
  #endif
  #ifdef __USB_ENABLED
  	  USB_RTOS_Init();
  	  //USB_Config();
  #endif

  ATT_Config ( SEL_ATT_PREVIO_BF );
  ATT_Config ( SEL_ATT_PREVIO_AF );
  ATT_Config ( SEL_ATT_GANANCIA_AF );
  ATT_SetTotalAttenuation ( ATT_MAX , BF );
  //ATT_SetTotalAttenuation ( ATT_MIN , BF );		/*DEBBUGING!! ERASE FOR FINAL VERSION*/
  ATT_SetTotalAttenuation ( 2*ATT_MAX , AF );

  TIMER_A1_Config();

  //ADC12_Config();

  #ifdef __WDT_ENABLED
  	  TIMER_B0_Config();
  #endif

  ADC12IE = 0x0000;		//NEEDED! otherwise ADC ISR halts continuously and collapses cpu.

  BIOS_start();
  POWER_ControlledReset(0);
}





//======== UNMI_ISR ========
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(UNMI_VECTOR))) UNMI_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
 #ifndef DRIVERLIB_LEGACY_MODE
            UCS_clearFaultFlag(UCS_XT2OFFG);
            UCS_clearFaultFlag(UCS_DCOFFG);
            SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
#else
            UCS_clearFaultFlag(UCS_BASE, UCS_XT2OFFG);
            UCS_clearFaultFlag(UCS_BASE, UCS_DCOFFG);
            SFR_clearInterrupt(SFR_BASE, SFR_OSCILLATOR_FAULT_INTERRUPT);

#endif
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:
            // If the CPU accesses USB memory while the USB module is
            // suspended, a "bus error" can occur.  This generates an NMI.  If
            // USB is automatically disconnecting in your software, set a
            // breakpoint here and see if execution hits it.  See the
            // Programmer's Guide for more information.
            SYSBERRIV = 0; //clear bus error flag
            USB_disable(); //Disable
    }
}

