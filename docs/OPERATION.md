# ELS2 — Electronic Leadscrew for Sieg C3

NanoELS H5 firmware adapted for a Sieg C3 mini lathe with a web UI instead of a Nextion touchscreen.

Based on [kachurovskiy/nanoels](https://github.com/kachurovskiy/nanoels) (H5 version).

## Hardware

- **MCU**: ESP32-S3 DevKitC-1
- **Z driver**: CL57T V4.1 (closed-loop, S3 selector = 5V, DIP SW1-4 ON = 200 steps/rev)
- **Z motor**: NEMA 23 (23HS45-4204D-E1000, 3Nm, built-in 1000 PPR encoder)
- **Z leadscrew**: Stock 1.5mm pitch, GT2 belt 20T motor : 80T leadscrew (4:1 reduction)
- **X driver**: DM542 (1600 microsteps/rev, DIP SW3/7/8 ON)
- **X leadscrew**: Stock 1.26mm pitch (measured), motor 1:1 coupled to screw
- **Spindle encoder**: 600 PPR optical, AB quadrature
- **Display**: Web UI served to a tablet (no Nextion)
- **Input**: Web UI + physical button on GPIO 12
- **OLED**: SH1106 2.42" I2C on GPIO 9/10 — shows RPM and X stepper status (FREE/LOCK)
- **Power**: 48V PSU → CL57T, ESP32 powered from CL57T P2 encoder supply

See [WIRING.md](WIRING.md) for the wiring diagram and the [README](../README.md) for operating procedures.

## GPIO Pin Map

| Function | GPIO |
|---|---|
| Spindle Encoder A | 17 |
| Spindle Encoder B | 16 |
| Z STEP | 4 |
| Z DIR | 5 |
| Z ENA | 6 |
| X STEP | 7 |
| X DIR | 8 |
| X ENA | 13 |
| Physical Button | 12 |
| OLED SCL | 9 |
| OLED SDA | 10 |

## Firmware Configuration

| Parameter | Z axis | X axis |
|---|---|---|
| Screw pitch | 1.5mm (`SCREW_Z_DU=15000`) | 1.26mm (`SCREW_X_DU=12600`) |
| Motor steps | 800 (`MOTOR_STEPS_Z=800`) | 1600 (`MOTOR_STEPS_X=1600`) |
| Backlash | 0.52mm (`BACKLASH_DU_Z=5194`) | 0.10mm (`BACKLASH_DU_X=1000`) |
| Invert dir | false | true |
| Invert ENA | true (active low) | true (active low) |
| Speed (start/accel/max) | 1x / 25x / 8x | 1/4x / 6x / 2x |

X axis speeds are conservative to avoid step skipping on the stock screw.
On boot, X starts disabled with its ENA output explicitly driven inactive (HIGH),
releasing the cross-slide for handwheel use. Use the web UI's X enable toggle to lock it.

## Building

Requires `arduino-cli` with the ESP32 core, `WebSockets`, and `U8g2` libraries.
Create `els2/els_wifi_config.h` with a quoted-string definition of `ELS_AP_PASSWORD`
using your chosen 8–63 character password. This private file is ignored by Git.

```bash
# One-time setup
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
arduino-cli lib install "WebSockets"
arduino-cli lib install "U8g2"

# Compile
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M \
  --export-binaries els2/
```

The compiled binary lands at `els2/build/esp32.esp32.esp32s3/els2.ino.bin`.

## Flashing

### First flash (USB)

```bash
arduino-cli upload \
  --fqbn esp32:esp32:esp32s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M \
  -p /dev/ttyACM0 els2/
```

### OTA (over the ELS WiFi access point)

Once the firmware is running, connect to WiFi network **ELS** using the password supplied at build time.
Future updates then go via `POST /update`:

```bash
curl -X POST -F "update=@els2/build/esp32.esp32.esp32s3/els2.ino.bin" http://192.168.100.1/update
```

Or tap the **OTA** button in the web UI and upload the `.bin` from the browser.

The ESP32 reboots automatically after a successful flash. OTA is blocked while the machine is running (`isOn == true`).

## Web UI

Connect the phone or tablet to WiFi network **ELS** using the password supplied at build time, then open
`http://192.168.100.1` in its browser. The ESP32 runs as a standalone access point, so no
existing WiFi network or internet connection is required.
Clients receive an address automatically on `192.168.100.0/24`. The controller does not
join the home WiFi network. WiFi passwords must contain 8 to 63 characters.

- **`/`** — Main control UI (jog, numpad, mode select, stops, axis positions)
- **`/gcode`** — GCode file management (upload, list, delete stored programs)
- **`/status`** — Read-only diagnostics, including X disabled state and ENA GPIO level
- **`/update`** — OTA firmware upload endpoint

The UI communicates over WebSocket (port 81). Buttons send keycodes (`=N\n`), the firmware broadcasts JSON status at 10Hz. Jog buttons support hold-to-move via press/release keycodes (`=-N\n` for release).

### Stop Buttons

Stop buttons define soft limits for automated operations. They are labeled by function:

| Button | Sets | Meaning |
|---|---|---|
| **STOP Z Chuck** | Z left stop | Limit toward chuck |
| **STOP Z Tail** | Z right stop | Limit toward tailstock |
| **STOP X Depth** | X up stop | Depth limit (into workpiece) |
| **STOP X Start** | X down stop | Start position (OD surface) |

Stop buttons turn **green** when set. To clear a stop: jog to the stop position and press the same button again, or press **RST** to clear all.

### RST Button

Clears all stops, positions, and settings. Sends the `R` command over WebSocket. Equivalent to holding OFF for 3 seconds. Requires confirmation dialog.

### OTA Button

Opens the firmware update dialog. Select a compiled `.bin` file and upload. Machine must be stopped.

### Mode Selector

Tap **MODE** to open the mode selection overlay. Tap a mode to switch. Modes can also be cycled by pressing MODE repeatedly.

## Physical Button (GPIO 12)

The physical button is disabled in the supplied firmware because it interfered
with X enable. Use the web UI's X enable toggle and OFF control instead.
X status shows on the OLED as **X FREE** or **X LOCK**.

## OLED Display

The SH1106 2.42" OLED on GPIO 9/10 shows:
- **RPM** in large digits (42px font, right-aligned)
- **X FREE** or **X LOCK** in the top-right corner — indicates whether the X stepper is enabled or disabled

Updates every 500ms, non-blocking, runs on core 0 at lowest priority.

## Threading

Follow the [threading procedure in the README](../README.md#cutting-an-external-thread).
Select total radial depth and stock diameter from the required thread specification
and tooling, and verify fit by measurement.

## Modes

All NanoELS H5 modes are supported:

| Mode | Description |
|---|---|
| Gearbox | Electronic gearbox — Z follows spindle at set pitch |
| Turn | Multi-pass turning (Z main, X aux) |
| Face | Multi-pass facing (X main, Z aux) |
| Cone | Cone cutting (constant X/Z ratio) |
| Cut | Parting / cutoff |
| Thread | Multi-pass threading with progressive depth |
| Ellipse | Ellipse cutting |
| Async | Independent axis control (not spindle-synced) |
| GCode | Execute uploaded GCode programs |

## Disabling Steppers for Manual Handwheel Use

Via the web UI:
1. Tap **Z⏻** or **X⏻** to release the stepper
2. Use the handwheel
3. Tap **Z⏻** or **X⏻** again to re-lock
4. Tap **Z=0** or **X=0** to re-establish zero at the new position

The physical button is disabled in this firmware; use the web controls above.

## Changes from NanoELS H5

### Removed
- Nextion 5" touchscreen (Serial1 on GPIO 43/44)
- PS2 keyboard (GPIO 36/37)
- SN74HCT245N level shifters (CL57T S3=5V, direct GPIO drive)
- Custom JLCPCB PCB (direct wiring to DevKitC-1)

### Added
- Web UI on tablet via WiFi WebSocket
- Web OTA firmware update (`POST /update`)
- Physical button on GPIO 12 (X stepper toggle / abort)
- SH1106 OLED RPM + X status display
- JSON status broadcast at 10Hz over WebSocket
- Jog button release support (`=-N\n` negative keycodes)
- WebSocket `R` command for full reset
- Stop buttons labeled by function (STOP Z Chuck/Tail, STOP X Depth/Start)
- Stop buttons turn green when set
- Non-ASCII character stripping for WebSocket UTF-8 safety
- JSON escaping on user-controlled display strings
- Auto-reconnect WebSocket with DISCONNECTED banner
- Jog release on WebSocket reconnect (safety)

### Changed
- Encoder PPR: 1200 → 600
- Z leadscrew: 4mm ball screw → 1.5mm stock
- X leadscrew: 4mm ball screw → 1.26mm stock (measured)
- X microsteps: 800 → 1600 (DM542 DIP SW3/7/8 ON)
- ENA pins: active high → active low (INVERT_Z_ENABLE, INVERT_X_ENABLE = true)
- Preferences namespace: `h5` → `els2` (version 2, clean start)

### Unchanged
- All motion control logic (threading, turning, facing, cone, cut, ellipse, async, GCode)
- Spindle encoder PCNT hardware pulse counting
- Trapezoidal acceleration profiles
- Backlash compensation
- Preferences save/restore
- LittleFS GCode storage

## Calibration

Use a dial indicator to verify axis movement at STEP=0.1mm (low speed, no skipping). The calibration formula is:

```
new SCREW_DU = old SCREW_DU × commanded_mm / actual_mm
```

Procedure:
1. Mount dial indicator against the axis
2. Set STEP to 0.1mm
3. Jog one direction a few times to establish direction
4. Zero the dial
5. Tap jog 10 times same direction (= 1mm commanded)
6. Read dial — that's the actual movement
7. Apply the formula above

Backlash is measured by reversing direction and noting the dead zone before the dial moves.

## Upstream Reference

The original project is [NanoELS H5](https://github.com/kachurovskiy/nanoels/tree/main/h5).
This repository contains the adapted firmware; current upstream features may differ.
