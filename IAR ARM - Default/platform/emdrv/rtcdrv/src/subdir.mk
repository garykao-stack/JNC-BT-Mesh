################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/emdrv/rtcdrv/src/rtcdriver.c 

OBJS += \
./platform/emdrv/rtcdrv/src/rtcdriver.o 

C_DEPS += \
./platform/emdrv/rtcdrv/src/rtcdriver.d 


# Each subdirectory must supply rules for building sources it contributes
platform/emdrv/rtcdrv/src/rtcdriver.o: ../platform/emdrv/rtcdrv/src/rtcdriver.c
	@echo 'Building file: $<'
	@echo 'Invoking: IAR C/C++ Compiler for ARM'
	iccarm "$<" -o "$@" --silent --no_wrap_diagnostics -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\CMSIS\Include" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\Device\SiliconLabs\BGM13\Include" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\middleware\glib\dmd\ssd2119" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\common\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\protocol\bluetooth\bt_mesh\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\protocol\bluetooth\bt_mesh\inc\common" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emlib\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\protocol\bluetooth\bt_mesh\src" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\middleware\glib\glib" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\middleware\glib" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\hardware\kit\common\bsp" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\halconfig\inc\hal-config" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\hardware\kit\common\drivers" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\radio\rail_lib\common" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\sleep\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\gpiointerrupt\src" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emlib\src" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\hardware\module\config" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\hardware\kit\common\halconfig" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\gpiointerrupt\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\bootloader\api" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\radio\rail_lib\protocol\ieee802154" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\protocol\bluetooth\bt_mesh\inc\soc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\hardware\kit\BGM13_BRD4306B\config" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\uartdrv\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\middleware\glib\dmd" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\Device\SiliconLabs\BGM13\Source" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\radio\rail_lib\protocol\ble" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\sleep\src" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\radio\rail_lib\chip\efr32\efr32xg1x" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\Device\SiliconLabs\BGM13\Source\IAR" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\middleware\glib\dmd\display" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\iar" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\user" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\user\bus_drv" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\dmadrv\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\spidrv\config" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\spidrv\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\rtcdrv\inc" -I"D:\Work\Silicon-BLE\BLE-Mesh\MeshSdk152-Client-Server\platform\emdrv\rtcdrv\config" -e --cpu Cortex-M4 --fpu VFPv4_sp --debug --endian little --cpu_mode thumb -On --no_cse --no_unroll --no_inline --no_code_motion --no_tbaa --no_clustering --no_scheduling '-DMESH_LIB_NATIVE=1' '-DHAL_CONFIG=1' '-DBGM13P32F512GA=1' --diag_suppress pa050 --diag_error pe223 --no_path_in_file_macros --no_dwarf4 --dependencies=m platform/emdrv/rtcdrv/src/rtcdriver.d
	@echo 'Finished building: $<'
	@echo ' '


