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
  VCOM/UART, and by reading device state over Modbus. There are no unit tests to run.

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

## Configuration (compile-time, in `user/global.h`)

- **Product select** (exactly one `#define`): `JNC_BT_MESH` (default) — others are `BT_MESH_G6`,
  `BTM_A308`, `BTM_TRANSMITTER`, `ULTRA_SOUND_SKYNET`. Each sets its own `FW_VER`, `MODEL_NAME`,
  `DEVICE_NAME`.
- **Temp/RH sensor:** `SensorIsSHT3x` (default) vs `SensorIsSi7021` — exactly one.
- **Debug output:** `DPRINT 1` enables `dprint()` (serial). `DEBUG_PRINT` gates the heavier
  `Print*`/`Trace` helpers.
- Target part is `BGM13P32F512GA`; linker config `bgm13p32f512ga.icf`.
