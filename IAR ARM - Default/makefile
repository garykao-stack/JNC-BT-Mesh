################################################################################
# Automatically-generated file. Do not edit!
################################################################################

export WINEDEBUG=-all

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include user/bus_drv/subdir.mk
-include user/subdir.mk
-include protocol/bluetooth/lib/EFR32XG13X/IAR/subdir.mk
-include protocol/bluetooth/bt_mesh/src/subdir.mk
-include protocol/bluetooth/ble_stack/src/soc/subdir.mk
-include platform/service/sleeptimer/src/subdir.mk
-include platform/radio/rail_lib/plugin/coexistence/hal/efr32/subdir.mk
-include platform/radio/rail_lib/plugin/coexistence/common/subdir.mk
-include platform/middleware/glib/glib/subdir.mk
-include platform/middleware/glib/dmd/display/subdir.mk
-include platform/emlib/src/subdir.mk
-include platform/emdrv/uartdrv/src/subdir.mk
-include platform/emdrv/spidrv/src/subdir.mk
-include platform/emdrv/sleep/src/subdir.mk
-include platform/emdrv/nvm3/src/subdir.mk
-include platform/emdrv/gpiointerrupt/src/subdir.mk
-include platform/emdrv/dmadrv/src/subdir.mk
-include platform/Device/SiliconLabs/BGM13/Source/IAR/subdir.mk
-include platform/Device/SiliconLabs/BGM13/Source/subdir.mk
-include iar/Debug/Obj/subdir.mk
-include hardware/kit/common/drivers/subdir.mk
-include hardware/kit/common/bsp/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(MSA_UPPER_DEPS)),)
-include $(MSA_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S79_DEPS)),)
-include $(S79_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(MSA_DEPS)),)
-include $(MSA_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(S79_UPPER_DEPS)),)
-include $(S79_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: JNC-Modbus-Mesh.out

# Tool invocations
JNC-Modbus-Mesh.out: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: IAR Linker for ARM'
	ilinkarm $(OBJS) $(USER_OBJS) -o "JNC-Modbus-Mesh.out" --config "D:\Work\Silicon-BLE\BLE-Mesh\JNC-Modbus-Mesh\bgm13p32f512ga.icf" --config_def __STACK_SIZE=0x1000 --config_def __HEAP_SIZE=0x1700 --no_wrap_diagnostics "D:\Work\Silicon-BLE\BLE-Mesh\JNC-Modbus-Mesh\platform\radio\rail_lib\autogen\librail_release\librail_config_bgm13p32f512ga_iar.a" "D:\Work\Silicon-BLE\BLE-Mesh\JNC-Modbus-Mesh\platform\emdrv\nvm3\lib\libnvm3_CM4_iar.a" "D:\Work\Silicon-BLE\BLE-Mesh\JNC-Modbus-Mesh\protocol\bluetooth\lib\EFR32XG13X\IAR\libbluetooth_mesh.a" "D:\Work\Silicon-BLE\BLE-Mesh\JNC-Modbus-Mesh\platform\radio\rail_lib\autogen\librail_release\librail_module_efr32xg13_iar_release.a" "D:\Work\Silicon-BLE\BLE-Mesh\JNC-Modbus-Mesh\protocol\bluetooth\lib\EFR32XG13X\IAR\binapploader.o" --entry __iar_program_start --map "D:/Work/Silicon-BLE/BLE-Mesh/JNC-Modbus-Mesh/IAR ARM - Default/JNC-Modbus-Mesh.map" --no_range_reservations --silent --keep __iar_program_start  
	ielftool --srec --verbose "JNC-Modbus-Mesh.out" "JNC-Modbus-Mesh.s37"  
	ielftool --ihex --verbose "JNC-Modbus-Mesh.out" "JNC-Modbus-Mesh.hex"  
	ielftool --bin --verbose "JNC-Modbus-Mesh.out" "JNC-Modbus-Mesh.bin"
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(MSA_UPPER_DEPS)$(EXECUTABLES)$(OBJS)$(ASM_DEPS)$(S79_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(MSA_DEPS)$(C_DEPS)$(S79_UPPER_DEPS) JNC-Modbus-Mesh.out
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
