# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Bluetooth Mesh sensor firmware for the **Silicon Labs BGM13P32F512GA** SoC, internally named
**JNC-BT-Mesh** (the "Skynet" 藍芽網路感測及接受系統 / wireless sensor network). Built on the
Silicon Labs Bluetooth Mesh SDK (Gecko stack, derived from the "BT Mesh Sensor Server" example).

A single firmware image runs in multiple **node roles** (server / client / setup) and supports many
sensor types selected at runtime, plus a Modbus-over-RS485 interface. The team works and documents
in **Traditional Chinese** — preserve that in comments and `CHANGELOG.md`.

## Build / Flash / Verify

There is **no command-line build or automated test suite** — this is IAR/Simplicity Studio embedded firmware.

- **IDE / Toolchain:** Simplicity Studio 4 + IAR ARM compiler (8.3+). Import via `JNC-BT-Mesh.sls`
  (see `README.md` for the full setup walkthrough).
- **Build (GUI):** Simplicity Studio menu → `Project > Build Project`. Output `.bin` lands in the
  `IAR ARM - Default/` (or `IAR ARM - Default - BGM13P32F512GA/`) folder. These build-output folders
  (`*.o`, `*.d`, generated `.bin`/`.gbl`, `output_gbl/`) are artifacts — do not hand-edit them.
- **Build (command line, verified working):** the GUI build is just GNU make over the
  CDT-generated `IAR ARM - Default/Makefile`. To run it headless, put the bundled msys tools and the
  IAR 8.3 compiler on `PATH`, then `make all` in that folder:
  ```cmd
  set "PATH=C:\SiliconLabs\SimplicityStudio\v5_4\support\common\build\msys\1.0\bin;C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.3\arm\bin;C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.3\common\bin;%PATH%"
  cd /d "<repo>\IAR ARM - Default"
  make clean && make all
  ```
  This compiles with `--cpu Cortex-M4 --fpu VFPv4_sp`, links via `bgm13p32f512ga.icf`, and emits
  `JNC-BT-Mesh_3_2.{out,bin,hex,s37}`. Adjust the `v5_4` segment to the installed Studio version.
- **Do NOT build the `iar/iar.ewp` project with `IarBuild.exe`** — it is leftover Silicon Labs SDK
  example scaffolding whose `CoreVariant` is unset (defaults to ARM7TDMI → `cmsis_iccarm.h` "Unknown
  target"), and its include paths omit `user/`. It is not the real build. Use the makefile above.
- **Flash:** Simplicity Commander (`commander`) → connect Adapter + Target (`BGM13P32F512GA`) →
  Flash tab → select `JNC-BT-Mesh.bin` → Flash.
- **Bootloader / OTA images:** `create_bl_files.bat` produces GBL files (used for OTA; OTA is enabled
  by the `JNC_OTA` define in `main.c`, advertised device name `"JNC-OTA"`).
- **Verification is on-hardware:** observe behavior via serial `printf`/`dprint()` debug output over
  VCOM/UART (see Debug output below), and by reading device state over Modbus. There are no unit tests to run.

## Debug output (serial) — verified

Firmware `printf` / `dprint()` is retargeted to **USART0 (VCOM)** by `RETARGET_SerialInit()`
(`user/global.c`, active because `DEBUG_PORT == UART_PORT`).

- **Port / pins:** USART0 — **TX = `PA0`, RX = `PA1`** (location 0). Selected by `#define VCOM_USART0`
  in `hardware/kit/BGM13_BRD4306B/config/hal-config-board.h` (`BSP_SERIAL_APP_PORT`).
- **Line settings:** **115200 8N1** (baud hardcoded in `hardware/kit/common/drivers/retargetserial.c`).
- **NOT the same as the sensor bus:** the `baudrate:9600` shown in the boot banner is the
  **RS485/Modbus sensor bus on USART2 (CMD)** — pins PF4/PF5 loc17, baud from `pMeshNodeData->BaudRate`
  (`UsartInit()` in `user/bus_drv/bus_usart.c`), a *different* UART. `dprint`/`Printf`/`Trace` verbosity
  is gated by `DPRINT` / `DEBUG_PRINT` in `user/global.h`.
- **How to read it:** connect the debug TX (PA0) to a USB-UART adapter and open the COM port at
  115200, or run `scripts/serial_monitor.ps1` (defaults to `COM6` @ 115200), or the VSCode task
  **"序列埠監看 (Serial Monitor COM6)"**. Verified 2026-06-22 on COM6: clean boot banner
  (`BTM001: Firmware Version ==> v1.41`, MAC/ID, `[WATCHDOG] Initialized`, `Booting Seconds` ticks).

## Release Conventions

- `CHANGELOG.md` is the authoritative history. Each release is tagged like `ALL DEVICE v1.41` with a
  date and author initial (e.g. 明 / 珊 / 曾), newest at top.
- `FW_VER` in `user/global.h` is the firmware version as an integer (e.g. `141` = v1.41) and **must be
  bumped to match** any new `CHANGELOG.md` entry.

## Architecture (big picture)

**Boot → super-loop.** `main.c` runs `initMcu()/initBoard()/initApp()`, fills out the
`gecko_configuration_t` (BLE + Mesh stack heap, advertiser sets, PA, OTA name), then calls
`appMain()` in `app.c`.

`appMain()` (`app.c`) is the heart:
1. `BleMeshNodeInit()` — `gecko_stack_init`, registers BGAPI classes (`gecko_bgapi_classes_init`),
   then initializes peripherals: serial, display, LEDs, buttons, DI/DO, sensors, RS485/USART buses,
   and the **watchdog** (8 s timeout). Registers per-role sensor client/server BGAPI classes based on
   `NodeRole`, then `BleEventInit()` / `MeshEventInit()`.
2. `while(1)` loop: `gecko_wait_event()` → dispatch event → `watchdog_feed()` → run the role task.

**Event dispatch is table-driven.** `BleMeshEventProc()` walks `BleEventFun[]` then `MeshEventFun[]`
(arrays of `{EventID, handler}`), matching the incoming BGAPI event ID; unmatched events fall through
to `EventIDtoStringProc()`. To handle a new stack event, add an entry to the appropriate table
(`user/ble_event.c` / `user/mesh_event.c`) rather than editing the dispatch loop.

**Node roles** (`NodeRole`, set from persistent config): `NR_SERVER` (sensor node — reads sensors,
sleeps, responds to the client), `NR_CLIENT` (aggregator — polls servers over mesh), `NR_SETUP` /
`NR_SETUP_SERVER` (Windows-utility configuration mode). The main loop branches into
`ServerNodeTask()` / `ClientNodeTask()` / `BtMeshSetupTask()` accordingly. Server power management
(deep sleep, LPN/proxy/beacon toggling) lives in `user/Mesh_Node.c` (`SetSleeping`, `SetSleepingTimer`).

### Where the custom code lives

`user/` is the JNC application layer (the part you'll usually edit). `hardware/`, `platform/`, and
`protocol/` are Silicon Labs SDK / CMSIS / board support — generally treat as vendor code, don't modify.

Key `user/` modules:
- **`global.h`** — central config header included almost everywhere (see Configuration below).
- **`Mesh_Node.c` / `.h`** — node state machine: stages, timers, sleep/power management, role init.
- **`Mesh_Client.c`, `Mesh_Server.c`** — per-role task loops.
- **`node_data.c` / `.h`** — persistent configuration in flash (`pMeshNodeData`): element address,
  `MeshNodeID` (BT station ID), baud rate, `SensorClass`, reboot timers, sensor gain/offset.
  Magic id `NODE_DATA_ID = 0xA5A5`.
- **Modbus:** `mod_bus.c`, `ClientModbus.c`, `modbus_to_mesh.c`. Device exposes registers over RS485 —
  e.g. FC2 for DI status, FC6 register `0xF000` toggles maintenance mode, `0x900` reads `BootingSeconds`.
- **Buses:** `bus_drv/` — `device_bus`, `bus_usart`, `bus_rs485`, `bus_i2c`, `bus_spi`.
- **Sensor drivers (selected by `SensorClass`):** `sensor_dev.c`, `people_count_sensor`, `PMSA003`
  (PM2.5), `SGPxx` (TVOC), `SHT3x`/`si7013` (temp/RH), `BQ3200`, `AD7147`, `DAC7760`, plus PZEM/DC600/
  FTM94 handlers. `SensorClass` string list is in `app.c` (`SensorClassStr[]`): `NO Sensor`, `Auto Scan`,
  `PZEM`, `Visual Sensor`, `DC600`, `FTM94`, `BTM-G6`, `BTM485`, `DI Mode`.
- **Comm glue:** `ble_comm`, `ble_event`, `mesh_event`, `cmd_to_bt_mesh`, `jnc_cmd`, `com_port`.
- **Feature modules:** `MeshFeatures`, `ivi_features`, `G6_BT_Mesh`, `A308_Server`,
  `RS485_Transmitter`, `water_level_mesh`.
- **`watchdog.c` / `.h`** — WDOG0 on ULFRCO (1 kHz), default 8 s timeout, fed once per main-loop
  iteration. Any long blocking operation in the loop risks an 8 s reboot — feed or split the work.

## Parameter storage (persistence) — verified

Runtime parameters are stored in the **BGM13 internal flash**, NOT in the on-board 24FC512 EEPROM.

- **NVM3 / "SIMEE" region** (linker block `nvm3` / section `SIMEE`, placed at the **end of the 512 KB
  main flash** by `bgm13p32f512ga.icf`) — the main persistent store, accessed via the Bluetooth stack
  Persistent Store API (`Cmd_flash_ps_save` / `Cmd_flash_ps_load` in `user/node_data.c`):
  - PS key `0x4000` `PS_KEY_MESH_NODE_DATA` → `_Mesh_Node_Data` (node ID, role, baud, `SensorClass`,
    element addr, key/index fields, `WorkingTimer`, reboot timers, `SegPPercent`…; magic `0xA5A5`).
  - PS key `0x4001` `PS_KEY_ADJUST_VALUE` → `_AdjustValue` (temp/RH gain & offset, RS485-transmitter / G6).
  - PS keys `0x4003`–`0x4006` → water-level calibration.
  - The Bluetooth **Mesh stack** also keeps its own data here: provisioning, NetKey/AppKey/DeviceKey,
    IV index, sequence number, model bindings/pub/sub.
- **User Data flash page** (`USERDATA_BASE = 0x0FE00000`, written via `em_msc` `MSC_WriteWord`):
  `CAL_DATA_ADDR` calibration + `CTUNE_UD_ADDR` (0x0FE00100) HFXO crystal tuning + backup.
- **Gecko Bootloader** lives in the bootloader region `0x0FE10000` (16 KB); AppLoader at flash start.
- **RAM only (not persisted):** `SensorData[96]` live sensor values (`WriteWlCalData`/`ReadWlCalData`
  are empty stubs).
- **24FC512 I2C EEPROM (I2C0 addr `0xA0`): present in code but UNUSED by current firmware.** The
  `_MeshNodeSysData` struct + `Eeprom*` helpers in `user/bus_drv/bus_i2c.c` are legacy — `EepromInit()`
  is never called and no application code reads/writes the EEPROM (superseded by the NVM3 PS-key scheme).
  `I2CInit()` only sets up I2C0 for the temp/RH sensor (SHT3x/Si7021) + battery ADC.

## Configuration (compile-time, in `user/global.h`)

- **Product select** (exactly one `#define`): `JNC_BT_MESH` (default) — others are `BT_MESH_G6`,
  `BTM_A308`, `BTM_TRANSMITTER`, `ULTRA_SOUND_SKYNET`. Each sets its own `FW_VER`, `MODEL_NAME`,
  `DEVICE_NAME`.
- **Temp/RH sensor:** `SensorIsSHT3x` (default) vs `SensorIsSi7021` — exactly one.
- **Debug output:** `DPRINT 1` enables `dprint()` (serial). `DEBUG_PRINT` gates the heavier
  `Print*`/`Trace` helpers.
- Target part is `BGM13P32F512GA`; linker config `bgm13p32f512ga.icf`.
