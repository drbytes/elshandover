# ELS — Electronic Leadscrew for the Sieg C3

ESP32-S3 firmware for spindle-synchronised feed, threading and powered cross-slide
movement on a Sieg C3 mini lathe. A browser provides the controls; an OLED shows
spindle RPM and the cross-slide lock state.

## Getting started

1. Power the controller and connect your phone, tablet or computer to **ELS** Wi-Fi.
2. Open **http://192.168.100.1**. Stay connected if your device reports no Internet.
3. Check that the panel is connected and ELS is **OFF** before setting up.
4. With the spindle stopped and the tool clear, check axis directions, motor enable
   states and available travel.

The spindle is controlled by the lathe's own switches. **ELS OFF does not stop the
spindle.** Keep the physical stop accessible: a disconnected browser does not prove
that the machine has stopped. Automatic positioning and return moves can happen
even with the spindle stopped. Test new setups clear of the work before cutting.

## Controls

Z moves the carriage along the bed. X moves the cross-slide. X readings and infeed
are radial: 0.10 mm inward movement reduces an outside diameter by about 0.20 mm.

| Control | Action |
|---|---|
| MODE | Select the operation |
| ON | Confirm a value, advance a setup prompt, or start the operation |
| OFF | Stop/abort the controller operation |
| Z arrows | Jog toward chuck/tailstock, except while selecting a setup direction |
| X arrows | Jog into/away from the work |
| STEP | Change jog step; the 1 mm setting is continuous while held |
| REV | Reverse the programmed feed/pitch sign |
| X / Z enable | Toggle holding torque |
| X=0 / Z=0 | Zero the displayed position |
| MM / IN / TPI | Change units |
| MS | Change thread starts; use one for ordinary single-start threads |
| RST | Clear positions, stops and operating settings |
| OTA | Upload firmware |

Use 0.1 mm or 0.01 mm jog steps for approach. Check physical directions before
setting stops. X starts released at boot; the OLED shows **X FREE** or **X LOCK**.
Z restores its saved enable state. The physical GPIO 12 button is disabled.

### Numeric entry and stops

In MM mode, enter distances and pitch in thousandths of a millimetre:
`100` = 0.100 mm, `800` = 0.800 mm, `1250` = 1.250 mm.
Outside a setup prompt, enter feed/pitch followed by **ON**; this confirms the
number without also starting motion. At the pass-count prompt, enter a whole number.

| Stop | Meaning |
|---|---|
| STOP Z Chuck | Chuck-side travel endpoint |
| STOP Z Tail | Tailstock-side travel endpoint |
| STOP X Depth | Inward/final-depth limit |
| STOP X Start | Outward/start limit |

With no number pending, jog to a position and tap its stop button. Green means set;
tapping a set stop clears it. Alternatively, type a distance then a stop button to
set a stop that distance from the current position, in the button's direction.
This is relative travel, not an absolute coordinate or a diameter.

Turn, Face and Thread require all four stops. Cut requires both X stops. These are
software limits, not physical limit switches. Check them after zeroing or resetting.

## Manual movement and zeroing

Set ELS OFF and stop the spindle. Release the required motor using its enable
button, then move the handwheel. Re-enable it before powered operation.
X has no position encoder, so manual movement is not tracked: establish zero again
at a known reference and check the stops. Touch off with small increments and the
spindle stopped, then retract before starting it. Measure actual diameters with a
micrometer or caliper; a display zero does not measure the stock.

## Straight turning

### Single pass: Gearbox

1. Select **Gearbox** and **MM**. Enter feed per spindle revolution and press ON.
   For example, `100` programs 0.10 mm/rev; choose feed for the tool and material.
2. Set the Z start/end stops clear of the chuck and shoulders. Set the X cutting
   position, lock X and position Z at the beginning of the cut.
3. Check spindle and feed directions with the tool clear. Correct the feed sign
   with REV while stopped if required.
4. Start the spindle using the lathe controls and press ON to engage feed.
5. At the end, press OFF, stop the spindle, retract X and return Z clear of the
   work. Measure before selecting the next infeed.

### Repeated passes: Turn

Select **Turn** and enter the feed per revolution. Set both Z stops for the cut
length, X Start at the stock surface and X Depth at the final radial position.
Enable both axes. Press ON, choose the pass count and confirm. At **External?**,
confirm outside turning; a Z arrow toggles External/Internal at this prompt.
Read the **Go …?** repositioning prompt, prove the cycle clear, start the spindle
and confirm. Turn divides the radial removal over the passes and returns automatically.
Allow for the configured 0.5 mm retract and the complete tool-holder envelope.

## Cutting an external thread

Use a tool matching the thread form, set on centre height and square to the work.
Prepare the stock diameter and runout for the required thread specification.
The following uses **Thread**, powered X infeed and a straight single-start thread.

1. With ELS OFF, select **Thread**, **MM**, and one thread start using MS.
2. Enter pitch and press ON: M5 × 0.8 uses `800`; M8 × 1.25 uses `1250`.
   Check the displayed value.
3. Set STOP Z Tail at the start with approach clearance and STOP Z Chuck at the
   end/runout, clear of the chuck and shoulders.
4. Touch the stationary outside surface, press X=0, and set STOP X Start there.
5. Set STOP X Depth to the planned total radial infeed. From the surface, type
   the infeed distance and press that stop button to define it without moving
   the tool into stationary material. Use the thread specification and insert
   geometry to choose depth; pitch alone does not establish the correct fit.
6. Retract clear, enable both axes, check all four stops and press ON for setup.
7. At **passes?**, enter the required number and confirm. Choose it for the tool
   and material; threading takes larger early cuts and smaller finishing cuts.
8. Confirm **External?**; use a Z arrow to change it if necessary.
9. At **Cone ratio …?**, enter `0` and confirm for a straight thread.
10. Review **Go …?** and prove the cycle clear of the work first. For the cutting
    run, start the spindle at a suitable low threading speed and press ON.
11. The controller synchronises each pass, cuts along Z, retracts X, returns and
    advances infeed. After the final pass, stop the spindle with the lathe control.
12. Check pitch and fit with an appropriate gauge with the spindle stopped.
    Base further infeed on measurement, not a universal depth table.

Thread mode retracts 0.5 mm beyond X Start between passes: check this clearance.
Do not move handwheels, reset, change zero or change pitch during the cycle.
Pressing ON while running can advance the pass sequence; do not repeatedly press
it because the screen seems slow. OFF aborts the controller operation.

## Facing

For manual facing, keep ELS OFF, establish the Z cutting depth, lock the carriage
and release X for handwheel feed. Re-establish X zero before powered operation.

For **Face**, enter X feed per revolution. Set X Start at the outer face and
X Depth at its inward endpoint. Set both Z stops to bound the axial stock to remove.
Enable both axes, press ON, select passes, and choose **Right to left?** or
**Left to right?** using a Z arrow. Review Go, prove the path, start the spindle
and confirm. X makes the cutting strokes; Z advances depth between passes.
Check tool-holder and centre clearance before running the cycle.

## Grooving and parting

Align a suitable tool on centre height, square to the work, with minimal overhang.
Position and lock Z at the groove. Select **Cut**, set X feed per revolution and
both X stops, and enable X. Press ON, choose peck passes, check the indicated
external/internal direction and review Go. Prove the path, start the spindle
and confirm. X advances to progressively deeper endpoints and returns to X Start.

Choose speed, feed, depth and lubrication for the tool and material. A groove ends
at its specified depth. For parting, arrange support and clearance for the separated
piece; do not catch it by hand while the spindle runs.

## Other modes

- **Cone:** coupled X/Z travel for tapers. Ratio is diameter change divided by
  axial length; `10000` enters 0.10000, for example 2 mm diameter change over 20 mm.
- **Async:** powered feed independent of spindle rotation.
- **Ellipse:** coordinated curved-profile cutting within programmed stops.
- **GCode:** stored programs using the implemented G-code subset, managed at
  **http://192.168.100.1/gcode**. Do not assume full CNC compatibility.

Prove advanced toolpaths clear of the work; these modes are not covered by the
step-by-step cutting procedures above.

## Install the development tools — Windows and macOS

Normal operation requires only a browser. To modify firmware, install **Arduino IDE 2**
from [Arduino](https://www.arduino.cc/en/software). On Windows run the installer.
On macOS choose Apple Silicon or Intel as appropriate, open the disk image and
drag the app into Applications. [Official installation guide](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE).
This workflow does not require Git, Python or Arduino CLI.

1. In IDE Preferences/Settings, add this Additional Boards Manager URL:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
2. In Boards Manager, install **esp32 by Espressif Systems**, version **3.3.11**.
   [ESP32 installation guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
3. In Library Manager install **WebSockets by Markus Sattler 2.7.2** and
   **U8g2 by Oliver Kraus 2.36.19** using their version selectors.
   [Library installation guide](https://support.arduino.cc/hc/en-us/articles/5145457742236-Install-libraries-in-the-Arduino-IDE).
4. Download this repository using **Code → Download ZIP**, extract it and open
   `els2/els2.ino`. Keep it inside the `els2` folder. Complete the
   [local configuration step](docs/BUILDING.md#local-configuration).

## Compile and update firmware

Choose **Tools → Board → esp32 → ESP32S3 Dev Module**:

| Setting | Value |
|---|---|
| Flash Size | 8MB (64Mb) |
| Partition Scheme | 8M with spiffs (3MB APP/1.5MB SPIFFS) |
| USB CDC On Boot | Enabled |
| USB Mode | Hardware CDC and JTAG |
| Upload Mode | UART0 / Hardware CDC |

Leave other options at this board's defaults. Use this partition scheme even
though the application uses LittleFS. Click **Verify** to compile.

### Wi-Fi update

Select **Sketch → Export Compiled Binary** and find
`els2/build/esp32.esp32.esp32s3/els2.ino.bin`.
Stop the spindle and set ELS OFF. Connect with a strong signal, open the panel,
click OTA and upload **els2.ino.bin**, not the merged, bootloader or partition file.
Wait for completion and reboot, reconnect and reload. A dropped connection alone
does not confirm success. [Arduino binary export reference](https://docs.arduino.cc/arduino-cli/platform-specification/#recipes-to-export-compiled-binary).

### USB upload and recovery

Connect a USB data cable to the ESP32-S3 native USB connector. Select **Tools → Port**:
usually `COM…` on Windows or `/dev/cu.usbmodem…` on macOS, then click **Upload**.
If detection fails, hold BOOT, press and release RESET/EN, release BOOT and select
the newly appearing port. A reset may be needed after the first upload.
[Espressif USB flashing guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/cdc_dfu_flash.html).
If using a separate USB-to-UART connector, identify its chip and install the
manufacturer's driver if required; native USB and UART settings can differ.

## Troubleshooting and references

- **X locked:** stop the spindle, set ELS OFF, toggle X enable and check X FREE.
- **X will not move:** enable X and check X LOCK before powered operation.
- **ON does not start:** read the prompt; check stops, feed, pass count and axis enables.
- **DISCONNECTED:** check the Wi-Fi connection and distance. Use the physical stop
  when required rather than relying on a disconnected screen.
- **Build error:** check the local configuration and installed board/library versions.
- **OTA failure:** improve signal or use USB recovery.
- **Diagnostics:** `/status` reports X.disabled and X.enableLevel; both are 1 for
  the firmware's configured release state.

See [build configuration](docs/BUILDING.md), [wiring](docs/WIRING.md),
[machine configuration and calibration](docs/OPERATION.md), and
[firmware source](els2/els2.ino).

Based on [NanoELS H5](https://github.com/kachurovskiy/nanoels/tree/main/h5), adapted
for this lathe and browser interface. Upstream features can differ from this version.
