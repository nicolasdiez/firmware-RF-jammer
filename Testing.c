#include "HAL.h"
#include "ATT.h"
//#include "msp430f5636.h"
#include <xdc/cfg/global.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <ti/sysbios/knl/Event.h>

#include <xdc/runtime/Log.h>

#include "USB_app/usbConstructs.h"
#include <xdc/runtime/System.h>

#include "msp430.h"
#include "driverlib.h"

#include "USB_test.h"
#include "HAL.h"
#include "PDU.h"
#include "UART.h"

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
#endif

//Crear esta Task con P=4
void ATT_Loop (UArg arg0, UArg arg1)
{
	while(1)
	{
		short i=0;
		signed long ATT=0;

		for(i=0 ; i<25 ; i++)
		{
			ATT_SetAttenuation (ATT , SEL_ATT_PREVIO_BF);
			__delay_cycles(10000000);
			ATT = ATT + 50;
		}
	}
}




