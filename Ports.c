/********************************************************/
/* Author: Nicolás Díez Risueño							*/
/* Date: July 2014										*/
/* File: Ports.c										*/
/********************************************************/

#include "HAL.h"
#include "Ports.h"

//---------------------------------------- VARIABLE DECLARATIONS ----------------------------------------

//------------------------------------------- FUCNTION METHODS ------------------------------------------
void Ports_Init(void)
{
	//DAC
	DAC1_POUT &= ~DAC1_MASK;
	DAC1_DIR |= DAC1_MASK;

	//Pin Aux 1	- TESTING: Using this pin to provide 3V3 to Generadora (Connector J12-1)
	AUX_1_POUT &= ~AUX_1_MASK;
	AUX_1_DIR |= AUX_1_MASK;

	//VENTILADORES ENABLE (0 = OFF / 1 = ON)
	//AUX_1_POUT &= ~AUX_1_MASK;
	//AUX_1_DIR |= AUX_1_MASK;

	AUX_2_POUT &= ~AUX_2_MASK;	//P9.6 - Used also as toogle pin for timing purposes
	AUX_2_DIR |= AUX_2_MASK;

	//AUX_3_POUT &= ~AUX_3_MASK;
	//AUX_3_DIR |= AUX_3_MASK;

	//RS_485 communications Data Enable
	RS485_TXENABLE_POUT &= ~RS485_TXENABLE_MASK;		//P8.0 set to 0
	RS485_TXENABLE_DIR |= RS485_TXENABLE_MASK;			//P8.0 direction set as output

	RS485_RXENABLE_POUT &= ~RS485_RXENABLE_MASK;		//P8.1 set to 0
	RS485_RXENABLE_DIR |= RS485_RXENABLE_MASK;			//P8.1 direction set as output

	RS485_TERMINADORA_POUT &= ~RS485_TERMINADORA_MASK;
	RS485_TERMINADORA_DIR |= RS485_TERMINADORA_MASK;

	//Señales de MUTE para HPA BF y HPA AF
	MUTE_HPA1_POUT |= MUTE_HPA1_MASK;	//Power-up with HPAs dissabled
	MUTE_HPA2_POUT |= MUTE_HPA2_MASK;

	MUTE_HPA1_PDIR |= MUTE_HPA1_MASK;
	MUTE_HPA2_PDIR |= MUTE_HPA2_MASK;

	//Mute DC/DC 1 (BF)
	MUTE_DCDC_BF_POUT |= MUTE_DCDC_BF_MASK;	//Power-up with DCDCs dissabled
	MUTE_DCDC_BF_DIR |= MUTE_DCDC_BF_MASK;	//output

	//Mute DC/DC 2 (AF)
	MUTE_DCDC_AF_POUT |= MUTE_DCDC_AF_MASK;
	MUTE_DCDC_AF_DIR |= MUTE_DCDC_AF_MASK;	//output

	//LEDs
	LED_1_POUT &= ~LED_1_MASK;
	LED_1_DIR |= LED_1_MASK;	//output

	LED_2_POUT &= ~LED_2_MASK;
	LED_2_DIR |= LED_2_MASK;	//output

	LED_3_POUT &= ~LED_3_MASK;
	LED_3_DIR |= LED_3_MASK;	//output

	LED_4_POUT &= ~LED_4_MASK;
	LED_4_DIR |= LED_4_MASK;	//output

	//I2C AUX
	I2C_SCLAUX_POUT |= I2C_SCLAUX_MASK;
	I2C_SCLAUX_DIR |= I2C_SCLAUX_MASK;

	//PIN AUX (parece que no funciona este pin 9.7)
	//AUX_4_POUT &= ~AUX_4_MASK;
	//AUX_4_DIR |= AUX_4_MASK;

}
