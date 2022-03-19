#include <xdc/std.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#define ti_sysbios_family_msp430_Hwi__internalaccess
#include <ti/sysbios/family/msp430/Hwi.h>
#include <ti/sysbios/family/msp430/Power.h>

extern ti_sysbios_family_msp430_Hwi_Object ti_sysbios_family_msp430_Hwi_Object__table__V[];

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi45_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void I2C_ISR(UArg);
#if defined(__ICC430__)
#pragma vector = 45 * 2
#else
#pragma vector = 45;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi45(Void)
{
    UInt taskKey;

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi45_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi45_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    I2C_ISR(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi46_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void UART_ISR(UArg);
#if defined(__ICC430__)
#pragma vector = 46 * 2
#else
#pragma vector = 46;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi46(Void)
{
    UInt taskKey;

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi46_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi46_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    UART_ISR(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi49_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void ISR_TimerA1(UArg);
#if defined(__ICC430__)
#pragma vector = 49 * 2
#else
#pragma vector = 49;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi49(Void)
{
    UInt taskKey;

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi49_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi49_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    ISR_TimerA1(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi51_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void iUsbInterruptHandler(UArg);
#if defined(__ICC430__)
#pragma vector = 51 * 2
#else
#pragma vector = 51;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi51(Void)
{
    UInt taskKey;

    /* keep CPU awake upon RETI */
    __bic_SR_register_on_exit(0xF0);

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi51_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi51_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    iUsbInterruptHandler(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi53_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void ti_sysbios_family_msp430_Timer_periodicStub__E(UArg);
#if defined(__ICC430__)
#pragma vector = 53 * 2
#else
#pragma vector = 53;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi53(Void)
{
    UInt taskKey;

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi53_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi53_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    ti_sysbios_family_msp430_Timer_periodicStub__E(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi54_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void ADC12_ISR(UArg);
#if defined(__ICC430__)
#pragma vector = 54 * 2
#else
#pragma vector = 54;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi54(Void)
{
    UInt taskKey;

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi54_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi54_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    ADC12_ISR(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}

extern Void ti_sysbios_family_xxx_Hwi_switchAndRunFunc(Void (*func)());

extern Void ti_sysbios_family_msp430_Hwi59_p2(Void);

#if defined(__ICC430__)
#pragma inline=never
#endif
extern Void ISR_TimerB0(UArg);
#if defined(__ICC430__)
#pragma vector = 59 * 2
#else
#pragma vector = 59;
#endif
__interrupt Void ti_sysbios_family_msp430_Hwi59(Void)
{
    UInt taskKey;

    /* disable Task scheduler */
    taskKey = ti_sysbios_knl_Task_disable();

    /* switch stacks and then run the phase 2 function */
    ti_sysbios_family_xxx_Hwi_switchAndRunFunc(&ti_sysbios_family_msp430_Hwi59_p2);

    /* handle any Task re-scheduling as required */
    ti_sysbios_knl_Task_restoreHwi(taskKey);

}

Void ti_sysbios_family_msp430_Hwi59_p2(Void)
{
    ti_sysbios_BIOS_ThreadType prevThreadType;
    UInt swiKey;

    /* disable Swi scheduler */
    swiKey = ti_sysbios_knl_Swi_disable();

    /* set thread type to Hwi */
    prevThreadType = ti_sysbios_BIOS_setThreadType(ti_sysbios_BIOS_ThreadType_Hwi);

    /* run ISR function */
    ISR_TimerB0(0);

    /* run any posted Swis */
    ti_sysbios_knl_Swi_restoreHwi(swiKey);

    /* restore thread type */
    ti_sysbios_BIOS_setThreadType(prevThreadType);

}


