
XOPTS = -I"c:/ti/xdctools_3_30_01_25_core/packages/" -Dxdc_target_types__=C:/ti/tirtos_msp430_2_10_01_38/products/bios_6_41_00_26/packages/ti/targets/msp430/elf/std.h -Dxdc_target_name__=MSP430X

vpath % C:/ti/tirtos_msp430_2_10_01_38/products/bios_6_41_00_26/packages/ti/sysbios/
vpath %.c c:/ti/xdctools_3_30_01_25_core/packages/

CCOPTS    = -vmspx --near_data=none --code_model=large --data_model=restricted -q --advice:power=1  --program_level_compile -g

XDC_ROOT  = c:/ti/xdctools_3_30_01_25_core/packages/

BIOS_ROOT = C:/ti/tirtos_msp430_2_10_01_38/products/bios_6_41_00_26/packages/ti/sysbios/

BIOS_DEFS =  -Dti_sysbios_BIOS_swiEnabled__D=TRUE -Dti_sysbios_BIOS_taskEnabled__D=TRUE -Dti_sysbios_BIOS_clockEnabled__D=TRUE -Dti_sysbios_BIOS_runtimeCreatesEnabled__D=TRUE -Dti_sysbios_hal_Hwi_DISABLE_ALL_HOOKS -Dti_sysbios_knl_Swi_DISABLE_ALL_HOOKS -Dti_sysbios_knl_Task_DISABLE_ALL_HOOKS -Dti_sysbios_BIOS_smpEnabled__D=FALSE -Dti_sysbios_Build_useHwiMacros -Dti_sysbios_knl_Swi_numPriorities__D=16 -Dti_sysbios_knl_Task_deleteTerminatedTasks__D=FALSE -Dti_sysbios_knl_Task_numPriorities__D=16 -Dti_sysbios_knl_Task_checkStackFlag__D=TRUE -Dti_sysbios_knl_Clock_TICK_SOURCE=ti_sysbios_knl_Clock_TickSource_TIMER -Dti_sysbios_knl_Clock_TICK_MODE=ti_sysbios_knl_Clock_TickMode_PERIODIC -Dti_sysbios_hal_Core_delegate_getId=ti_sysbios_hal_CoreNull_getId__E -Dti_sysbios_hal_Core_delegate_interruptCore=ti_sysbios_hal_CoreNull_interruptCore__E -Dti_sysbios_hal_Core_delegate_lock=ti_sysbios_hal_CoreNull_lock__E -Dti_sysbios_hal_Core_delegate_unlock=ti_sysbios_hal_CoreNull_unlock__E -Dti_sysbios_hal_Core_numCores__D=1 -Dti_sysbios_hal_CoreNull_numCores__D=1 -Dti_sysbios_utils_Load_taskEnabled__D=TRUE -Dti_sysbios_utils_Load_swiEnabled__D=FALSE -Dti_sysbios_family_msp430_Hwi_dispatcherSwiSupport__D=TRUE -Dti_sysbios_family_msp430_Hwi_dispatcherTaskSupport__D=TRUE -Dti_sysbios_family_msp430_Hwi_dispatcherAutoNestingSupport__D=FALSE -Dti_sysbios_family_msp430_Hwi_dispatcherIrpTrackingSupport__D=TRUE -Dti_sysbios_knl_Semaphore_supportsEvents__D=FALSE -Dti_sysbios_knl_Semaphore_supportsPriority__D=TRUE

BIOS_INC  = -I"C:/ti/tirtos_msp430_2_10_01_38/products/bios_6_41_00_26/packages/"

TARGET_INC = -I"C:/ti/tirtos_msp430_2_10_01_38/products/bios_6_41_00_26/packages/"

INCS = $(BIOS_INC) $(TARGET_INC)  

CC = c:/ti/ccsv6/tools/compiler/msp430_4.3.1/bin/cl430 --abi=eabi -c $(CCOPTS) -I c:/ti/ccsv6/tools/compiler/msp430_4.3.1/include
ASM = c:/ti/ccsv6/tools/compiler/msp430_4.3.1/bin/cl430 --abi=eabi -c $(CCOPTS) -I c:/ti/ccsv6/tools/compiler/msp430_4.3.1/include
AR = c:/ti/ccsv6/tools/compiler/msp430_4.3.1/bin/ar430 rq

DEL = cmd.exe /q /c del /f
CP = cmd.exe /q /c copy /Y

define RM
	$(if $(wildcard $1),$(DEL) $1,:)
endef

define ASSEMBLE
	@echo asme430X $< ...
	@$(ASM) $(BIOS_DEFS) $(XOPTS) $(INCS) $<
endef

all: sysbios.ae430X

msp430_Hwi_switch_asm.obj: family/msp430/Hwi_switch_asm.asm makefile
	@-$(call RM, $@)
	$(ASSEMBLE) --output_file=msp430_Hwi_switch_asm.obj

msp430_TaskSupport_asm.obj: family/msp430/TaskSupport_asm.asm makefile
	@-$(call RM, $@)
	$(ASSEMBLE) --output_file=msp430_TaskSupport_asm.obj


BIOS.obj: BIOS.c family/msp430/Hwi.c family/msp430/IntrinsicsSupport.c family/msp430/TaskSupport.c family/msp430/Timer.c family/msp430/TimestampProvider.c family/msp430/Power.c family/msp430/ClockFreqs.c knl/Clock.c knl/Idle.c knl/Intrinsics.c knl/Event.c knl/Queue.c knl/Semaphore.c knl/Swi.c knl/Swi_andn.c knl/Task.c hal/Core.c hal/CoreNull.c hal/Hwi.c hal/Hwi_stack.c hal/Hwi_startup.c gates/GateHwi.c gates/GateMutex.c heaps/HeapMem.c ./HwiFuncs.c makefile
	@-$(call RM, $@)
	@echo cle430X $< ...
	@$(CC) $(BIOS_DEFS) $(XOPTS) $(INCS) \
	$(BIOS_ROOT)BIOS.c \
	$(BIOS_ROOT)family/msp430/Hwi.c \
	$(BIOS_ROOT)family/msp430/IntrinsicsSupport.c \
	$(BIOS_ROOT)family/msp430/TaskSupport.c \
	$(BIOS_ROOT)family/msp430/Timer.c \
	$(BIOS_ROOT)family/msp430/TimestampProvider.c \
	$(BIOS_ROOT)family/msp430/Power.c \
	$(BIOS_ROOT)family/msp430/ClockFreqs.c \
	$(BIOS_ROOT)knl/Clock.c \
	$(BIOS_ROOT)knl/Idle.c \
	$(BIOS_ROOT)knl/Intrinsics.c \
	$(BIOS_ROOT)knl/Event.c \
	$(BIOS_ROOT)knl/Queue.c \
	$(BIOS_ROOT)knl/Semaphore.c \
	$(BIOS_ROOT)knl/Swi.c \
	$(BIOS_ROOT)knl/Swi_andn.c \
	$(BIOS_ROOT)knl/Task.c \
	$(BIOS_ROOT)hal/Core.c \
	$(BIOS_ROOT)hal/CoreNull.c \
	$(BIOS_ROOT)hal/Hwi.c \
	$(BIOS_ROOT)hal/Hwi_stack.c \
	$(BIOS_ROOT)hal/Hwi_startup.c \
	$(BIOS_ROOT)gates/GateHwi.c \
	$(BIOS_ROOT)gates/GateMutex.c \
	$(BIOS_ROOT)heaps/HeapMem.c \
	./HwiFuncs.c \

sysbios.ae430X: BIOS.obj msp430_Hwi_switch_asm.obj msp430_TaskSupport_asm.obj
	@-$(call RM, $@)
	@echo are430X $^ ...
	@$(AR) $@ $^


clean:
	@-$(call RM, sysbios.ae430X)
	@-$(call RM, BIOS.obj)
	@-$(call RM, msp430_Hwi_switch_asm.obj)
	@-$(call RM, msp430_TaskSupport_asm.obj)
