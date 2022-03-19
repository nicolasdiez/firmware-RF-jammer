/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: HAL.h											*/
/********************************************************/

#ifndef __HAL_H
#define __HAL_H

#include "msp430f5638.h"
//#include "msp430f5636.h"

#ifdef	__MSP430F5638
	#include "msp430f5638.h"
#endif
#ifdef	__MSP430F5636
	#include "msp430f5636.h"
#endif

/*//RTOS Includes
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <xdc/runtime/Error.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>
//#include <ti/sysbios/knl/Clock.h>
//#include <ti/sysbios/knl/Task.h>
//#include <ti/sysbios/knl/Semaphore.h>
//#include <ti/sysbios/knl/Queue.h>
*/

//////////////////////////////
//Formato de variable de los Registros del sistema
union register_entry {
  signed short s_16;				//Caso de acceder al sistema principal (longitud de registro 2bytes)
  unsigned char u_char[2];			//Caso de acceder a subsistemas (I2C trabaja en unidades de 1byte)
};

//#pragma location=0x15940
//union register_entry array[0x106F];				//Nico: Cada slot de estos [0x106F] ocupa 2bytes

typedef struct
{
  char StandBy, ONreciente;
  char ATT;
  //char ATTTEMPCOUNTER;
  char WarSWR, ErrorSWR, OKSWR, WarPII, ErrorPII, OKPII, WarTEMP, ErrorTEMP, OKTEMP, WarTension, ErrorTension, OKTension, ErrorATETEMP;
  char CriticalSWR, CriticalTEMP;
  char MaxATETEMP;
  long TEMPanterior;

  char HPA_Stop;
  char System_Stop;
  char ControllerPrevio_ON;
  unsigned short PowerPrevioController_counter;
  char PrevioOK_StartHPA;
  char PrevioPotInsuf;
}FlagAlarms;

FlagAlarms Flags[2];							//Flags[0] for BF & Flags[1] for AF
//////////////////////////////

//RS485 and I2C Maximo Data Field Size
#define PDU_DATASIZE        256				//Max length of the registers

typedef struct {
	unsigned char DevSel;				//Nico: 8 bits
	unsigned short RegAdd;				//Nico: 16 bits
	unsigned char Data_Length;			//Nico: 8 bits. In BYTES !!!!!!
	unsigned char Data[PDU_DATASIZE];	//Nico: 8 bits. Array de 256 posiciones (tamaño max del campo 'Data') de 16 bits
}PDU_I2C;

typedef struct {
  unsigned short Add;						//Nico: 16bits
  unsigned char Frame_Number;				//Nico: 8bits
  unsigned char Command;					//Nico: 8bits
  unsigned short Register;					//Nico: 16bits
  unsigned char Data_Length;				//Nico: 8bits . In CHARS !!!!!!
  unsigned short Data[PDU_DATASIZE];		//Nico: Array de 256 posiciones (tamaño max del campo 'Data') de 16 bits
}PDU_RS485;

//*********** CARD TYPE CHOICE ********
#define IS_TARJETA_BF
//#define IS_TARJETA_AF
//*************************************

//*********** MODULE ENABLING *********
#define __RTOS_ENABLED
#define __WDT_ENABLED

//#define __FULL_DUPLEX
#define __RS485_ENABLED
#define __USB_ENABLED
//#define __USB_ECHO
#define __I2C_CHECKBUS_ENABLED

#define __ALARMASMARCAR_BFAF_ENABLED
#define __POWERCONTROLLER_BF_ENABLED
#define __POWERCONTROLLER_AF_ENABLED
#define __FSM_BF_ENABLED
#define __FSM_AF_ENABLED
//#define __AUTOSTARTSYSTEM_BF_ENABLED
//#define __AUTOSTARTSYSTEM_AF_ENABLED

//#define __SIMULATEGEN_ENABLED	//Simulate Read/Write from/to Generadora by using a local ArrayGen[]
#define __TIMERA1_ENABLED		// DO NOT DISSABLE !

#define __POST_USBRXEVENTS_IN_TASK
//*************************************


//*********************** WATCHDOG *********************
#define WDT_MAX_RESET_CYCLES_INITIAL        120/*80*//*bien 100*//*60*/
#define WDT_MAX_RESET_CYCLES                60/*100*//*bien 80*//*40*/	//WARNING! Si se bajan los ciclos de wdt hay que ver que todas las task firman en todos los casos! incluso ante saturación de bus de comunicaciones!!

#define SIGNATURE_TASK_PDURECEIVE			0x01
#define SIGNATURE_TASK_UART_RXTIMEOUT		0x02
#define SIGNATURE_TASK_USB_RXTIMEOUT		0x04
#define SIGNATURE_TASK_ACTUARBF				0x08
#define SIGNATURE_TASK_ACTUARAF				0x10
#define SIGNATURE_TASK_MARCARBFAF			0x20
//#define SIGNATURE_TASK_WATCHDOG			0x40

#define WDT_SIGNATURE_MASK                  0x3F/*0x24*//*0x34*//*0x17*//*0x3F*/

#define WDT_SIGNATURE_DISABLED              0x80  //For the USB key?
#define WDT_ADLY_14s                        WDT_ARST_1000
#define WDT_PLUS_HOLD                       WDTHOLD
//******************************************************

// Tiempo de Tick (en milisegundos)
#define SYSTEM_TICK_TIME        60

// This defines the timeout period for a millisecond timer using ACLK as clock source
#define MAIN_XTAL	32768/*7372800*/		//PREGUNTAR: De donde se saca este número? XIN XOUT
#define MS_TIMEOUT  (unsigned long int)((((MAIN_XTAL/8) * SYSTEM_TICK_TIME) * 0.001 ) / 2)
#define MCLK		20000000/*24000000*/

// Time interval definitions
#define MACRO_TIME_MILS(time) 		(time/SYSTEM_TICK_TIME)
#define MACRO_TIME_SECONDS(ticks)  	((unsigned long)(((unsigned long long)(ticks*SYSTEM_TICK_TIME))/1000))

// Time interval definitions
#define MACRO_TIME_MICROS(time) 	((unsigned long)(((unsigned long long)time)*(MAIN_XTAL/1000000)))

#define UART_A1_KBPS 			57600//9600//19200//57600//115200
//#define MACRO_delay_485(time) 	1145/(time/9600)

#define TABLA_HEX2ASCII 		"0123456789ABCDEF"
#define TABLA_ASCIIHEX  		"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00\x00\x00\x00\x00\x00\x00\x0A\x0B\x0C\x0D\x0E\x0F"

//USB
#define USB_TIMEOUT_MS			12000/*20000*/		//miliseconds [20000 * CPU_TICK (1000 us) = 20000000us = 20seg]
#define UART_TIMEOUT_MS			6000/*12000*/
#define CPU_TICKPERIOD_USEG		1000		//1000us = 1ms //Defined in .cfg file

//CAMPO COMMAND DE LA TRAMA RECIBIDA RS485
#define COMMAND_ACTION			0X11
#define WRITE               	0x22
#define READ                	0X33
#define RESET               	0xEE

//COMANDO-ACTION
#define ACTION_ENCLAVAMIENTO			0x01
#define ACTION_BITE						0x02
#define ACTION_CAMBIOPERFIL				0x03
#define ACTION_BORRARALARMAS			0x04
#define ACTION_BORRAREMERGENCIA			0x05
#define ACTION_RESET					0x06
#define ACTION_UPLOADFW					0x07
#define ACTION_TESTPREVIO				0x08
#define ACTION_CLEARFLASH				0x09
#define ACTION_READREGSFROMGENERADORA	0x0A
//define ACTION_XXXXXXXXXX				0x0B		//Reserved for some Generadora functionality
#define ACTION_TEST_ATT1				0x0C
#define ACTION_TEST_ATT2				0x0D
#define ACTION_TEST_LEDS				0x0E

#define true      		1
#define false     		0

#define YES				1
#define NO				0

#define I2C_WRITE		0
#define I2C_READ		1

#define NOTREADY		1
#define READY			0
#define ALARMS			2

#define USB_TX			0
#define USB_RX			1

#define BF				0
#define AF				1
#define UAF				2
#define ERROR_CARD		0xFF

#define OK				0
#define ERROR			1

#define RAISE			1	/*DAC mV*/
#define LOWER			0	/*DAC mV*/

//Mask Status Register TARJETA CONTROL - ESTADO OPERATIVO (registro sistema 0x0000)
#define ST_OK              				0x0001	//Bit 0
#define ST_ARRANCANDO      				0x0002	//Bit 1
#define ST_AUTOTEST        				0x0004	//Bit 2
#define ST_CAMPERFIL       				0x0008	//Bit 3
#define ST_USBTRASNFER    			 	0x0010	//Bit 4
#define ST_STANDBY_USER        			0x0020	//Bit 5
#define ST_ZEROING         				0x0040	//Bit 6
#define ST_STANDBY_HPA         			0x0080	//Bit 7
#define ST_WARNING_CONTROL				0x0100	//Bit 8
#define ST_CRITICAL_CONTROL				0x0200	//Bit 9
#define ST_WARNING_BF					0x0400	//Bit 10
#define ST_CRITICAL_BF					0x0800	//Bit 11
#define ST_WARNING_AF					0x1000	//Bit 12
#define ST_CRITICAL_AF					0x2000	//Bit 13

//Status Register GENERADORA
#define ST_GEN_TODOOK					0x0001	//Bit 0
#define ST_GEN_APAGADO					0x0010	//Bit 4
#define ST_GEN_ALARMASCRITICAS			0x8000	//Bit 15

//************** MEMORIA TARJETA DE CONTROL ************
#define TC_NUM_REGS						0x4E/*4D+1*/	//Num de registros 0x4A - 0x00

#define REG_TC_ESTADOOPERATIVO			0x00
#define REG_TC_ALARMASACTIVASTC			0x01
#define REG_TC_ALARMASNORECTC			0x02
#define REG_TC_ALARMASACTIVASHPABF		0x03
#define REG_TC_ALARMASNORECHPABF		0x04
#define REG_TC_ALARMASACTIVASHPAAF		0x05
#define REG_TC_ALARMASNORECHPAAF		0x06
#define REG_TC_PERFILACTIVO				0x09

//BF Monitors
#define REG_POTFWDOBJREAL_BF			0x0A	//Pot Fwd Objetivo user defined
#define REG_POTFWD_BF					0x0B
#define REG_POTRVS_BF					0x0C
#define REG_SWR_BF						0x0D	//RL
#define REG_T_BF						0x0E
#define REG_ATTTOT_BF					0x0F
#define REG_GAN2_BF						0x10
#define REG_POT1PREVIO_BF				0x11
#define	REG_TENSION_BF					0x12

//AF Monitors
#define REG_POTFWDOBJREAL_AF			0x13
#define REG_POTFWD_AF					0x14
#define REG_POTRVS_AF					0x15
#define REG_SWR_AF						0x16
#define REG_T_AF						0x17
#define REG_ATTTOT_AF					0x18
#define REG_ATT2_AF						0x19
#define REG_POT1PREVIO_AF				0x1A
#define	REG_TENSION_AF					0x1B

#define	REG_PMAX_HPABF					0x26		//Used to regulate the max output power of the system
#define	REG_PMAX_HPAAF					0x27		//Used to regulate the max output power of the system

#define REG_MUTE_DCDCs					0x28
#define REG_MUTE_HPAs					0x29

//Umbrales BF
#define REG_POTFWDOBJUSER_BF			0x2A
#define REG_UWARPOTFWDINSUF_BF			0x2B
#define REG_UERRPOTFWDINSUF_BF			0x2C
#define REG_UWARSWR_BF					0x2D	//RL
#define REG_UERRSWR_BF					0x2E
#define REG_UWARINFTEMP_BF				0x2F
#define REG_UERRINFTEMP_BF				0x30
#define REG_UWARSUPTEMP_BF				0x31
#define REG_UERRSUPTEMP_BF				0x32
#define REG_UWARINFTENSION_BF			0x33
#define REG_UERRINFTENSION_BF			0x34
#define REG_UWARSUPTENSION_BF			0x35
#define REG_UERRSUPTENSION_BF			0x36

//Umbrales AF
#define REG_POTFWDOBJUSER_AF			0x37
#define REG_UWARPOTFWDINSUF_AF			0x38
#define REG_UERRPOTFWDINSUF_AF			0x39
#define REG_UWARSWR_AF					0x3A
#define REG_UERRSWR_AF					0x3B
#define REG_UWARINFTEMP_AF				0x3C
#define REG_UERRINFTEMP_AF				0x3D
#define REG_UWARSUPTEMP_AF				0x3E
#define REG_UERRSUPTEMP_AF				0x3F
#define REG_UWARINFTENSION_AF			0x40
#define REG_UERRINFTENSION_AF			0x41
#define REG_UWARSUPTENSION_AF			0x42
#define REG_UERRSUPTENSION_AF			0x43

#define REG_TIPOTARJETA					0x44	//RF card type (0:BF 1:AF)
#define REG_CODIGOMODULO1				0x45	//0C
#define REG_CODIGOMODULO2				0x46	//CC
#define REG_MESANYO						0x47	//MA
#define REG_NUMSERIE					0x48	//XX
#define REG_VERSION_HW					0x49	//version HW
#define REG_VERSION_FW					0x4A	//version FW

#define REG_POTPREVIOOBJ_BF				0x4B
#define REG_POTPREVIOOBJ_AF				0x4C
#define REG_DIRECCION_RS485				0x4D
//******************************************************

//************** MEMORIA TARJETA GENERADORA ************
#define GEN_NUM_REGS					0xD1/*D0+1*/	//Num de registros 0x10D0 - 0x1000
#define REG_START_GEN					0x1000
#define REG_END_GEN						0x10D0

#define REG_GEN_ESTADOOPERATIVO			0x00
#define REG_GEN_ALARMASACTIVAS			0x01
#define REG_GEN_ALARMASNOREC			0x03
#define REG_GEN_PERFILACTIVO			0x05
#define REG_GEN_MASCARAPERFILES			0x06
#define REG_GEN_PROGCAMBIOPERFIL		0x07
#define REG_GEN_CLAVECIFRADO			0x08
#define REG_GEN_IDPLANINHIBICION		0x10
#define REG_GEN_TEMPDAC					0x18
#define REG_GEN_NUMSERIE				0x32
#define REG_GEN_VERSIONHW				0x36
#define REG_GEN_VERSIONFW				0x37
#define REG_GEN_LONGINFOUSUARIO			0x38
#define REG_GEN_INFOUSUARIO				0x39
#define REG_GEN_LONGSEMILLA				0xB9
#define REG_GEN_LRCSEMILLA				0xBB
#define REG_GEN_COMANDO					0xD0

#define REG_GEN_COMMAND					0x1040
//******************************************************

//****************** BITS DE ALARMAS HPA ****************
// MASK ALARMS para registros STATUS DE ALARMAS
#define SI_CRITICAL						0xFF00
#define SI_WARNING						0x00FF

#define CRITICAL_POTPREVIO_INSUF		0x0100	//Bit 8
#define CRITICAL_POTPREVIO_EXCES		0x0200	//Bit 9
#define CRITICAL_FALLO_HPA				0x0400	//Bit 10
#define CRITICAL_SWR					0x0800	//Bit 11
#define CRITICAL_T  	       			0x1000	//Bit 12
#define CRITICAL_POT_INS 	    		0x2000	//Bit 13
#define CRITICAL_POT_PREVIO 			0x4000	//Bit 14
#define CRITICAL_TENSION	 			0x8000	//Bit 15

#define WARNING_SWR						0x0008	//Bit 3
#define WARNING_T          				0x0010	//Bit 4
#define WARNING_POT_INS					0x0020	//Bit 5
#define WARNING_TENSION					0x0080	//Bit 7
//******************************************************

//****************** BITS DE ALARMAS TC ****************
#define CRITICAL_INIT_PARAMS_GEN		0x0100	//Bit 8
#define CRITICAL_RESET_I2CBUS			0x0200	//Bit 9
#define CRITICAL_READ_CARDTYPE			0x0400	//Bit 10
//******************************************************

//********************* ATENUADORES ********************
#define	SEL_ATT_PREVIO_BF			0
#define SEL_ATT_PREVIO_AF			1
#define SEL_ATT_GANANCIA_AF			2

#define ATT_PREVIO_BF_PIN			P1IN
#define ATT_PREVIO_BF_OUT			P1OUT
#define ATT_PREVIO_BF_DIR			P1DIR
#define ATT_PREVIO_BF_V7_BIT		0
#define ATT_PREVIO_BF_V7_MASK		(1<<ATT_PREVIO_BF_V7_BIT)
#define ATT_PREVIO_BF_V6_BIT		1
#define ATT_PREVIO_BF_V6_MASK		(1<<ATT_PREVIO_BF_V6_BIT)
#define ATT_PREVIO_BF_V5_BIT		2
#define ATT_PREVIO_BF_V5_MASK		(1<<ATT_PREVIO_BF_V5_BIT)
#define ATT_PREVIO_BF_V4_BIT		3
#define ATT_PREVIO_BF_V4_MASK		(1<<ATT_PREVIO_BF_V4_BIT)
#define ATT_PREVIO_BF_V3_BIT		4
#define ATT_PREVIO_BF_V3_MASK		(1<<ATT_PREVIO_BF_V3_BIT)
#define ATT_PREVIO_BF_V2_BIT		5
#define ATT_PREVIO_BF_V2_MASK		(1<<ATT_PREVIO_BF_V2_BIT)
#define ATT_PREVIO_BF_V1_BIT		6
#define ATT_PREVIO_BF_V1_MASK		(1<<ATT_PREVIO_BF_V1_BIT)
#define ATT_PREVIO_BF_LE_BIT		7
#define ATT_PREVIO_BF_LE_MASK		(1<<ATT_PREVIO_BF_LE_BIT)

#define ATT_PREVIO_AF_PIN			P3IN
#define ATT_PREVIO_AF_OUT			P3OUT
#define ATT_PREVIO_AF_DIR			P3DIR
#define ATT_PREVIO_AF_V7_BIT		0
#define ATT_PREVIO_AF_V7_MASK		(1<<ATT_PREVIO_AF_V7_BIT)
#define ATT_PREVIO_AF_V6_BIT		1
#define ATT_PREVIO_AF_V6_MASK		(1<<ATT_PREVIO_AF_V6_BIT)
#define ATT_PREVIO_AF_V5_BIT		2
#define ATT_PREVIO_AF_V5_MASK		(1<<ATT_PREVIO_AF_V5_BIT)
#define ATT_PREVIO_AF_V4_BIT		3
#define ATT_PREVIO_AF_V4_MASK		(1<<ATT_PREVIO_AF_V4_BIT)
#define ATT_PREVIO_AF_V3_BIT		4
#define ATT_PREVIO_AF_V3_MASK		(1<<ATT_PREVIO_AF_V3_BIT)
#define ATT_PREVIO_AF_V2_BIT		5
#define ATT_PREVIO_AF_V2_MASK		(1<<ATT_PREVIO_AF_V2_BIT)
#define ATT_PREVIO_AF_V1_BIT		6
#define ATT_PREVIO_AF_V1_MASK		(1<<ATT_PREVIO_AF_V1_BIT)
#define ATT_PREVIO_AF_LE_BIT		7
#define ATT_PREVIO_AF_LE_MASK		(1<<ATT_PREVIO_AF_LE_BIT)

#define ATT_GANANCIA_AF_PIN			P4IN
#define ATT_GANANCIA_AF_OUT			P4OUT
#define ATT_GANANCIA_AF_DIR			P4DIR
#define ATT_GANANCIA_AF_V7_BIT		0
#define ATT_GANANCIA_AF_V7_MASK		(1<<ATT_GANANCIA_AF_V7_BIT)
#define ATT_GANANCIA_AF_V6_BIT		1
#define ATT_GANANCIA_AF_V6_MASK		(1<<ATT_GANANCIA_AF_V6_BIT)
#define ATT_GANANCIA_AF_V5_BIT		2
#define ATT_GANANCIA_AF_V5_MASK		(1<<ATT_GANANCIA_AF_V5_BIT)
#define ATT_GANANCIA_AF_V4_BIT		3
#define ATT_GANANCIA_AF_V4_MASK		(1<<ATT_GANANCIA_AF_V4_BIT)
#define ATT_GANANCIA_AF_V3_BIT		4
#define ATT_GANANCIA_AF_V3_MASK		(1<<ATT_GANANCIA_AF_V3_BIT)
#define ATT_GANANCIA_AF_V2_BIT		5
#define ATT_GANANCIA_AF_V2_MASK		(1<<ATT_GANANCIA_AF_V2_BIT)
#define ATT_GANANCIA_AF_V1_BIT		6
#define ATT_GANANCIA_AF_V1_MASK		(1<<ATT_GANANCIA_AF_V1_BIT)
#define ATT_GANANCIA_AF_LE_BIT		7
#define ATT_GANANCIA_AF_LE_MASK		(1<<ATT_GANANCIA_AF_LE_BIT)

#define ATT_MAX        	3175/*(31.75dB*)*//*155*/       // Atenuador Real (dB*100)
#define ATT_MIN        	0
#define ATT_FACTORREAL	25							//Conversion Factor
//******************************************************


//******************** LINEAS DE MUTE ******************
#define MUTE_HPA1_PIN				P9IN
#define MUTE_HPA1_POUT				P9OUT
#define MUTE_HPA1_PDIR				P9DIR
#define MUTE_HPA1_BIT				0
#define MUTE_HPA1_MASK				(1<<MUTE_HPA1_BIT)

#define MUTE_HPA2_PIN				P9IN
#define MUTE_HPA2_POUT				P9OUT
#define MUTE_HPA2_PDIR				P9DIR
#define MUTE_HPA2_BIT				1
#define MUTE_HPA2_MASK				(1<<MUTE_HPA2_BIT)
//******************************************************

//********************* ENABLE DC/DC *******************
#define MUTE_DCDC_BF_PIN			P2IN
#define MUTE_DCDC_BF_POUT			P2OUT
#define MUTE_DCDC_BF_DIR			P2DIR
#define MUTE_DCDC_BF_BIT			3
#define MUTE_DCDC_BF_MASK			(1<<MUTE_DCDC_BF_BIT)

#define MUTE_DCDC_AF_PIN			P2IN
#define MUTE_DCDC_AF_POUT			P2OUT
#define MUTE_DCDC_AF_DIR			P2DIR
#define MUTE_DCDC_AF_BIT			4
#define MUTE_DCDC_AF_MASK			(1<<MUTE_DCDC_AF_BIT)
//******************************************************

//********************* I2C AUX *******************
#define I2C_SCLAUX_PIN			P2IN
#define I2C_SCLAUX_POUT			P2OUT
#define I2C_SCLAUX_DIR			P2DIR
#define I2C_SCLAUX_BIT			2
#define I2C_SCLAUX_MASK			(1<<MUTE_DCDC_BF_BIT)
//******************************************************

//********************** LEDS ***********************
// ALARMS LED
#define LED_1_PIN				P9IN
#define LED_1_POUT				P9OUT
#define LED_1_DIR				P9DIR
#define LED_1_BIT				2
#define LED_1_MASK				(1<<LED_1_BIT)

// ON-OFF LED
#define LED_2_PIN				P9IN
#define LED_2_POUT				P9OUT
#define LED_2_DIR				P9DIR
#define LED_2_BIT				3
#define LED_2_MASK				(1<<LED_2_BIT)

//BUS LED
#define LED_3_PIN				P9IN
#define LED_3_POUT				P9OUT
#define LED_3_DIR				P9DIR
#define LED_3_BIT				4
#define LED_3_MASK				(1<<LED_3_BIT)

//
#define LED_4_PIN				P9IN
#define LED_4_POUT				P9OUT
#define LED_4_DIR				P9DIR
#define LED_4_BIT				5
#define LED_4_MASK				(1<<LED_4_BIT)
//******************************************************

//******************** AUX (FANS?) ******************
#define AUX_1_PIN				P9IN
#define AUX_1_POUT				P9OUT
#define AUX_1_DIR				P9DIR
#define AUX_1_BIT				7
#define AUX_1_MASK				(1<<AUX_1_BIT)

#define AUX_2_PIN				P9IN
#define AUX_2_POUT				P9OUT
#define AUX_2_DIR				P9DIR
#define AUX_2_BIT				6
#define AUX_2_MASK				(1<<AUX_2_BIT)
//******************************************************

//********************* RS485 ENABLE *******************
#define RS485_TXENABLE_DIR    		P8DIR
#define RS485_TXENABLE_PIN    		P8IN
#define RS485_TXENABLE_POUT   		P8OUT
#define RS485_TXENABLE_BIT    		0
#define RS485_TXENABLE_MASK   		(1<<RS485_TXENABLE_BIT)

#define RS485_RXENABLE_DIR    		P8DIR
#define RS485_RXENABLE_PIN    		P8IN
#define RS485_RXENABLE_POUT   		P8OUT
#define RS485_RXENABLE_BIT    		1
#define RS485_RXENABLE_MASK   		(1<<RS485_RXENABLE_BIT)

#define RS485_TERMINADORA_DIR  		P8DIR
#define RS485_TERMINADORA_PIN		P8IN
#define RS485_TERMINADORA_POUT		P8OUT
#define RS485_TERMINADORA_BIT  		7
#define RS485_TERMINADORA_MASK		(1<<RS485_TERMINADORA_BIT)

//RS485 Frame
#define UART_RX_SIZE_MAX 	196/*Sparta 128*//*48*//*532*/				//Tamaño máximo del buffer de recepción de la TRAMA RS485 EN CHARS = (1 + 2 + 1 + 1 + 2 + 1 + 256 + 1 + 1) * 2
#define UART_TX_SIZE_MAX 	196/*Sparta 128*//*48*//*532*/
#define USB_RX_SIZE_MAX		128
#define USB_BUFFER_SIZE 	128/*256*//*532*//*1024*/		//Cambio Sparta

//RS485 Frame Field Sizes
#define LENGTH_HEADER		2
#define LENGTH_ADDRESS		4
#define LENGTH_FRAMENUM		2
#define LENGTH_COMMAND		2
#define LENGTH_REGISTERS	4
#define LENGTH_DATALENGTH	2
#define LENGTH_LRC			2
#define LENGTH_ENDMARK		2
//******************************************************

//************************** DAC ***********************
#define DAC1_DIR  		P7DIR
#define DAC1_PIN		P7IN
#define DAC1_POUT		P7OUT
#define DAC1_BIT  		7
#define DAC1_MASK		(1<<DAC1_BIT)
//******************************************************

//************* VALORES POR DEFECTO TC ****************
#define DEFAULT_TC_ESTADOOPERATIVO			0x0001
#define DEFAULT_TC_ALARMASACTIVASTC			0x0000
#define DEFAULT_TC_ALARMASNORECTC			0x0000
#define DEFAULT_TC_ALARMASACTIVASHPABF		0x0000
#define DEFAULT_TC_ALARMASNORECHPABF		0x0000
#define DEFAULT_TC_ALARMASACTIVASHPAAF		0x0000
#define DEFAULT_TC_ALARMASNORECHPAAF		0x0000
#define DEFAULT_TC_PERFILACTIVO				0x00FF

//BF Monitors
#define DEFAULT_POTFWDOBJREAL_BF			400	//Pot Fwd Objetivo user defined
#define DEFAULT_POTFWD_BF					400
#define DEFAULT_POTRVS_BF					200
#define DEFAULT_SWR_BF						200	//RL
#define DEFAULT_T_BF						30
#define DEFAULT_ATTTOT_BF					3175
#define DEFAULT_GAN2_BF						0
#define DEFAULT_POT1PREVIO_BF				-100

//AF Monitors
#define DEFAULT_POTFWDOBJREAL_AF			400
#define DEFAULT_POTFWD_AF					400
#define DEFAULT_POTRVS_AF					200
#define DEFAULT_SWR_AF						200
#define DEFAULT_T_AF						30
#define DEFAULT_ATTTOT_AF					6350
#define DEFAULT_ATT2_AF						3175
#define DEFAULT_POT1PREVIO_AF				0

#define DEFAULT_MUTE_HPAs					0x0003
#define DEFAULT_MUTE_DCDCs					0x0003

//Umbrales BF
#define DEFAULT_POTFWDOBJUSER_BF			460
#define DEFAULT_UWARPOTFWDINSUF_BF			0
#define DEFAULT_UERRPOTFWDINSUF_BF			400
#define DEFAULT_UWARSWR_BF					80	//RL
#define DEFAULT_UERRSWR_BF					60
#define DEFAULT_UWARINFTEMP_BF				60
#define DEFAULT_UERRINFTEMP_BF				50
#define DEFAULT_UWARSUPTEMP_BF				70
#define DEFAULT_UERRSUPTEMP_BF				80

//Umbrales AF
#define DEFAULT_POTFWDOBJUSER_AF			440
#define DEFAULT_UWARPOTFWDINSUF_AF			0
#define DEFAULT_UERRPOTFWDINSUF_AF			400
#define DEFAULT_UWARSWR_AF					80
#define DEFAULT_UERRSWR_AF					60
#define DEFAULT_UWARINFTEMP_AF				60
#define DEFAULT_UERRINFTEMP_AF				50
#define DEFAULT_UWARSUPTEMP_AF				70
#define DEFAULT_UERRSUPTEMP_AF				80

#define DEFAULT_CODIGOMODULO1				0	//0C
#define DEFAULT_CODIGOMODULO2				0	//CC
#define DEFAULT_MESANYO						0	//MA
#define DEFAULT_NUMSERIE					0	//XX
#define DEFAULT_VERSION_HW					0	//version HW
#define DEFAULT_VERSION_FW					0	//version FW

#define DEFAULT_POTPREVIOOBJ_BF				0
#define DEFAULT_POTPREVIOOBJ_AF				100
//******************************************************

//************* VALORES POR DEFECTO GEN ****************
#define DEFAULT_GEN_ESTADOOPERATIVO			0x0001
#define DEFAULT_GEN_ALARMASACTIVAS			0
#define DEFAULT_GEN_ALARMASNOREC			0
#define DEFAULT_GEN_PERFILACTIVO			0x00FF
#define DEFAULT_GEN_MASCARAPERFILES			0
#define DEFAULT_GEN_PROGCAMBIOPERFIL		0
#define DEFAULT_GEN_CLAVECIFRADO			0
#define DEFAULT_GEN_IDPLANINHIBICION		0
#define DEFAULT_GEN_TEMPDAC					30
#define DEFAULT_GEN_NUMSERIE				0
#define DEFAULT_GEN_VERSIONHW				0
#define DEFAULT_GEN_VERSIONFW				0
#define DEFAULT_GEN_LONGINFOUSUARIO			0
#define DEFAULT_GEN_INFOUSUARIO				0
#define DEFAULT_GEN_LONGSEMILLA				0
#define DEFAULT_GEN_LRCSEMILLA				0
#define DEFAULT_GEN_COMANDO					0
//******************************************************

#endif
