# Wiring Diagram — Electronic Leadscrew

Complete wiring for ESP32-S3 DevKitC-1 + CL57T + 600 PPR spindle encoder + NEMA 23 motor encoder + optional cross-slide stepper.

---

## GPIO Assignments

| GPIO | Function | Direction | Notes |
|------|----------|-----------|-------|
| 4 | Z-axis STEP | Output | Direct to CL57T PUL+ (S3 set to 5V) |
| 5 | Z-axis DIR | Output | Direct to CL57T DIR+ (S3 set to 5V) |
| 6 | Z-axis ENA | Output | Direct to CL57T ENA+ (active low) |
| 7 | X-axis STEP | Output | Cross-slide stepper driver STEP |
| 8 | X-axis DIR | Output | Cross-slide stepper driver DIR |
| 13 | X-axis ENA | Output | Cross-slide stepper driver ENA (active low) |
| 9 | OLED SCL | Output | I2C clock, SH1106 2.42" display |
| 10 | OLED SDA | I/O | I2C data, SH1106 2.42" display |
| 12 | Button | Input | Internal pull-up, active low to GND |
| 16 | Spindle Encoder B | Input | Internal pull-up, 5V signal (swapped A/B for correct direction) |
| 17 | Spindle Encoder A | Input | Internal pull-up, 5V signal (swapped A/B for correct direction) |
| 18 | Motor Encoder EB+ | Input | Internal pull-up, tapped from DB15 |
| 21 | Motor Encoder EA+ | Input | Internal pull-up, tapped from DB15 |

**Reserved for future MCP4131 spindle speed control:** GPIO 11 (SDI/SDO), 14 (CS), 15 (SCK).

**Avoid:** GPIO 0/45 (strapping), 19/20 (USB), 26-32 (internal flash/PSRAM), 43/44 (UART0 serial).

**ENA on GPIO 6** — Active low. Z restores its saved enable state on boot (enabled if unset). Disable via the web UI button. X on GPIO 13 always starts disabled.

---

## Full Wiring

```
                            +---------------------------------------------+
                            |         ESP32-S3 DevKitC-1                   |
                            |                                             |
                            |  GPIO 16 <-- Spindle Encoder B (White)      |
                            |  GPIO 17 <-- Spindle Encoder A (Green)      |
                            |                                             |
                            |  GPIO 18 <-- Motor Encoder EB+ (tap)        |
                            |  GPIO 21 <-- Motor Encoder EA+ (tap)        |
                            |                                             |
                            |  GPIO 4  --> CL57T PUL+ (STEP)              |
                            |  GPIO 5  --> CL57T DIR+ (DIR)               |
                            |  GPIO 6  --> CL57T ENA+ (ENABLE)            |
                            |                                             |
                            |  GPIO 7  --> X-axis STEP (cross-slide)      |
                            |  GPIO 8  --> X-axis DIR  (cross-slide)      |
                            |  GPIO 13 --> X-axis ENA  (cross-slide)      |
                            |                                             |
                            |  GPIO 12 <-- Button (pull-up, to GND)       |
                            |                                             |
                            |  GPIO 9  --> OLED SCL                       |
                            |  GPIO 10 <-> OLED SDA                       |
                            |                                             |
                            |  5V <----- CL57T P2 VCC (encoder 5V out)    |
                            |  GND <---- CL57T P2 EGND                    |
                            +---------------------------------------------+


+------------------------+                              +------------------------+
|  Spindle Encoder       |                              |       CL57T            |
|  600 PPR Optical       |                              |   Stepper Driver       |
|                        |                              |                        |
|  White (A) --> GPIO 17 |                              |  PUL+ <-- GPIO 4      |
|  Green (B) --> GPIO 16 |                              |  PUL- <-- GND         |
|  Red (VCC) --> 5V      |                              |  DIR+ <-- GPIO 5      |
|  Black (GND) -> GND    |                              |  DIR- <-- GND         |
|  Shield -----> GND     |                              |  ENA+ <-- GPIO 6       |
                                                        |  ENA- <-- GND          |
+------------------------+                              |                        |
                                                        |  S3 selector: 5V !!    |
                                                        |                        |
                                                        |  DC+ <-- 48V PSU (+)   |
                                                        |  DC- <-- 48V PSU (-)   |
                                                        |                        |
                                                        |  P2 ENCODER CONNECTOR  |
+----------------------------------------------+        |  VCC  --> ESP32-S3 5V  |
|  NEMA 23 Motor - DB15 Encoder Connector      |        |  EGND --> ESP32-S3 GND |
|  23HS45-4204D-E1000 (1000 PPR differential)  |        +------------------------+
|                                               |
|  Pin 1  (BLK) EA+ --+-> CL57T encoder A+     |
|                      +-> ESP32-S3 GPIO 21     |
|  Pin 13 (BLU) EA- ----> CL57T encoder A-     |
|  Pin 11 (YEL) EB+ --+-> CL57T encoder B+     |
|                      +-> ESP32-S3 GPIO 18     |
|  Pin 12 (GRN) EB- ----> CL57T encoder B-     |
|  Pin 2  (RED) VCC ----> CL57T P2 VCC         |
|  Pin 3  (WHT) GND ----> CL57T P2 EGND        |
+----------------------------------------------+

NEMA 23 Motor Coils (to CL57T P3):
  Black  --> A+
  Green  --> A-
  Red    --> B+
  Blue   --> B-


POWER
-----
48V PSU (+) ----> CL57T DC+
48V PSU (-) ----> CL57T DC-
CL57T P2 VCC --> ESP32-S3 5V pin (powers ESP32-S3 from driver)
CL57T P2 EGND -> ESP32-S3 GND (common ground for everything)


BUTTON (guided threading advance / abort)
------------------------------------------
ESP32-S3 GPIO 12 ---> Button ---> GND
(The physical button is disabled in the supplied firmware; use the web controls.)


CROSS-SLIDE STEPPER (optional, GPIO 7/8/13 to DM542 or similar)
-----------------------------------------------------------------
ESP32-S3 GPIO 7  --> Driver PUL+ (STEP)
ESP32-S3 GPIO 8  --> Driver DIR+ (DIR)
ESP32-S3 GPIO 13 --> Driver ENA+ (ENABLE, active low)
Driver PUL-/DIR-/ENA- --> GND
Driver power: 24-48V PSU
Motor: NEMA 23 (1.2Nm+ recommended for cross-slide)


OLED DISPLAY (SH1106 2.42" I2C, 128x64)
-------------------------------------------
OLED VDD ------> ESP32-S3 3.3V
OLED GND ------> ESP32-S3 GND
OLED SCK ------> ESP32-S3 GPIO 9
OLED SDA ------> ESP32-S3 GPIO 10


BELT DRIVES
-----------
Spindle shaft --- GT2 belt (1:1 matching pulleys) --- Spindle Encoder (600 PPR)
Motor shaft ----- GT2 belt (20T motor : 80T leadscrew) --- Leadscrew
```

---

## Notes

1. **No level shifter needed** — ESP32-S3 3.3V GPIO drives the CL57T directly when the S3 selector is set to 5V. The CL57T's optocouplers trigger reliably at 3.3V in this mode.
2. **CL57T S3 selector must be set to 5V** — factory default is 24V. Without this, the 3.3V GPIO signals won't trigger the optocouplers.
3. **PUL-/DIR- must be connected to GND** — the optocouplers need a return path. Signal goes to PUL+/DIR+, ground goes to the minus pins.
4. **ENA on GPIO 6** — Active low. Z restores its saved enable state; X starts disabled. Disable via the web UI button to release holding torque.
5. **ESP32-S3 is powered from CL57T P2 connector** — the VCC pin on P2 provides 5V (encoder supply), EGND is the power ground. This eliminates the need for a separate 5V supply and ensures common ground between ESP32-S3, CL57T, and motor encoder.
6. **Motor encoder EA+/EB+ are tapped** — one wire to CL57T for closed-loop, one to ESP32-S3 for carriage position. EA-/EB- only go to CL57T (it needs differential, ESP32-S3 doesn't).
7. **Use shielded cable** for the spindle encoder — solder shield braid to GND.
8. **Encoder signal levels** — check encoder output levels against the ESP32-S3 input ratings and provide level conversion where required; do not assume 5V tolerance. Keep encoder belt ratio 1:1 to avoid speed multiplication.
