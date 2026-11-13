// ELS2 — NanoELS H5 adapted for Sieg C3 with web UI
// Based on https://github.com/kachurovskiy/nanoels

/* Change values in this section to suit your hardware. */

// Define your hardware parameters here.
const int ENCODER_PPR = 600; // 600 PPR spindle optical rotary encoder. Fractional values not supported.
const int ENCODER_BACKLASH = 3; // Numer of impulses encoder can issue without movement of the spindle

// Spindle rotary encoder pins. Swap values if the rotation direction is wrong.
#define ENC_A 17
#define ENC_B 16

// Main lead screw (Z) parameters.
const long SCREW_Z_DU = 15000; // 1.5mm Sieg C3 stock leadscrew
const long MOTOR_STEPS_Z = 800;
const long SPEED_START_Z = MOTOR_STEPS_Z; // Initial speed of a motor, steps / second.
const long ACCELERATION_Z = 25 * MOTOR_STEPS_Z; // Acceleration of a motor, steps / second ^ 2.
const long SPEED_MANUAL_MOVE_Z = 8 * MOTOR_STEPS_Z; // Maximum speed of a motor during manual move, steps / second.
const bool INVERT_Z = false; // change (true/false) if the carriage moves e.g. "left" when you press "right".
const bool INVERT_Z_ENABLE = true; // CL57T ENA is active low
const bool NEEDS_REST_Z = false; // Set to false for closed-loop drivers, true for open-loop.
const long MAX_TRAVEL_MM_Z = 300; // Lathe bed doesn't allow to travel more than this in one go, 30cm / ~1 foot
const long BACKLASH_DU_Z = 5194; // 0.5194mm backlash measured on Sieg C3
const char NAME_Z = 'Z'; // Text shown on screen before axis position value, GCode axis name

// Cross-slide lead screw (X) parameters.
const long SCREW_X_DU = 12600; // 1.26mm measured cross-slide screw pitch
const long MOTOR_STEPS_X = 1600;
const long SPEED_START_X = MOTOR_STEPS_X / 4; // Initial speed, conservative for stock screw
const long ACCELERATION_X = 6 * MOTOR_STEPS_X; // Acceleration of a motor, steps / second ^ 2.
const long SPEED_MANUAL_MOVE_X = 2 * MOTOR_STEPS_X; // Max jog speed, limited to avoid skipping on stock screw
const bool INVERT_X = true; // X↑ = into workpiece (engage), X↓ = away (retract)
const bool INVERT_X_ENABLE = true; // cross-slide driver ENA is active low
const bool NEEDS_REST_X = false; // Set to false for all kinds of drivers or X will be unlocked when not moving.
const long MAX_TRAVEL_MM_X = 100; // Cross slide doesn't allow to travel more than this in one go, 10cm
const long BACKLASH_DU_X = 1000; // 0.10mm backlash measured on Sieg C3 cross-slide
const char NAME_X = 'X'; // Text shown on screen before axis position value, GCode axis name

// Manual stepping with left/right/up/down buttons. Only used when step isn't default continuous (1mm or 0.1").
const long STEP_TIME_MS = 500; // Time in milliseconds it should take to make 1 manual step.
const long DELAY_BETWEEN_STEPS_MS = 80; // Time in milliseconds to wait between steps.

// Start a standalone WiFi access point and expose the web UI to control and receive GCode.
const bool WIFI_ENABLED = true;
const char* AP_SSID = "ELS";
#if __has_include("els_wifi_config.h")
#include "els_wifi_config.h"
#endif
#ifndef ELS_AP_PASSWORD
#error "Create els_wifi_config.h defining ELS_AP_PASSWORD as a quoted string"
#endif
constexpr char AP_PASSWORD[] = ELS_AP_PASSWORD;
static_assert(sizeof(AP_PASSWORD) - 1 >= 8 && sizeof(AP_PASSWORD) - 1 <= 63,
              "WiFi AP password must contain 8 to 63 characters");
const long INCOMING_BUFFER_SIZE = 100000;
const long OUTGOING_BUFFER_SIZE = 100000;

/* Changing anything below shouldn't be needed for basic use. */

// Configuration for axis connected to Y. This is uncommon. Dividing head (C) motor parameters.
// Throughout the configuration below we assume 1mm = 1degree of rotation, so 1du = 0.0001degree.
const bool ACTIVE_Y = false; // Whether the axis is connected
const bool ROTARY_Y = true; // Whether the axis is rotary or linear
const long MOTOR_STEPS_Y = 300; // Number of motor steps for 1 rotation of the the worm gear screw (full step with 20:30 reduction)
const long SCREW_Y_DU = 20000; // Degrees multiplied by 10000 that the spindle travels per 1 turn of the worm gear. 2 degrees.
const long SPEED_START_Y = 1600; // Initial speed of a motor, steps / second.
const long ACCELERATION_Y = 16000; // Acceleration of a motor, steps / second ^ 2.
const long SPEED_MANUAL_MOVE_Y = 3200; // Maximum speed of a motor during manual move, steps / second.
const bool INVERT_Y = false; // change (true/false) if the carriage moves e.g. "left" when you press "right".
const bool INVERT_Y_ENABLE = false; // change (true/false) if the Y axis enable pin is inverted
const bool NEEDS_REST_Y = false; // Set to false for closed-loop drivers. Open-loop: true if you need holding torque, false otherwise.
const long MAX_TRAVEL_MM_Y = 360; // Probably doesn't make sense to ask the dividin head to travel multiple turns.
const long BACKLASH_DU_Y = 0; // Assuming no backlash on the worm gear
const char NAME_Y = 'Y'; // Text shown on screen before axis position value, GCode axis name

// Manual handwheels. Ignore if you don't have them installed.
const float PULSE_PER_REVOLUTION = 600; // PPR of handwheels.

const int ENCODER_STEPS_INT = ENCODER_PPR * 2; // Number of encoder impulses PCNT counts per revolution of the spindle
const int ENCODER_FILTER = 1; // Encoder pulses shorter than this will be ignored. Clock cycles, 1 - 1023.
const int PCNT_LIM = 31000; // Limit used in hardware pulse counter logic.
const int PCNT_CLEAR = 30000; // Limit where we reset hardware pulse counter value to avoid overflow. Less than PCNT_LIM.
const long DUPR_MAX = 254000; // No more than 1 inch pitch
const int32_t STARTS_MAX = 124; // No more than 124-start thread
const long PASSES_MAX = 999; // No more turn or face passes than this
const long SAFE_DISTANCE_DU = 5000; // Step back 0.5mm from the material when moving between cuts in automated modes
const long SAVE_DELAY_US = 5000000; // Wait 5s after last save and last change of saveable data before saving again
const long DIRECTION_SETUP_DELAY_US = 5; // Stepper driver needs some time to adjust to direction change
const long STEPPED_ENABLE_DELAY_MS = 100; // Delay after stepper is enabled and before issuing steps

// Version of the pref storage format, should be changed when non-backward-compatible
// changes are made to the storage logic, resulting in Preferences wipe on first start.
#define PREFERENCES_VERSION 2
#define PREF_NAMESPACE "els2"

// GCode-related constants.
const float LINEAR_INTERPOLATION_PRECISION = 0.1; // 0 < x <= 1, smaller values make for quicker G0 and G1 moves
const long GCODE_WAIT_EPSILON_STEPS = 10;
const bool SPINDLE_PAUSES_GCODE = true; // pause GCode execution when spindle stops
const int GCODE_MIN_RPM = 30; // pause GCode execution if RPM is below this

// To be incremented whenever a measurable improvement is made.
#define SOFTWARE_VERSION 14

// To be changed whenever a different PCB / encoder / stepper / ... design is used.
#define HARDWARE_VERSION 5

#define Z_ENA 6
#define Z_DIR 5
#define Z_STEP 4

#define Z_PULSE_A -1  // no handwheel
#define Z_PULSE_B -1

#define X_ENA 13
#define X_DIR 8
#define X_STEP 7

#define X_PULSE_A -1  // no handwheel
#define X_PULSE_B -1

#define Y_ENA 45
#define Y_DIR 48
#define Y_STEP 47

#define Y_PULSE_A -1
#define Y_PULSE_B -1

#define BUTTON_PIN 12  // Physical button, internal pull-up, active low

#define B_LEFT 21 // Left arrow - controls Z axis movement to the left
#define B_RIGHT 22 // Right arrow - controls Z axis movement to the right
#define B_UP 23 // Up arrow - controls X axis movement forwards
#define B_DOWN 24 // Down arrow - controls X axis movement backwards
#define B_FORWARD 85 // u - Advance Y axis
#define B_BACK 74 // j - Retreat Y axis
#define B_MINUS 60 // Numpad minus - recrements the pitch or number of passes
#define B_PLUS 95 // Numpad plus - increments the pitch or number of passes
#define B_ON 30 // Enter - starts operation or mode
#define B_OFF 27 // ESC - stops operation or mode
#define B_STOPL 65 // a - sets left stop
#define B_STOPR 68 // d - sets right stop
#define B_STOPU 87 // w - sets forward stop
#define B_STOPD 83 // s - sets rear stop
#define B_STOPF 73 // i - Y forward stop
#define B_STOPB 75 // k - Y backward stop
#define B_MULTISTART 84 // t - multi-start thread button
#define B_DISPL 12 // Win - changes info displayed in the bottom line (angle, rpm, ...)
#define B_STEP 64 // Tilda - changes distance moved when movement buttons are used
#define B_MEASURE 77 // m - controls metric / imperial / tpi
#define B_REVERSE 82 // r - changes pitch sign (left / right thread)
#define B_DIAMETER 79 // o - sets X0 so that centerline is at the middle of a given diameter value
#define B_0 48 // 0 top row - for number entry
#define B_1 49 // 1 top row
#define B_2 50 // ...
#define B_3 51
#define B_4 52
#define B_5 53
#define B_6 54
#define B_7 55
#define B_8 56
#define B_9 57
#define B_BACKSPACE 28 // removes the last entered number
#define B_MODE_GEARS 97 // F1 - sets the mode to gearbox
#define B_MODE_TURN 98 // F2 - ...
#define B_MODE_FACE 99 // F3
#define B_MODE_CONE 100 // F4
#define B_MODE_CUT 101 // F5
#define B_MODE_THREAD 102 // F6
#define B_MODE_ASYNC 103 // F7
#define B_MODE_ELLIPSE 104 // F8
#define B_MODE_GCODE 105 // F9
#define B_MODE_Y 106 // F10
#define B_MODE 107 // F11 - to cycle through modes
#define B_X 88 // x - zeroes X axis
#define B_Z 90 // z - zeroes Z axis
#define B_Y 72 // h - zeroes Y axis
#define B_X_ENA 67 // c - enables / disables X axis
#define B_Z_ENA 81 // q - enables / disables Z axis
#define B_Y_ENA 89 // y - enables / disables Y axis

#define PREF_VERSION "v"
#define PREF_DUPR "d"
#define PREF_POS_Z "zp"
#define PREF_LEFT_STOP_Z "zls"
#define PREF_RIGHT_STOP_Z "zrs"
#define PREF_ORIGIN_POS_Z "zpo"
#define PREF_POS_GLOBAL_Z "zpg"
#define PREF_MOTOR_POS_Z "zpm"
#define PREF_DISABLED_Z "zd"
#define PREF_POS_X "xp"
#define PREF_LEFT_STOP_X "xls"
#define PREF_RIGHT_STOP_X "xrs"
#define PREF_ORIGIN_POS_X "xpo"
#define PREF_POS_GLOBAL_X "xpg"
#define PREF_MOTOR_POS_X "xpm"
#define PREF_DISABLED_X "xd"
#define PREF_POS_Y "y1p"
#define PREF_LEFT_STOP_Y "y1ls"
#define PREF_RIGHT_STOP_Y "y1rs"
#define PREF_ORIGIN_POS_Y "y1po"
#define PREF_POS_GLOBAL_Y "y1pg"
#define PREF_MOTOR_POS_Y "y1pm"
#define PREF_DISABLED_Y "y1d"
#define PREF_SPINDLE_POS "sp"
#define PREF_SPINDLE_POS_AVG "spa"
#define PREF_OUT_OF_SYNC "oos"
#define PREF_SPINDLE_POS_GLOBAL "spg"
#define PREF_SHOW_ANGLE "ang"
#define PREF_SHOW_TACHO "rpm"
#define PREF_STARTS "sta"
#define PREF_MODE "mod"
#define PREF_MEASURE "mea"
#define PREF_CONE_RATIO "cr"
#define PREF_TURN_PASSES "tp"
#define PREF_MOVE_STEP "ms"
#define PREF_AUX_FORWARD "af"

#define MOVE_STEP_1 10000 // 1mm
#define MOVE_STEP_2 1000 // 0.1mm
#define MOVE_STEP_3 100 // 0.01mm

#define MOVE_STEP_IMP_1 25400 // 1/10"
#define MOVE_STEP_IMP_2 2540 // 1/100"
#define MOVE_STEP_IMP_3 254 // 1/1000" also known as 1 thou

#define MODE_NORMAL 0
#define MODE_ASYNC 2
#define MODE_CONE 3
#define MODE_TURN 4
#define MODE_FACE 5
#define MODE_CUT 6
#define MODE_THREAD 7
#define MODE_ELLIPSE 8
#define MODE_GCODE 9
#define MODE_Y 10

#define MEASURE_METRIC 0
#define MEASURE_INCH 1
#define MEASURE_TPI 2

#define ESTOP_NONE 0
#define ESTOP_POS 2
#define ESTOP_MARK_ORIGIN 3
#define ESTOP_ON_OFF 4
#define ESTOP_OFF_MANUAL_MOVE 5

#define TIMER_FREQ 1000000 // 1MHz async timer frequency

struct CircleBuffer {
  char* buffer;
  size_t head;
  size_t tail;
  size_t size;
};

#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
// ArduinoOTA removed — using web OTA via /update instead
#include <WebSocketsServer.h> // install via Libraries as "WebSockets"
#include <driver/pcnt.h>
#include <Preferences.h>
// PS2 keyboard removed — input via web UI + physical button
#define PS2_BREAK 0x8000  // Key release flag (originally from PS2KeyAdvanced.h)
#include <U8g2lib.h>
#include <Wire.h>

#define OLED_SCL 9
#define OLED_SDA 10

const char indexhtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>ELS2</title>
<link rel="icon" href="data:;base64,">
<style>
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent}
body{font-family:'SF Mono',Consolas,monospace;background:#1a1a2e;color:#e0e0e0;touch-action:manipulation;user-select:none;overflow:auto;height:100vh}
.wrap{display:flex;flex-direction:column;height:100vh;padding:2px;gap:2px}
.status{display:flex;align-items:center;gap:4px;padding:4px 8px;background:#16213e;border-radius:4px;flex-wrap:wrap}
.status .lbl{color:#888;font-size:11px}
.status .val{font-size:14px;font-weight:bold}
.pill{padding:3px 10px;border-radius:4px;font-size:13px;font-weight:bold;cursor:pointer;border:none}
.pill-off{background:#c0392b;color:#fff}
.pill-on{background:#27ae60;color:#fff}
.pill-syn{background:#e67e22;color:#fff}
.pill-mode{background:#2980b9;color:#fff}
.pill-meas{background:#555;color:#fff}
.info{display:flex;gap:12px;padding:2px 8px;background:#16213e;border-radius:4px;font-size:11px;color:#aaa}
.info .val{color:#e0e0e0;font-weight:bold}
.axes{display:flex;flex-direction:column;gap:1px;padding:3px 8px;background:#16213e;border-radius:4px}
.axis-row{display:flex;align-items:center;gap:8px;font-size:14px}
.axis-name{font-weight:bold;color:#3498db;width:18px}
.axis-pos{font-size:18px;font-weight:bold;color:#ecf0f1;min-width:100px;text-align:right}
.axis-stop{font-size:12px;color:#888;min-width:80px}
.ctx{padding:3px 8px;background:#0f3460;border-radius:4px;font-size:13px;color:#f39c12;font-weight:bold;min-height:22px;display:flex;align-items:center}
.controls{display:flex;gap:2px}
.col{display:flex;flex-direction:column;gap:2px;flex:1}
.row{display:flex;gap:2px;height:48px}
button{border:none;border-radius:4px;font-family:inherit;font-size:12px;font-weight:bold;cursor:pointer;display:flex;align-items:center;justify-content:center;min-height:0;flex:1}
.btn-jog{background:#2c3e50;color:#ecf0f1;font-size:18px}
.btn-jog:active{background:#3498db}
.btn-ctrl{background:#34495e;color:#ecf0f1}
.btn-ctrl:active{background:#5dade2}
.btn-stop{background:#555;color:#f39c12;font-size:11px}
.btn-stop:active{background:#e67e22}
.btn-stop.set{background:#27ae60;color:#fff}
.btn-zero{background:#1a5276;color:#85c1e9}
.btn-zero:active{background:#2980b9}
.btn-ena{background:#1a5276;color:#85c1e9}
.btn-ena:active{background:#2980b9}
.btn-num{background:#2c3e50;color:#ecf0f1;font-size:18px}
.btn-num:active{background:#5dade2}
.btn-on{background:#27ae60;color:#fff;font-size:16px}
.btn-on:active{background:#2ecc71}
.btn-off{background:#c0392b;color:#fff;font-size:16px}
.btn-off:active{background:#e74c3c}
.bar{display:flex;gap:2px;height:38px}
.modal-bg{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.7);z-index:10;align-items:center;justify-content:center}
.modal-bg.show{display:flex}
.modal{background:#16213e;border-radius:10px;padding:12px;display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;min-width:300px}
.modal button{padding:14px;font-size:15px;background:#2c3e50;color:#ecf0f1;border-radius:8px}
.modal button:active{background:#3498db}
.disconnected{position:fixed;top:0;left:0;width:100%;padding:8px;background:#c0392b;color:#fff;text-align:center;font-weight:bold;z-index:20;display:none}
</style>
</head>
<body>
<div class="disconnected" id="disc">DISCONNECTED</div>
<div class="wrap">
  <div class="status">
    <span class="pill pill-off" id="sStatus" onclick="press(27)">OFF</span>
    <span class="pill pill-mode" id="sMode" onclick="showModes()">GEAR</span>
    <span class="lbl">Pitch:</span><span class="val" id="sPitch">0</span>
    <span class="pill pill-meas" id="sMeas" onclick="press(77)">MM</span>
    <span class="lbl">Step:</span><span class="val" id="sStep">1mm</span>
  </div>
  <div class="info">
    <span><span class="lbl">RPM </span><span class="val" id="iRpm">0</span></span>
    <span><span class="lbl">Turns </span><span class="val" id="iTrn">0</span></span>
    <span><span class="lbl">Angle </span><span class="val" id="iAng">0</span></span>
  </div>
  <div class="axes">
    <div class="axis-row">
      <span class="axis-name">Z</span>
      <span class="axis-pos" id="aZ">0.000</span>
      <span class="axis-stop" id="aZL"></span>
      <span class="axis-stop" id="aZR"></span>
    </div>
    <div class="axis-row">
      <span class="axis-name">X</span>
      <span class="axis-pos" id="aX">0.000</span>
      <span class="axis-stop" id="aXU"></span>
      <span class="axis-stop" id="aXD"></span>
    </div>
  </div>
  <div class="ctx" id="ctx"></div>
  <div class="controls">
    <div class="col">
      <div class="row"><button class="btn-jog" onmousedown="jogS(21)" onmouseup="jogE(21)" ontouchstart="jogS(21)" ontouchend="jogE(21)" ontouchcancel="jogE(21)">Z&#9664;</button><button class="btn-jog" onmousedown="jogS(22)" onmouseup="jogE(22)" ontouchstart="jogS(22)" ontouchend="jogE(22)" ontouchcancel="jogE(22)">Z&#9654;</button></div>
      <div class="row"><button class="btn-jog" onmousedown="jogS(23)" onmouseup="jogE(23)" ontouchstart="jogS(23)" ontouchend="jogE(23)" ontouchcancel="jogE(23)">X&#9650;</button><button class="btn-jog" onmousedown="jogS(24)" onmouseup="jogE(24)" ontouchstart="jogS(24)" ontouchend="jogE(24)" ontouchcancel="jogE(24)">X&#9660;</button></div>
      <div class="row"><button class="btn-ctrl" onclick="press(95)">+</button><button class="btn-ctrl" onclick="press(60)">-</button></div>
      <div class="row"><button class="btn-ctrl" onclick="press(64)">STEP</button><button class="btn-ctrl" onclick="press(82)">REV</button></div>
    </div>
    <div class="col">
      <div class="row"><button class="btn-stop" id="bZL" onclick="press(65)">STOP Z Chuck</button><button class="btn-stop" id="bZR" onclick="press(68)">STOP Z Tail</button></div>
      <div class="row"><button class="btn-stop" id="bXU" onclick="press(87)">STOP X Depth</button><button class="btn-stop" id="bXD" onclick="press(83)">STOP X Start</button></div>
      <div class="row"><button class="btn-zero" onclick="press(90)">Z=0</button><button class="btn-zero" onclick="press(88)">X=0</button></div>
      <div class="row"><button class="btn-ena" id="bZE" onclick="press(81)">Z ON</button><button class="btn-ena" id="bXE" onclick="press(67)">X ON</button></div>
    </div>
    <div class="col">
      <div class="row"><button class="btn-num" onclick="press(49)">1</button><button class="btn-num" onclick="press(50)">2</button><button class="btn-num" onclick="press(51)">3</button></div>
      <div class="row"><button class="btn-num" onclick="press(52)">4</button><button class="btn-num" onclick="press(53)">5</button><button class="btn-num" onclick="press(54)">6</button></div>
      <div class="row"><button class="btn-num" onclick="press(55)">7</button><button class="btn-num" onclick="press(56)">8</button><button class="btn-num" onclick="press(57)">9</button></div>
      <div class="row"><button class="btn-num" onclick="press(28)">&#9003;</button><button class="btn-num" onclick="press(48)">0</button><button class="btn-ctrl" onclick="press(84)">MS</button></div>
    </div>
  </div>
  <div class="bar">
    <button class="btn-off" onclick="press(27)">&#9632; OFF</button>
    <button class="btn-on" onclick="press(30)">&#9654; ON</button>
    <button class="pill pill-mode" onclick="showModes()" style="flex:0.5">MODE</button>
    <button class="pill pill-meas" onclick="doReset()" style="flex:0.3">RST</button>
    <button class="pill pill-meas" onclick="showOta()" style="flex:0.3">OTA</button>
  </div>
</div>

<div class="modal-bg" id="modeBg" onclick="hideModes()">
  <div class="modal" onclick="event.stopPropagation()">
    <button onclick="selMode(97)">Gearbox</button>
    <button onclick="selMode(98)">Turn</button>
    <button onclick="selMode(99)">Face</button>
    <button onclick="selMode(100)">Cone</button>
    <button onclick="selMode(101)">Cut</button>
    <button onclick="selMode(102)">Thread</button>
    <button onclick="selMode(103)">Async</button>
    <button onclick="selMode(104)">Ellipse</button>
    <button onclick="selMode(105)">GCode</button>
  </div>
</div>

<div class="modal-bg" id="otaBg" onclick="hideOta()">
  <div class="modal" onclick="event.stopPropagation()" style="grid-template-columns:1fr;min-width:320px">
    <h3 style="color:#ecf0f1;margin:0">Firmware Update</h3>
    <p style="color:#aaa;font-size:12px;margin:4px 0">Select .bin file. Machine must be stopped.</p>
    <form id="otaForm" style="display:flex;flex-direction:column;gap:8px">
      <input type="file" id="otaFile" accept=".bin" style="color:#e0e0e0;font-size:14px">
      <button type="submit" style="background:#e67e22;padding:12px">Upload &amp; Flash</button>
    </form>
    <p id="otaMsg" style="color:#aaa;font-size:13px;margin:4px 0"></p>
  </div>
</div>

<script>
let ws,reconn;
const disc=document.getElementById('disc');

function connect(){
  ws=new WebSocket('ws://'+location.hostname+':81');
  ws.onopen=()=>{disc.style.display='none';clearTimeout(reconn);jogE(21);jogE(22);jogE(23);jogE(24)};
  ws.onclose=()=>{disc.style.display='block';reconn=setTimeout(connect,1500)};
  ws.onerror=()=>{ws.close()};
  ws.onmessage=(e)=>{
    const d=e.data;
    if(d.startsWith('{')){try{const s=JSON.parse(d);updateUI(s)}catch(ex){}}
  };
}

function updateUI(s){
  document.getElementById('sStatus').textContent=s.s||'OFF';
  const st=document.getElementById('sStatus');
  st.className='pill '+(s.s==='ON'?'pill-on':s.s==='SYN'?'pill-syn':'pill-off');
  document.getElementById('sMode').textContent=s.m||'';
  document.getElementById('sPitch').textContent=s.p||'0';
  document.getElementById('sMeas').textContent=s.u||'MM';
  document.getElementById('sStep').textContent=s.st||'';
  document.getElementById('iRpm').textContent=s.rpm||'0';
  document.getElementById('iTrn').textContent=s.trn||'0';
  document.getElementById('iAng').textContent=(s.ang||'0')+'\u00B0';
  document.getElementById('aZ').textContent=s.z||'0.000';
  document.getElementById('aZL').textContent=s.zl?('\u2190'+s.zl):'';
  document.getElementById('aZR').textContent=s.zr?(s.zr+'\u2192'):'';
  document.getElementById('aX').textContent=s.x||'0.000';
  document.getElementById('aXU').textContent=s.xu?('\u2191'+s.xu):'';
  document.getElementById('aXD').textContent=s.xd?(s.xd+'\u2193'):'';
  document.getElementById('bZL').className='btn-stop'+(s.zl?' set':'');
  document.getElementById('bZR').className='btn-stop'+(s.zr?' set':'');
  document.getElementById('bXU').className='btn-stop'+(s.xu?' set':'');
  document.getElementById('bXD').className='btn-stop'+(s.xd?' set':'');
  document.getElementById('ctx').textContent=s.t3||'';
  document.getElementById('bZE').textContent=s.zd?'Z OFF':'Z ON';
  document.getElementById('bXE').textContent=s.xd?'X OFF':'X ON';
  if(s.est&&s.est!==0){
    document.getElementById('sStatus').textContent='ESTOP';
    document.getElementById('sStatus').className='pill pill-off';
  }
}

function press(k){if(ws&&ws.readyState===1)ws.send('='+k+'\n')}
function jogS(k){if(ws&&ws.readyState===1)ws.send('='+k+'\n')}
function jogE(k){if(ws&&ws.readyState===1)ws.send('=-'+k+'\n')}

function showModes(){document.getElementById('modeBg').classList.add('show')}
function hideModes(){document.getElementById('modeBg').classList.remove('show')}
function selMode(k){press(k);hideModes()}

function doReset(){if(confirm('Clear all stops, positions and settings?'))if(ws&&ws.readyState===1)ws.send('R')}
function showOta(){document.getElementById('otaBg').classList.add('show')}
function hideOta(){document.getElementById('otaBg').classList.remove('show')}
document.getElementById('otaForm').addEventListener('submit',function(e){
  e.preventDefault();
  const file=document.getElementById('otaFile').files[0];
  if(!file)return;
  const msg=document.getElementById('otaMsg');
  msg.textContent='Uploading...';msg.style.color='#f39c12';
  const fd=new FormData();
  fd.append('update',file);
  fetch('/update',{method:'POST',body:fd}).then(r=>r.text()).then(t=>{
    if(t.trim()==='OK'){msg.textContent='OK! Rebooting...';msg.style.color='#27ae60'}
    else{msg.textContent='Failed: '+t;msg.style.color='#c0392b'}
  }).catch(err=>{msg.textContent='Error: '+err;msg.style.color='#c0392b'});
});

document.addEventListener('touchstart',function(e){
  if(e.target.classList.contains('btn-jog'))e.preventDefault();
},{passive:false});

connect();
</script>
</body>
</html>
)rawliteral";

// GCode management page (original NanoELS H5 web UI)
const char gcodehtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>NanoEls H5</title>
  <link rel="icon" href="data:;base64,">
  <style>
    body {
      font-family: Roboto, sans-serif;
      margin: 0 auto;
      max-width: 800px;
      padding: 20px;
      background-color: #f4f4f4;
    }
    h1, h2 {
      color: #333;
    }
    input[type=text], textarea {
      width: 100%;
    }
    #log {
      height: 200px;
      overflow-y: scroll;
      border: 1px solid #ccc;
      padding: 10px;
      background-color: #fff;
      margin-bottom: 20px;
    }
    #log p {
      padding: 0;
      margin: 0;
    }
    #command-container, #gcode-container {
      display: flex;
      align-items: center;
      margin-bottom: 20px;
    }
    #command {
      flex: 1;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 4px;
      margin-right: 10px;
    }
    button {
      padding: 10px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      color: #fff;
      background-color: #218838;
    }
    button:hover {
      background-color: #105b21;
    }
    #gcode-list {
      margin-top: 20px;
      background-color: #fff;
      padding: 0;
      border: 1px solid #ccc;
      border-radius: 4px;
    }
    #gcode-list.empty {
      padding: 10px;
      text-align: center;
    }
    #gcode-name, #gcode-content {
      box-sizing: border-box;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 4px;
      margin-bottom: 10px;
    }
    #gcode-content {
      height: 200px;
      resize: vertical;
    }
    .remove-icon {
      cursor: pointer;
      color: #dc3545;
      font-size: 16px;
      width: 20px;
    }
    .remove-icon:hover {
      color: #c82333;
    }
    button.disabled {
      background-color: #ccc;
      cursor: not-allowed;
    }
    button.disabled:hover {
      background-color: #ccc;
    }
    .gcode-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 10px;
      cursor: pointer;
    }
    .gcode-row:hover {
      background-color: #f0f0f0;
    }
    .gcode-item {
      flex: 1;
    }
    .gcode-size {
      flex-basis: 80px;
      font-size: 0.9em;
      color: #666;
    }
    .checkbox-container {
      align-items: center;
      display: flex;
      margin-bottom: 10px;
    }
    .checkbox-container input {
      margin: 10px 10px 10px 20px;
    }
  </style>
</head>
<body>
  <h1>NanoEls H5</h1>
  <p>This Web UI is served from your NanoEls controller memory. It doesn't need Internet connection. Anyone on your local network has access to it.</p>
  <p>
    It communicates with NanoEls in 2 ways. For saving, loading and removing stored GCode files it uses HTTP calls that can succeed or fail
    individually and it doesn't affect the realtime communication with the NanoEls controller.
  </p>
  <p>
    For realtime communication it uses WebSocket that is always open and can be used to send commands to the controller and receive responses from it.
    Realtime log can also be used for debugging since serial communication with NanoEls can be problematic as that channel is used by the screen.
  </p>
  <h2>Stored GCode</h2>
  <div id="gcode-list"></div>
  <p id="free-space"></p>
  <h2>Add GCode</h2>
  <p>You can generate suitable GCode using <a href="https://kachurovskiy.com/lathecode/" target="_blank">lathecode</a> by uploading STL model of the part
    and specifying other parameters like tool and stock diameter.</p>
  <input type="text" id="gcode-name" placeholder="GCode name" required minlength="2">
  <textarea id="gcode-content" placeholder="GCode content" required minlength="2"></textarea>
  <div class="checkbox-container">
    <button id="add-gcode">Save</button>
    <input type="checkbox" id="remove-comments" checked>
    <label for="remove-comments">Remove comments before saving</label>
  </div>

  <h2>WebSocket realtime communication</h2>
  <div id="log"></div>
  <div id="command-container">
    <input type="text" id="command" placeholder="Enter command" value="?" minlength="1" required>
    <button id="send">Send</button>
  </div>
  <p>Supported websocket commands:</p>
  <ul>
    <li><code>?</code> requests controller status</li>
    <li><code>=20</code> send key code 20 as if it's pressed on the keyboard</li>
    <li><code>!</code> turns the controller off</li>
    <li><code>~</code> turns the controller on</li>
    <li><code>""</code> removes all GCode</li>
  </ul>

  <script>
    const log = document.getElementById('log');
    const commandInput = document.getElementById('command');
    const sendButton = document.getElementById('send');
    const gcodeList = document.getElementById('gcode-list');
    const gcodeNameInput = document.getElementById('gcode-name');
    const gcodeContentInput = document.getElementById('gcode-content');
    const addGcodeButton = document.getElementById('add-gcode');
    const removeCommentsCheckbox = document.getElementById('remove-comments');

    const ws = new WebSocket(`ws://${window.location.host.split(':')[0]}:81`);

    ws.onopen = () => {
      logMessage('Connected to server');
    };

    ws.onmessage = (event) => {
      logMessage('Received: ' + event.data);
    };

    ws.onclose = () => {
      logMessage('Disconnected from server');
    };

    function updateButtonStates() {
      sendButton.disabled = commandInput.value.trim().length < 1;
      addGcodeButton.disabled = gcodeNameInput.value.trim().length < 2 || gcodeContentInput.value.trim().length < 2;
      sendButton.classList.toggle('disabled', sendButton.disabled);
      addGcodeButton.classList.toggle('disabled', addGcodeButton.disabled);
    }

    commandInput.addEventListener('input', updateButtonStates);
    gcodeNameInput.addEventListener('input', updateButtonStates);
    gcodeContentInput.addEventListener('input', updateButtonStates);

    document.addEventListener('DOMContentLoaded', () => {
      updateButtonStates();
    });

    function send() {
      const command = commandInput.value.trim();
      if (command) {
        logMessage('Sent: ' + command);
        ws.send(command + '\n');
        commandInput.value = '';
        updateButtonStates();
      }
    }

    commandInput.addEventListener('keydown', (event) => {
      if (event.key === 'Enter') send();
    });

    sendButton.addEventListener('click', () => {
      send();
    });

    function removeComments(content) {
      return content.split('\n').map(line => line.split(';')[0].trim()).filter(line => !!line).join('\n');
    }

    addGcodeButton.addEventListener('click', () => {
      const name = gcodeNameInput.value.trim();
      let content = gcodeContentInput.value.trim();
      if (removeCommentsCheckbox.checked) {
        content = removeComments(content);
      }
      if (name && content) {
        fetch('/gcode/add', {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          body: new URLSearchParams({ name, gcode: content })
        })
        .then(response => response.text())
        .then(data => {
          logMessage(data);
          listGcodes();
          gcodeNameInput.value = '';
          gcodeContentInput.value = '';
          updateButtonStates();
        });
      }
    });

    function listGcodes() {
      fetch('/gcode/list')
        .then(response => response.text())
        .then(data => {
          gcodeList.innerHTML = '';
          gcodeList.classList.toggle('empty', !data);
          if (data) {
            data.split('\n').map(g => g.trim()).filter(g => !!g).forEach(gcode => {
              const row = document.createElement('div');
              row.className = 'gcode-row';
              row.dataset.name = gcode;
              row.innerHTML = `
                <span class="gcode-item" data-name="${gcode}">${gcode}</span>
                <span class="gcode-size"></span>
                <span class="remove-icon" data-name="${gcode}">&times;</span>
              `;
              row.addEventListener('click', (event) => {
                loadGcode(event.target.dataset.name);
              });
              row.title = 'Click to load G-code';
              gcodeList.appendChild(row);
              fetch(`/gcode/get?name=${encodeURIComponent(gcode)}`)
                .then(response => response.text())
                .then(text => {
                  row.querySelector('.gcode-size').textContent = `${(text.length / 1024).toFixed(1)} KB`;
                })
            });
            document.querySelectorAll('.remove-icon').forEach(icon => {
              icon.title = 'Click to remove G-code';
              icon.addEventListener('click', (event) => {
                const name = event.target.getAttribute('data-name');
                removeGcode(name);
              });
            });
          } else {
            gcodeList.innerHTML = 'No G-code stored';
          }
          fetchFreeSpace();
        });
    }

    function loadGcode(name) {
      fetch(`/gcode/get?name=${encodeURIComponent(name)}`)
        .then(response => response.text())
        .then(data => {
          gcodeNameInput.value = name;
          gcodeContentInput.value = data;
          updateButtonStates();
        });
    }

    function removeGcode(name) {
      fetch('/gcode/remove', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: new URLSearchParams({ name })
      })
      .then(response => response.text())
      .then(data => {
        logMessage(data);
        listGcodes();
      });
    }

    function fetchFreeSpace() {
      fetch('/status')
        .then(response => response.text())
        .then(data => {
          const freeSpaceBytes = Number(data.split('\n').find(l => l.startsWith('LittleFS.freeSpace=')).substr('LittleFS.freeSpace='.length));
          if (freeSpaceBytes) {
            const freeSpaceElement = document.getElementById('free-space');
            freeSpaceElement.textContent = `Free space: ${Math.floor(freeSpaceBytes / 1024)} KB`;
          }
        });
    }

    function logMessage(message) {
      const p = document.createElement('p');
      p.textContent = message;
      log.appendChild(p);
      log.scrollTop = log.scrollHeight;
    }

    listGcodes();
  </script>
</body>
</html>
)rawliteral";

#define FORMAT_LITTLEFS_IF_FAILED true

// For MEASURE_TPI, round TPI to the nearest integer if it's within this range of it.
// E.g. 80.02tpi would be shown as 80tpi but 80.04tpi would be shown as-is.
const float TPI_ROUND_EPSILON = 0.03;

const float ENCODER_STEPS_FLOAT = ENCODER_STEPS_INT; // Convenience float version of ENCODER_STEPS_INT
const long RPM_BULK = ENCODER_STEPS_INT; // Measure RPM averaged over this number of encoder pulses

const long GCODE_FEED_DEFAULT_DU_SEC = 20000; // Default feed in du/sec in GCode mode
const float GCODE_FEED_MIN_DU_SEC = 167; // Minimum feed in du/sec in GCode mode - F1

#define DHIGH(x) digitalWrite(x, HIGH)
#define DLOW(x) digitalWrite(x, LOW)

#define DELAY(x) vTaskDelay(x / portTICK_PERIOD_MS);

#define LCD_HASH_INITIAL -3845709 // Random number that's unlikely to naturally occur as an actual hash
long lcdHashLine0 = LCD_HASH_INITIAL;
long lcdHashLine1 = LCD_HASH_INITIAL;
long lcdHashLine2 = LCD_HASH_INITIAL;
long lcdHashLine3 = LCD_HASH_INITIAL;
bool splashScreen = false;

// Display state strings — populated by updateDisplay() via setText(), read by broadcastStatus()
String ds_status, ds_mode, ds_pitch, ds_measure;
String ds_step, ds_rpm, ds_turns, ds_angle;
String ds_z, ds_zl, ds_zr;
String ds_x, ds_xu, ds_xd;
String ds_y, ds_yu, ds_yd;
String ds_t3;
long ds_hash = 0;

unsigned long keypadTimeUs = 0;

// Most buttons we only have "down" handling, holding them has no effect.
// Buttons with special "holding" logic have flags below.
bool buttonLeftPressed = false;
bool buttonRightPressed = false;
bool buttonUpPressed = false;
bool buttonDownPressed = false;
bool buttonOffPressed = false;
bool buttonBackPressed = false;
bool buttonForwardPressed = false;

bool inNumpad = false;
int numpadDigits[20];
int numpadIndex = 0;

bool isOn = false;
bool nextIsOn; // isOn value that should be applied asap
bool nextIsOnFlag; // whether nextIsOn requires attention
unsigned long resetMillis = 0;
int emergencyStop = 0;

bool beepFlag = false; // allows time-critical code to ask for a beep on another core

long dupr = 0; // pitch, tenth of a micron per rotation
long savedDupr = 0; // dupr saved in Preferences
long nextDupr = dupr; // dupr value that should be applied asap
bool nextDuprFlag = false; // whether nextDupr requires attention

SemaphoreHandle_t motionMutex; // controls blocks of code where variables affecting the motion loop() are changed

int starts = 1; // number of starts in a multi-start thread
int savedStarts = 0; // starts saved in Preferences
int nextStarts = starts; // number of starts that should be used asap
bool nextStartsFlag = false; // whether nextStarts requires attention

struct Axis {
  SemaphoreHandle_t mutex;

  char name;
  bool active;
  bool rotational;
  float motorSteps; // motor steps per revolution of the axis
  float screwPitch; // lead screw pitch in deci-microns (10^-7 of a meter)

  long pos; // relative position of the tool in stepper motor steps
  long savedPos; // value saved in Preferences
  float fractionalPos; // fractional distance in steps that we meant to travel but couldn't
  long originPos; // relative position of the stepper motor to origin, in steps
  long savedOriginPos; // originPos saved in Preferences
  long posGlobal; // global position of the motor in steps
  long savedPosGlobal; // posGlobal saved in Preferences
  int pendingPos; // steps of the stepper motor that we should make as soon as possible
  long motorPos; // position of the motor in stepper motor steps, same as pos unless moving back, then differs by backlashSteps
  long savedMotorPos; // motorPos saved in Preferences
  bool continuous; // whether current movement is expected to continue until an unknown position

  long leftStop; // left stop value of pos
  long savedLeftStop; // value saved in Preferences
  long nextLeftStop; // left stop value that should be applied asap
  bool nextLeftStopFlag; // whether nextLeftStop required attention

  long rightStop; // right stop value of pos
  long savedRightStop; // value saved in Preferences
  long nextRightStop; // right stop value that should be applied asap
  bool nextRightStopFlag; // whether nextRightStop requires attention

  long speed; // motor speed in steps / second
  long speedStart; // Initial speed of a motor, steps / second.
  long speedMax; // To limit max speed e.g. for manual moves
  long speedManualMove; // Maximum speed of a motor during manual move, steps / second.
  long acceleration; // Acceleration of a motor, steps / second ^ 2.
  long decelerateSteps; // Number of steps before the end position the deceleration should start.

  bool direction; // To reset speed when direction changes.
  bool directionInitialized;
  unsigned long stepStartUs;
  int stepperEnableCounter;
  bool disabled;
  bool savedDisabled;

  bool invertStepper; // change (true/false) if the carriage moves e.g. "left" when you press "right".
  bool invertEnable; // change (true/false) if the Enable pin is inverted
  bool needsRest; // set to false for closed-loop drivers, true for open-loop.
  bool movingManually; // whether stepper is being moved by left/right buttons
  long estopSteps; // amount of steps to exceed machine limits
  long backlashSteps; // amount of steps in reverse direction to re-engage the carriage
  long gcodeRelativePos; // absolute position in steps that relative GCode refers to

  int ena; // Enable pin of this motor
  int dir; // Direction pin of this motor
  int step; // Step pin of this motor

  int pulseA;
  int pulseB;
  int pulseCount;
  pcnt_unit_t pulseUnit;
};

void initAxis(Axis* a, char name, bool active, bool rotational, float motorSteps, float screwPitch, long speedStart, long speedManualMove,
    long acceleration, bool invertStepper, bool invertEnable, bool needsRest, long maxTravelMm, long backlashDu, int ena, int dir, int step, int pulseA, int pulseB, pcnt_unit_t pulseUnit) {
  a->mutex = xSemaphoreCreateMutex();

  a->name = name;
  a->active = active;
  a->rotational = rotational;
  a->motorSteps = motorSteps;
  a->screwPitch = screwPitch;

  a->pos = 0;
  a->savedPos = 0;
  a->fractionalPos = 0.0;
  a->originPos = 0;
  a->savedOriginPos = 0;
  a->posGlobal = 0;
  a->savedPosGlobal = 0;
  a->pendingPos = 0;
  a->motorPos = 0;
  a->savedMotorPos = 0;
  a->continuous = false;

  a->leftStop = 0;
  a->savedLeftStop = 0;
  a->nextLeftStopFlag = false;

  a->rightStop = 0;
  a->savedRightStop = 0;
  a->nextRightStopFlag = false;

  a->speed = speedStart;
  a->speedStart = speedStart;
  a->speedMax = LONG_MAX;
  a->speedManualMove = speedManualMove;
  a->acceleration = acceleration;
  a->decelerateSteps = 0;
  long s = speedManualMove;
  while (s > speedStart) {
    a->decelerateSteps++;
    s -= a->acceleration / float(s);
  }

  a->direction = true;
  a->directionInitialized = false;
  a->stepStartUs = 0;
  a->stepperEnableCounter = 0;
  a->disabled = false;
  a->savedDisabled = false;

  a->invertStepper = invertStepper;
  a->invertEnable = invertEnable;
  a->needsRest = needsRest;
  a->movingManually = false;
  a->estopSteps = maxTravelMm * 10000 / a->screwPitch * a->motorSteps;
  a->backlashSteps = backlashDu * a->motorSteps / a->screwPitch;
  a->gcodeRelativePos = 0;

  a->ena = ena;
  a->dir = dir;
  a->step = step;

  a->pulseA = pulseA;
  a->pulseB = pulseB;
  a->pulseUnit = pulseUnit;
  a->pulseCount = 0;
}

Axis z;
Axis x;
Axis y;

unsigned long saveTime = 0; // micros() of the previous Prefs write
unsigned long spindleEncTime = 0; // micros() of the previous spindle update
unsigned long spindleEncTimeDiffBulk = 0; // micros() between RPM_BULK spindle updates
unsigned long spindleEncTimeAtIndex0 = 0; // micros() when spindleEncTimeIndex was 0
int spindleEncTimeIndex = 0; // counter going between 0 and RPM_BULK - 1
long spindlePos = 0; // Spindle position
long spindlePosAvg = 0; // Spindle position accounting for encoder backlash
long savedSpindlePosAvg = 0; // spindlePosAvg saved in Preferences
long savedSpindlePos = 0; // spindlePos value saved in Preferences
int spindleCount = 0; // Last processed spindle encoder pulse counter value.
int spindlePosSync = 0; // Non-zero if gearbox is on and a soft limit was removed while axis was on it
int savedSpindlePosSync = 0; // spindlePosSync saved in Preferences
long spindlePosGlobal = 0; // global spindle position that is unaffected by e.g. zeroing
long savedSpindlePosGlobal = 0; // spindlePosGlobal saved in Preferences

bool showAngle = false; // Whether to show 0-359 spindle angle on screen
bool showTacho = false; // Whether to show spindle RPM on screen
bool savedShowAngle = false; // showAngle value saved in Preferences
bool savedShowTacho = false; // showTacho value saved in Preferences
int shownRpm = 0;
unsigned long shownRpmTime = 0; // micros() when shownRpm was set

long moveStep = 0; // in deci-microns
long savedMoveStep = 0; // moveStep saved in Preferences

volatile int mode = -1; // mode of operation (ELS, multi-start ELS, asynchronous)
int nextMode = 0; // mode value that should be applied asap
bool nextModeFlag = false; // whether nextMode needs attention
int savedMode = -1; // mode saved in Preferences

int measure = MEASURE_METRIC; // Whether to show distances in inches
int savedMeasure = MEASURE_METRIC; // measure value saved in Preferences

float coneRatio = 1; // In cone mode, how much X moves for 1 step of Z
float savedConeRatio = 0; // value of coneRatio saved in Preferences
float nextConeRatio = 0; // coneRatio that should be applied asap
bool nextConeRatioFlag = false; // whether nextConeRatio requires attention

int turnPasses = 3; // In turn mode, how many turn passes to make
int savedTurnPasses = 0; // value of turnPasses saved in Preferences

long setupIndex = 0; // Index of automation setup step
bool auxForward = true; // True for external, false for internal thread
bool savedAuxForward = false; // value of auxForward saved in Preferences

long opIndex = 0; // Index of an automation operation
bool opIndexAdvanceFlag = false; // Whether user requested to move to the next pass
long opSubIndex = 0; // Sub-index of an automation operation
int opDuprSign = 1; // 1 if dupr was positive when operation started, -1 if negative
long opDupr = 0; // dupr that the multi-pass operation started with

String gcodeCommand = "";
long gcodeFeedDuPerSec = GCODE_FEED_DEFAULT_DU_SEC;
bool gcodeInitialized = false;
bool gcodeAbsolutePositioning = true;
bool gcodeInBrace = false;
bool gcodeInSemicolon = false;
bool wsInKeycode = false;
int wsKeycode = 0;
String keycodeCommand = "";
bool gcodeInSave = false;
bool gcodeInSaveFirstLine = false;
String gcodeSaveName = "";
String gcodeSaveValue = "";
int gcodeProgramIndex = 0;
int gcodeProgramCount = 0;
String gcodeProgram = "";
int gcodeProgramCharIndex = 0;

// PS2 keyboard removed

hw_timer_t *async_timer = timerBegin(TIMER_FREQ);
bool timerAttached = false;

CircleBuffer inBuffer;
CircleBuffer outBuffer;

bool bufferAvailable(CircleBuffer* b) {
  return b->head != b->tail;
}

bool writeBuffer(CircleBuffer* b, char c) {
  if ((b->head + 1) % b->size == b->tail) {
    return false;
  }
  b->buffer[b->head] = c;
  b->head = (b->head + 1) % b->size;
  return true;
}

bool writeBuffer(CircleBuffer* b, const char* str) {
  while (*str) {
    if (!writeBuffer(b, *str++)) {
      return false;
    }
  }
  return true;
}

bool writeBuffer(CircleBuffer* b, const String& str) {
  return writeBuffer(b, str.c_str());
}

bool writeBuffer(CircleBuffer* b, float f, int precision) {
  char buffer[16];
  dtostrf(f, 0, precision, buffer);
  return writeBuffer(b, buffer);
}

char shiftBuffer(CircleBuffer* b) {
  if (b->head == b->tail) {
    return 0;
  }
  char c = b->buffer[b->tail];
  b->tail = (b->tail + 1) % b->size;
  return c;
}

void initBuffer(CircleBuffer* b, size_t size) {
  b->size = size;
  b->buffer = (char*)malloc(size);
  b->head = 0;
  b->tail = 0;
}

void clearBuffer(CircleBuffer* b) {
  b->head = 0;
  b->tail = 0;
}

WebServer server(80);
WebSocketsServer webSocket(81);
String wifiStatus = "No WiFi";
unsigned long wifiStatusMillis = 0;

void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_TEXT) {
    for (size_t i = 0; i < length; i++) {
      writeBuffer(&inBuffer, payload[i]);
    }
  }
}

void handleClientRequests() {
  server.send(200, "text/html", indexhtml);
}

int getGcodeProgramCount() {
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    writeBuffer(&outBuffer, "error: failed to open directory\n");
    return 0;
  }

  int count = 0;
  File file = root.openNextFile();
  while (file) {
    String filename = file.name();
    file.close();
    if (filename.endsWith(".gcode")) {
      count++;
    }
    file = root.openNextFile();
  }

  return count;
}

bool saveGcode() {
  if (gcodeSaveName.length() < 2) {
    writeBuffer(&outBuffer, "error: name must be at least 2 chars\n");
    return false;
  }
  if (gcodeSaveValue.length() < 2) {
    writeBuffer(&outBuffer, "error: program too short\n");
    return false;
  }

  String filename = "/" + gcodeSaveName + ".gcode";
  File file = LittleFS.open(filename, "w");
  if (!file) {
    writeBuffer(&outBuffer, "error: failed to open file\n");
    return false;
  }

  file.print(gcodeSaveValue);
  file.close();

  writeBuffer(&outBuffer, "success: G-code saved\n");
  gcodeProgramCount = getGcodeProgramCount();
  return true;
}

bool removeGcodeByName(const String& name) {
  if (name.length() == 0) return false;

  String filename = "/" + name + ".gcode";
  if (!LittleFS.exists(filename)) {
    writeBuffer(&outBuffer, "error: file not found\n");
    return false;
  }

  if (!LittleFS.remove(filename)) {
    writeBuffer(&outBuffer, "error: failed to delete " + filename + "\n");
    return false;
  }

  writeBuffer(&outBuffer, "success: " + name + " deleted\n");
  gcodeProgramCount = getGcodeProgramCount();
  return true;
}

String readGcodeProgram(const String& name) {
  String filename = "/" + name + ".gcode";
  File file = LittleFS.open(filename, "r");
  if (!file) {
    return "";
  }

  String result;
  result.reserve(file.size());
  char buf[64];
  while (file.available()) {
    size_t bytesRead = file.readBytes(buf, sizeof(buf));
    result += String(buf).substring(0, bytesRead);
  }
  file.close();
  return result;
}

void handleGcodeAdd() {
  if (server.hasArg("name") && server.hasArg("gcode")) {
    gcodeSaveName = server.arg("name");
    gcodeSaveValue = server.arg("gcode");
    if (saveGcode()) {
      server.send(200, "text/plain", "G-code saved successfully");
    } else {
      server.send(500, "text/plain", "Failed to save G-code");
    }
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void handleGcodeList() {
  String response = "";
  File root = LittleFS.open("/");
  if (root && root.isDirectory()) {
    File file = root.openNextFile();
    while (file) {
      String filename = file.name();
      if (filename.endsWith(".gcode")) {
        response += filename.substring(0, filename.length() - 6) + "\n";
      }
      file.close();
      file = root.openNextFile();
    }
  }
  server.send(200, "text/plain", response);
}

void handleGcodeGet() {
  if (server.hasArg("name")) {
    String gcode = readGcodeProgram(server.arg("name"));
    if (gcode != "") {
      server.send(200, "text/plain", gcode);
    } else {
      server.send(404, "text/plain", "G-code file not found");
    }
  } else {
    server.send(400, "text/plain", "Missing parameter: name");
  }
}

void handleGcodeRemove() {
  if (server.hasArg("name")) {
    if (removeGcodeByName(server.arg("name"))) {
      server.send(200, "text/plain", "G-code removed successfully");
    } else {
      server.send(500, "text/plain", "Failed to remove G-code");
    }
  } else {
    server.send(400, "text/plain", "Missing parameter: name");
  }
}

void handleStatus() {
  server.send(200, "text/plain",
    "LittleFS.freeSpace=" + String(LittleFS.totalBytes() - LittleFS.usedBytes()) + "\n" +
    "X.disabled=" + String(x.disabled ? 1 : 0) + "\n" +
    "X.enableLevel=" + String(digitalRead(x.ena)) + "\n");
}

void setWiFiStatus(const String& status) {
  wifiStatus = status;
  wifiStatusMillis = millis();
}

void handleGcodePage() {
  server.send(200, "text/html", gcodehtml);
}

void handleOtaResult() {
  server.send(200, "text/plain", Update.hasError() ? "FAIL\n" : "OK\n");
  delay(500);
  ESP.restart();
}

void handleOtaUpload() {
  if (isOn) {
    server.send(503, "text/plain", "Stop the machine first\n");
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("OTA: %s (%u bytes)\n", upload.filename.c_str(), upload.totalSize);
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("OTA: success, %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

String jsonEscape(const String &s) {
  String out;
  out.reserve(s.length());
  for (int i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else out += c;
  }
  return out;
}

void broadcastStatus() {
  // Build a hash to avoid broadcasting identical frames
  long h = 0;
  for (int i = 0; i < ds_status.length(); i++) h += ds_status.charAt(i);
  for (int i = 0; i < ds_mode.length(); i++) h += ds_mode.charAt(i) * 3;
  for (int i = 0; i < ds_pitch.length(); i++) h += ds_pitch.charAt(i) * 7;
  for (int i = 0; i < ds_z.length(); i++) h += ds_z.charAt(i) * 11;
  for (int i = 0; i < ds_x.length(); i++) h += ds_x.charAt(i) * 13;
  for (int i = 0; i < ds_t3.length(); i++) h += ds_t3.charAt(i) * 17;
  h += ds_rpm.toInt() + ds_measure.length() * 19 + emergencyStop * 23;
  for (int i = 0; i < ds_step.length(); i++) h += ds_step.charAt(i) * 29;
  for (int i = 0; i < ds_zl.length(); i++) h += ds_zl.charAt(i) * 31;
  for (int i = 0; i < ds_zr.length(); i++) h += ds_zr.charAt(i) * 37;
  for (int i = 0; i < ds_xu.length(); i++) h += ds_xu.charAt(i) * 41;
  for (int i = 0; i < ds_xd.length(); i++) h += ds_xd.charAt(i) * 43;
  for (int i = 0; i < ds_turns.length(); i++) h += ds_turns.charAt(i) * 47;
  for (int i = 0; i < ds_angle.length(); i++) h += ds_angle.charAt(i) * 53;
  h += z.disabled * 59 + x.disabled * 61;

  if (h == ds_hash) return;
  ds_hash = h;

  String json = "{\"s\":\"" + ds_status +
    "\",\"m\":\"" + ds_mode +
    "\",\"p\":\"" + ds_pitch +
    "\",\"u\":\"" + ds_measure +
    "\",\"st\":\"" + ds_step +
    "\",\"rpm\":\"" + ds_rpm +
    "\",\"trn\":\"" + ds_turns +
    "\",\"ang\":\"" + ds_angle +
    "\",\"z\":\"" + ds_z +
    "\",\"zl\":\"" + ds_zl +
    "\",\"zr\":\"" + ds_zr +
    "\",\"x\":\"" + ds_x +
    "\",\"xu\":\"" + ds_xu +
    "\",\"xd\":\"" + ds_xd +
    "\",\"t3\":\"" + jsonEscape(ds_t3) +
    "\",\"zd\":" + String(z.disabled ? 1 : 0) +
    ",\"xd\":" + String(x.disabled ? 1 : 0) +
    ",\"est\":" + String(emergencyStop) + "}";
  webSocket.broadcastTXT(json);
}

void taskWiFi(void *param) {
  setWiFiStatus("Starting AP");
  const IPAddress apIP(192, 168, 100, 1);
  const IPAddress apSubnet(255, 255, 255, 0);
  if (!WiFi.mode(WIFI_AP) ||
      !WiFi.softAPConfig(apIP, apIP, apSubnet) ||
      !WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    setWiFiStatus("AP failed");
    vTaskDelete(NULL);
    return;
  }
  setWiFiStatus("See " + WiFi.softAPIP().toString());

  initBuffer(&inBuffer, 1024);
  initBuffer(&outBuffer, 1024);

  server.on("/", handleClientRequests);
  server.on("/gcode", handleGcodePage);
  server.on("/gcode/add", HTTP_POST, handleGcodeAdd);
  server.on("/gcode/list", HTTP_GET, handleGcodeList);
  server.on("/gcode/get", HTTP_GET, handleGcodeGet);
  server.on("/gcode/remove", HTTP_POST, handleGcodeRemove);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/update", HTTP_POST, handleOtaResult, handleOtaUpload);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(handleWebSocketEvent);


  unsigned long lastBroadcast = 0;

  while (emergencyStop == ESTOP_NONE) {
    server.handleClient();
    webSocket.loop();

    if (bufferAvailable(&outBuffer)) {
      String outData = "";
      while (bufferAvailable(&outBuffer)) {
        outData += shiftBuffer(&outBuffer);
      }
      webSocket.broadcastTXT(outData);
    }

    // Broadcast JSON status to all WebSocket clients every 100ms
    unsigned long now = millis();
    if (now - lastBroadcast >= 100) {
      lastBroadcast = now;
      broadcastStatus();
    }

    taskYIELD();
  }
  vTaskDelete(NULL);
}

void setAsyncTimerEnable(bool value) {
  if (value) {
    timerStart(async_timer);
  } else {
    timerStop(async_timer);
  }
}

void setEmergencyStop(int kind) {
  emergencyStop = kind;
  setAsyncTimerEnable(false);
  xSemaphoreTake(z.mutex, 10);
  xSemaphoreTake(x.mutex, 10);
  xSemaphoreTake(y.mutex, 10);
}

void updateEnable(Axis* a) {
  if (!a->disabled && (!a->needsRest || a->stepperEnableCounter > 0)) {
    digitalWrite(a->ena, a->invertEnable ? LOW : HIGH);
    // Stepper driver needs some time before it will react to pulses.
    DELAY(STEPPED_ENABLE_DELAY_MS);
  } else {
    digitalWrite(a->ena, a->invertEnable ? HIGH : LOW);
  }
}

void stepperEnable(Axis* a, bool value) {
  if (!a->needsRest || !a->active) {
    return;
  }
  if (value) {
    a->stepperEnableCounter++;
    if (value == 1) {
      updateEnable(a);
    }
  } else if (a->stepperEnableCounter > 0) {
    a->stepperEnableCounter--;
    if (a->stepperEnableCounter == 0) {
      updateEnable(a);
    }
  }
}

void markAxisOrigin(Axis* a) {
  bool hasSemaphore = xSemaphoreTake(a->mutex, 10) == pdTRUE;
  if (!hasSemaphore) {
    beepFlag = true;
  }
  if (a->leftStop != LONG_MAX) {
    a->leftStop -= a->pos;
  }
  if (a->rightStop != LONG_MIN) {
    a->rightStop -= a->pos;
  }
  a->motorPos -= a->pos;
  a->originPos += a->pos;
  a->pos = 0;
  a->fractionalPos = 0;
  a->pendingPos = 0;
  if (hasSemaphore) {
    xSemaphoreGive(a->mutex);
  }
}

void zeroSpindlePos() {
  spindlePos = 0;
  spindlePosAvg = 0;
  spindlePosSync = 0;
}

// Loose the thread and mark current physical positions of
// encoder and stepper as a new 0. To be called when dupr changes
// or ELS is turned on/off. Without this, changing dupr will
// result in stepper rushing across the lathe to the new position.
// Must be called while holding motionMutex.
void markOrigin() {
  markAxisOrigin(&z);
  markAxisOrigin(&x);
  markAxisOrigin(&y);
  zeroSpindlePos();
}

void markAxis0(Axis* a) {
  a->originPos = -a->pos;
}

void setIsOnFromTask(bool on) {
  nextIsOn = on;
  nextIsOnFlag = true;
}

void setIsOnFromLoop(bool on) {
  if (isOn && on) {
    return;
  }
  if (!on) {
    isOn = false;
    setupIndex = 0;
  }
  stepperEnable(&z, on);
  stepperEnable(&x, on);
  stepperEnable(&y, on);
  markOrigin();
  if (on) {
    isOn = true;
    opDuprSign = dupr >= 0 ? 1 : -1;
    opDupr = dupr;
    opIndex = 0;
    opIndexAdvanceFlag = false;
    opSubIndex = 0;
    setupIndex = 0;
  }
}

int getApproxRpm() {
  unsigned long t = micros();
  unsigned long elapsedTime = t - spindleEncTime;
  if (elapsedTime > 50000) {
    // RPM less than 10.
    spindleEncTimeDiffBulk = 0;
    return 0;
  }
  int rpm = 0;
  if (spindleEncTimeDiffBulk > 0) {
    rpm = 60000000 / spindleEncTimeDiffBulk;
    if (abs(rpm - shownRpm) > (rpm < 1000 ? 3 : 5)) {
      // Don't update RPM with insignificant differences.
      shownRpm = rpm;
      shownRpmTime = t;
    }
  }
  return rpm;
}

bool stepperIsRunning(Axis* a) {
  return micros() - a->stepStartUs < 50000;
}

void toScreen(const String &command) {
  // Nextion removed — no-op
}

void setText(const String &id, const String &text) {
  if (id == "bStatus") ds_status = text;
  else if (id == "bMode") ds_mode = text;
  else if (id == "tPitch") ds_pitch = text;
  else if (id == "bMeasure") ds_measure = text;
  else if (id == "tStepVal") ds_step = text;
  else if (id == "tRPMVal") ds_rpm = text;
  else if (id == "tTurnsVal") ds_turns = text;
  else if (id == "tAngleVal") {
    // Strip char(223) degree symbol — it's 0xDF which is invalid standalone UTF-8
    // and causes browsers to close WebSocket connections
    ds_angle = "";
    for (int i = 0; i < text.length(); i++) {
      if ((unsigned char)text.charAt(i) < 128) ds_angle += text.charAt(i);
    }
  }
  else if (id == "tZ") ds_z = text;
  else if (id == "tZLeft") ds_zl = text;
  else if (id == "tZRight") ds_zr = text;
  else if (id == "tX") ds_x = text;
  else if (id == "tXUp") ds_xu = text;
  else if (id == "tXDown") ds_xd = text;
  else if (id == "tY") ds_y = text;
  else if (id == "tYUp") ds_yu = text;
  else if (id == "tYDown") ds_yd = text;
  else if (id == "t3") ds_t3 = text;
}

// Returns number of letters printed.
String printDeciMicrons(long deciMicrons, int precisionPointsMax) {
  if (deciMicrons == 0) {
    return "0";
  }
  bool imperial = measure != MEASURE_METRIC;
  long v = imperial ? round(deciMicrons / 25.4) : deciMicrons;
  int points = 0;
  if (v == 0 && precisionPointsMax >= 5) {
    points = 5;
  } else if ((v % 10) != 0 && precisionPointsMax >= 4) {
    points = 4;
  } else if ((v % 100) != 0 && precisionPointsMax >= 3) {
    points = 3;
  } else if ((v % 1000) != 0 && precisionPointsMax >= 2) {
    points = 2;
  } else if ((v % 10000) != 0 && precisionPointsMax >= 1) {
    points = 1;
  }
  return String(deciMicrons / (imperial ? 254000.0 : 10000.0), points);
}

String printDegrees(long degrees10000) {
  int points = 0;
  if ((degrees10000 % 100) != 0) {
    points = 3;
  } else if ((degrees10000 % 1000) != 0) {
    points = 2;
  } else if ((degrees10000 % 10000) != 0) {
    points = 1;
  }
  return String(degrees10000 / 10000.0, points) + char(223); // degree symbol
}

String printDupr(long value) {
  if (measure != MEASURE_TPI) return printDeciMicrons(value, 5);

  float tpi = 254000.0 / value;
  String result = "";
  if (abs(tpi - round(tpi)) < TPI_ROUND_EPSILON) {
    result = String(int(round(tpi)));
  } else {
    int tpi100 = round(tpi * 100);
    int points = 0;
    if ((tpi100 % 10) != 0) {
      points = 2;
    } else if ((tpi100 % 100) != 0) {
      points = 1;
    }
    result = String(tpi, points);
  }
  return result;
}

long stepsToDu(Axis* a, long steps) {
  return round(steps * a->screwPitch / a->motorSteps);
}

long getAxisPosDu(Axis* a) {
  return stepsToDu(a, a->pos + a->originPos);
}

long getAxisStopDiffDu(Axis* a) {
  if (a->leftStop == LONG_MAX || a->rightStop == LONG_MIN) return 0;
  return stepsToDu(a, a->leftStop - a->rightStop);
}

String printAxisPos(Axis* a) {
  if (a->rotational) return printDegrees(getAxisPosDu(a));
  return printDeciMicrons(getAxisPosDu(a), 3);
}

long getAxisLeftStopDistanceDu(Axis* a) {
  return stepsToDu(a, a->leftStop - a->pos);
}

String printDistanceToLeftStop(Axis* a) {
  if (a->leftStop == LONG_MAX) return "";
  if (a->rotational) return printDegrees(getAxisLeftStopDistanceDu(a));
  return printDeciMicrons(getAxisLeftStopDistanceDu(a), 3);
}

long getAxisRightStopDistanceDu(Axis* a) {
  return stepsToDu(a, a->pos - a->rightStop);
}

String printDistanceToRightStop(Axis* a) {
  if (a->rightStop == LONG_MIN) return "";
  if (a->rotational) return printDegrees(getAxisRightStopDistanceDu(a));
  return printDeciMicrons(getAxisRightStopDistanceDu(a), 3);
}

String printAxisStopDiff(Axis* a, bool addTrailingSpace) {
  String result = "";
  if (a->rotational) {
    result = printDegrees(getAxisStopDiffDu(a));
  } else {
    result = printDeciMicrons(getAxisStopDiffDu(a), 3);
  }
  return addTrailingSpace ? result + ' ' : result;
}

String printNoTrailing0(float value) {
  long v = round(value * 100000);
  int points = 0;
  if ((v % 10) != 0) {
    points = 5;
  } else if ((v % 100) != 0) {
    points = 4;
  } else if ((v % 1000) != 0) {
    points = 3;
  } else if ((v % 10000) != 0) {
    points = 2;
  } else if ((v % 100000) != 0) {
    points = 1;
  }
  return String(value, points);
}

bool needZStops() {
  return mode == MODE_TURN || mode == MODE_FACE || mode == MODE_THREAD || mode == MODE_ELLIPSE;
}

bool isPassMode() {
  return mode == MODE_TURN || mode == MODE_FACE || mode == MODE_CUT || mode == MODE_THREAD || mode == MODE_ELLIPSE;
}

bool manualMovesAllowedWhenOn() {
  return mode == MODE_NORMAL || mode == MODE_ASYNC || mode == MODE_CONE || mode == MODE_Y;
}

bool manualMovesIgnoredWhenOn() {
  return mode == MODE_GCODE || isPassMode();
}

int getLastSetupIndex() {
  if (mode == MODE_CONE || mode == MODE_GCODE) return 2;
  if (mode == MODE_THREAD) return 4;
  if (mode == MODE_TURN || mode == MODE_FACE || mode == MODE_CUT || mode == MODE_ELLIPSE) return 3;
  return 0;
}

Axis* getPitchAxis() {
  return mode == MODE_FACE ? &x : &z;
}

long getPassModeZStart() {
  if (mode == MODE_TURN || mode == MODE_THREAD) return dupr > 0 ? z.rightStop : z.leftStop;
  if (mode == MODE_FACE) return auxForward ? z.rightStop : z.leftStop;
  if (mode == MODE_ELLIPSE) return dupr > 0 ? z.leftStop : z.rightStop;
  return z.pos;
}

long getPassModeXStart() {
  if (mode == MODE_TURN || mode == MODE_THREAD) return auxForward ? x.rightStop : x.leftStop;
  if (mode == MODE_FACE || mode == MODE_CUT) return dupr > 0 ? x.rightStop : x.leftStop;
  if (mode == MODE_ELLIPSE) return x.rightStop;
  return x.pos;
}

long getNumpadResult() {
  long result = 0;
  for (int i = 0; i < numpadIndex; i++) {
    result += numpadDigits[i] * pow(10, numpadIndex - 1 - i);
  }
  return result;
}

float numpadToConeRatio() {
  return getNumpadResult() / 100000.0;
}

long numpadToDeciMicrons() {
  long result = getNumpadResult();
  if (result == 0) {
    return 0;
  }
  if (measure == MEASURE_INCH) {
    result = result * 254;
  } else if (measure == MEASURE_TPI) {
    result = round(254000.0 / result);
  } else { // Metric
    result = result * 10;
  }
  return result;
}

long spindleModulo(long value) {
  value = value % ENCODER_STEPS_INT;
  if (value < 0) {
    value += ENCODER_STEPS_INT;
  }
  return value;
}

bool removeAllGcode() {
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    writeBuffer(&outBuffer, "error: failed to open directory\n");
    return false;
  }

  File file = root.openNextFile();
  while (file) {
    String path = file.path();
    file.close(); // can't remove while open
    if (path.endsWith(".gcode")) {
      if (!LittleFS.remove(path)) {
        writeBuffer(&outBuffer, "error: failed to delete " + path + "\n");
      }
    }
    file = root.openNextFile();
  }

  gcodeProgramCount = getGcodeProgramCount();
  return true;
}

String getCurrentGcodeProgramName() {
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    writeBuffer(&outBuffer, "error: failed to open directory\n");
    return "";
  }

  int count = 0;
  File file = root.openNextFile();
  while (file) {
    String filename = file.name();
    file.close();
    if (filename.endsWith(".gcode")) {
      if (count == gcodeProgramIndex) {
        return filename.substring(0, filename.length() - 6);
      }
      count++;
    }
    file = root.openNextFile();
  }

  return "";
}

String printMode() {
  if (mode == MODE_NORMAL) return "GEAR";
  if (mode == MODE_ASYNC) return "ASYNC";
  if (mode == MODE_CONE) return "CONE";
  if (mode == MODE_TURN) return "TURN";
  if (mode == MODE_FACE) return "FACE";
  if (mode == MODE_CUT) return "CUT";
  if (mode == MODE_THREAD) return "THREAD";
  if (mode == MODE_ELLIPSE) return "ELLIP";
  if (mode == MODE_GCODE) return "GCODE";
  if (mode == MODE_Y) return "Y";
  return "";
}

unsigned long lastDisplayUpdateTime = 0;

void updateDisplay() {
  if (millis() - lastDisplayUpdateTime < 100) return;
  lastDisplayUpdateTime = millis();

  long newHashLine0 = isOn + spindlePosSync + mode + measure + dupr + starts;
  if (lcdHashLine0 != newHashLine0) {
    lcdHashLine0 = newHashLine0;
    if (spindlePosSync) setText("bStatus", "SYN");
    else setText("bStatus", isOn ? "ON" : "OFF");
    setText("bMode", printMode());
    String bPitchText = printDupr(dupr);
    if (starts != 1) bPitchText += " x" + String(starts);
    setText("tPitch", bPitchText);
    setText("bMeasure", measure == MEASURE_INCH ? "IN" : measure == MEASURE_METRIC ? "MM" : "TPI");
  }

  int rpm = getApproxRpm();
  long newHashLine1 = moveStep + rpm + spindlePos + measure;
  if (lcdHashLine1 != newHashLine1) {
    lcdHashLine1 = newHashLine1;
    setText("tStepVal", printDeciMicrons(moveStep, 5));
    setText("tRPMVal", String(rpm));
    float turns = (float) abs(spindlePos) / ENCODER_STEPS_INT;
    setText("tTurnsVal", String(turns, turns < 100 ? 2 : (turns < 1000 ? 1 : 0)));
    setText("tAngleVal", String(spindleModulo(spindlePos) * 360 / ENCODER_STEPS_FLOAT, 2) + String(char(223)));
  }

  long newHashLine2 =
    x.pos + x.originPos + x.disabled + x.leftStop - x.rightStop +
    z.pos + z.originPos + z.disabled + z.leftStop - z.rightStop +
    y.pos + y.originPos + y.disabled + y.leftStop - y.rightStop + measure + x.pos % 100;
  if (lcdHashLine2 != newHashLine2) {
    lcdHashLine2 = newHashLine2;
    setText("tX", !x.active || x.disabled ? "" : printAxisPos(&x));
    setText("tXUp", !x.active || x.disabled ? "" : printDistanceToLeftStop(&x));
    setText("tXDown", !x.active || x.disabled ? "" : printDistanceToRightStop(&x));
    setText("tY", !y.active || y.disabled ? "" : printAxisPos(&y));
    setText("tYUp", !y.active || y.disabled ? "" : printDistanceToLeftStop(&y));
    setText("tYDown", !y.active || y.disabled ? "" : printDistanceToRightStop(&y));
    setText("tZ", !z.active || z.disabled ? "" : printAxisPos(&z));
    setText("tZLeft", !z.active || z.disabled ? "" : printDistanceToLeftStop(&z));
    setText("tZRight", !z.active || z.disabled ? "" : printDistanceToRightStop(&z));
  }

  long numpadResult = getNumpadResult();
  long gcodeCommandHash = 0;
  for (int i = 0; i < gcodeCommand.length(); i++) {
    gcodeCommandHash += gcodeCommand.charAt(i);
  }
  for (int i = 0; i < wifiStatus.length(); i++) {
    gcodeCommandHash += wifiStatus.charAt(i);
  }
  bool spindleStopped = micros() > spindleEncTime + 100000;
  long newHashLine3 = z.pos + (showAngle ? spindlePos : -1) + (showTacho ? rpm : -2) + measure + (numpadResult > 0 ? numpadResult : -1) + mode * 5 + dupr +
      (mode == MODE_CONE ? round(coneRatio * 10000) : 0) + turnPasses + opIndex + setupIndex + gcodeProgramIndex + gcodeProgramCount + spindleStopped * 3 + (isOn ? 139 : -117) + (inNumpad ? 10 : 0) + (auxForward ? 17 : -31) +
      (z.leftStop == LONG_MAX ? 123 : z.leftStop) + (z.rightStop == LONG_MIN ? 1234 : z.rightStop) +
      (x.leftStop == LONG_MAX ? 1235 : x.leftStop) + (x.rightStop == LONG_MIN ? 123456 : x.rightStop) + gcodeCommandHash +
      (mode == MODE_Y ? y.pos + y.originPos + (y.leftStop == LONG_MAX ? 123 : y.leftStop) + (y.rightStop == LONG_MIN ? 1234 : y.rightStop) + y.disabled : 0) + x.pos + x.originPos + z.pos;
  if (lcdHashLine3 != newHashLine3) {
    lcdHashLine3 = newHashLine3;
    String result = "";
    if (mode == MODE_GCODE) {
      if (setupIndex == 1 && gcodeProgramCount == 0) {
        result = "No stored programs";
      } else if (setupIndex == 1) {
        if (gcodeProgramIndex >= gcodeProgramCount) {
          result = "Program deleted";
        } else {
          result = getCurrentGcodeProgramName();
        }
      } else if (setupIndex == 2) {
        if (spindleStopped) result = "Turn on the spindle!";
        else result = "Spindle on. Go?";
      } else if (isOn) {
        result = gcodeCommand.substring(0, 20);
      }
    } else if (isPassMode()) {
      bool missingZStops = needZStops() && (z.leftStop == LONG_MAX || z.rightStop == LONG_MIN);
      bool missingStops = missingZStops || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN;
      if (!inNumpad && missingStops) {
        result = needZStops() ? "Set all stops" : "Set X stops";
      } else if (numpadResult != 0 && setupIndex == 1) {
        long passes = min(PASSES_MAX, numpadResult);
        result = String(passes);
        if (passes == 1) result += " pass?";
        else result += " passes?";
      } else if (!isOn && setupIndex == 1) {
        result = String(turnPasses);
        if (turnPasses == 1) result += " pass?";
        else result += " passes?";
      } else if (!isOn && setupIndex == 2) {
        if (mode == MODE_FACE) {
          result = auxForward ? "Right to left?" : "Left to right?";
        } else if (mode == MODE_CUT) {
          result = dupr >= 0 ? "Pitch > 0, external" : "Pitch < 0, internal";
        } else {
          result = auxForward ? "External?" : "Internal?";
        }
      } else if (mode == MODE_THREAD && !isOn && setupIndex == 3) {
        result = "Cone ratio " + String(numpadToConeRatio(), 5) + "?";
      } else if (!isOn && setupIndex == getLastSetupIndex()) {
        long zOffset = getPassModeZStart() - z.pos;
        long xOffset = getPassModeXStart() - x.pos;
        result = "Go";
        if (zOffset != 0) {
          result += " ";
          result += z.name;
          result += printDeciMicrons(stepsToDu(&z, zOffset), 2);
        }
        if (xOffset != 0) {
          result += " ";
          result += x.name;
          result += printDeciMicrons(stepsToDu(&x, xOffset), 2);
        }
        result += "?";
      } else if (isOn && numpadResult == 0) {
        result = "Pass " + String(opIndex) + " of " + String(max(opIndex, long(turnPasses * starts)));
      }
    } else if (mode == MODE_CONE) {
      if (numpadResult != 0 && setupIndex == 1) result = "Use ratio " + String(numpadToConeRatio(), 5) + "?";
      else if (!isOn && setupIndex == 1) result = "Use ratio " + printNoTrailing0(coneRatio) + "?";
      else if (!isOn && setupIndex == 2) result = auxForward ? "External?" : "Internal?";
      else if (!isOn && setupIndex == 3) result = "Go?";
      else if (isOn && numpadResult == 0) result = "Cone ratio " + printNoTrailing0(coneRatio);
    }

    if (inNumpad && result == "") result = "Use " + printDupr(numpadToDeciMicrons()) + "?";

    if (result == "" && (millis() - wifiStatusMillis < 7000 || !x.active || x.disabled)) result = wifiStatus;

    if (result == "" && x.active && !x.disabled) result = "Diameter " + printDeciMicrons(abs(2 * getAxisPosDu(&x)), 2);

    setText("t3", result);
  }
}

bool saveIfChanged() {
  // Should avoid calling Preferences whenever possible to reduce memory wear and avoid ~20ms write delay that blocks interrupts.
  if (dupr == savedDupr && starts == savedStarts && z.pos == z.savedPos && z.originPos == z.savedOriginPos && z.posGlobal == z.savedPosGlobal && z.motorPos == z.savedMotorPos && z.leftStop == z.savedLeftStop && z.rightStop == z.savedRightStop && z.disabled == z.savedDisabled &&
      spindlePos == savedSpindlePos && spindlePosAvg == savedSpindlePosAvg && spindlePosSync == savedSpindlePosSync && savedSpindlePosGlobal == spindlePosGlobal && showAngle == savedShowAngle && showTacho == savedShowTacho && moveStep == savedMoveStep &&
      mode == savedMode && measure == savedMeasure && x.pos == x.savedPos && x.originPos == x.savedOriginPos && x.posGlobal == x.savedPosGlobal && x.motorPos == x.savedMotorPos && x.leftStop == x.savedLeftStop && x.rightStop == x.savedRightStop && x.disabled == x.savedDisabled &&
      y.pos == y.savedPos && y.originPos == y.savedOriginPos && y.posGlobal == y.savedPosGlobal && y.motorPos == y.savedMotorPos && y.leftStop == y.savedLeftStop && y.rightStop == y.savedRightStop && y.disabled == y.savedDisabled &&
      coneRatio == savedConeRatio && turnPasses == savedTurnPasses && savedAuxForward == auxForward) return false;

  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  if (dupr != savedDupr) pref.putLong(PREF_DUPR, savedDupr = dupr);
  if (starts != savedStarts) pref.putInt(PREF_STARTS, savedStarts = starts);
  if (z.pos != z.savedPos) pref.putLong(PREF_POS_Z, z.savedPos = z.pos);
  if (z.posGlobal != z.savedPosGlobal) pref.putLong(PREF_POS_GLOBAL_Z, z.savedPosGlobal = z.posGlobal);
  if (z.originPos != z.savedOriginPos) pref.putLong(PREF_ORIGIN_POS_Z, z.savedOriginPos = z.originPos);
  if (z.motorPos != z.savedMotorPos) pref.putLong(PREF_MOTOR_POS_Z, z.savedMotorPos = z.motorPos);
  if (z.leftStop != z.savedLeftStop) pref.putLong(PREF_LEFT_STOP_Z, z.savedLeftStop = z.leftStop);
  if (z.rightStop != z.savedRightStop) pref.putLong(PREF_RIGHT_STOP_Z, z.savedRightStop = z.rightStop);
  if (z.disabled != z.savedDisabled) pref.putBool(PREF_DISABLED_Z, z.savedDisabled = z.disabled);
  if (spindlePos != savedSpindlePos) pref.putLong(PREF_SPINDLE_POS, savedSpindlePos = spindlePos);
  if (spindlePosAvg != savedSpindlePosAvg) pref.putLong(PREF_SPINDLE_POS_AVG, savedSpindlePosAvg = spindlePosAvg);
  if (spindlePosSync != savedSpindlePosSync) pref.putInt(PREF_OUT_OF_SYNC, savedSpindlePosSync = spindlePosSync);
  if (spindlePosGlobal != savedSpindlePosGlobal) pref.putLong(PREF_SPINDLE_POS_GLOBAL, savedSpindlePosGlobal = spindlePosGlobal);
  if (showAngle != savedShowAngle) pref.putBool(PREF_SHOW_ANGLE, savedShowAngle = showAngle);
  if (showTacho != savedShowTacho) pref.putBool(PREF_SHOW_TACHO, savedShowTacho = showTacho);
  if (moveStep != savedMoveStep) pref.putLong(PREF_MOVE_STEP, savedMoveStep = moveStep);
  if (mode != savedMode) pref.putInt(PREF_MODE, savedMode = mode);
  if (measure != savedMeasure) pref.putInt(PREF_MEASURE, savedMeasure = measure);
  if (x.pos != x.savedPos) pref.putLong(PREF_POS_X, x.savedPos = x.pos);
  if (x.posGlobal != x.savedPosGlobal) pref.putLong(PREF_POS_GLOBAL_X, x.savedPosGlobal = x.posGlobal);
  if (x.originPos != x.savedOriginPos) pref.putLong(PREF_ORIGIN_POS_X, x.savedOriginPos = x.originPos);
  if (x.motorPos != x.savedMotorPos) pref.putLong(PREF_MOTOR_POS_X, x.savedMotorPos = x.motorPos);
  if (x.leftStop != x.savedLeftStop) pref.putLong(PREF_LEFT_STOP_X, x.savedLeftStop = x.leftStop);
  if (x.rightStop != x.savedRightStop) pref.putLong(PREF_RIGHT_STOP_X, x.savedRightStop = x.rightStop);
  if (x.disabled != x.savedDisabled) pref.putBool(PREF_DISABLED_X, x.savedDisabled = x.disabled);
  if (y.pos != y.savedPos) pref.putLong(PREF_POS_Y, y.savedPos = y.pos);
  if (y.posGlobal != y.savedPosGlobal) pref.putLong(PREF_POS_GLOBAL_Y, y.savedPosGlobal = y.posGlobal);
  if (y.originPos != y.savedOriginPos) pref.putLong(PREF_ORIGIN_POS_Y, y.savedOriginPos = y.originPos);
  if (y.motorPos != y.savedMotorPos) pref.putLong(PREF_MOTOR_POS_Y, y.savedMotorPos = y.motorPos);
  if (y.leftStop != y.savedLeftStop) pref.putLong(PREF_LEFT_STOP_Y, y.savedLeftStop = y.leftStop);
  if (y.rightStop != y.savedRightStop) pref.putLong(PREF_RIGHT_STOP_Y, y.savedRightStop = y.rightStop);
  if (y.disabled != y.savedDisabled) pref.putBool(PREF_DISABLED_Y, y.savedDisabled = y.disabled);
  if (coneRatio != savedConeRatio) pref.putFloat(PREF_CONE_RATIO, savedConeRatio = coneRatio);
  if (turnPasses != savedTurnPasses) pref.putInt(PREF_TURN_PASSES, savedTurnPasses = turnPasses);
  if (auxForward != savedAuxForward) pref.putBool(PREF_AUX_FORWARD, savedAuxForward = auxForward);
  pref.end();
  return true;
}

void beep() {
  // TODO
}

void taskDisplay(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    updateDisplay();
    // Calling Preferences.commit() blocks all interrupts for 30ms, don't call saveIfChanged() if
    // encoder is likely to move soon.
    unsigned long now = micros();
    if (!stepperIsRunning(&z) && !stepperIsRunning(&x) && (now > spindleEncTime + SAVE_DELAY_US) && (now < saveTime || now > saveTime + SAVE_DELAY_US) && (now < keypadTimeUs || now > keypadTimeUs + SAVE_DELAY_US)) {
      if (saveIfChanged()) {
        saveTime = now;
      }
    }
    if (beepFlag) {
      beepFlag = false;
      beep();
    }
    if (abs(z.pendingPos) > z.estopSteps || abs(x.pendingPos) > x.estopSteps) {
      setEmergencyStop(ESTOP_POS);
    }
    taskYIELD();
  }
  setText("bMode", "ESTOP");
  if (emergencyStop == ESTOP_POS) {
    setText("t3", "Requested position outside machine");
  } else if (emergencyStop == ESTOP_MARK_ORIGIN) {
    setText("t3", "Unable to mark origin");
  } else if (emergencyStop == ESTOP_ON_OFF) {
    setText("t3", "Unable to turn on/off");
  } else if (emergencyStop == ESTOP_OFF_MANUAL_MOVE) {
    setText("t3", "Off during manual move");
  }
  vTaskDelete(NULL);
}

void setMeasure(int value) {
  if (measure == value) {
    return;
  }
  measure = value;
  moveStep = measure == MEASURE_METRIC ? MOVE_STEP_1 : MOVE_STEP_IMP_1;
}

void waitForPendingPosNear0(Axis* a) {
  while (abs(a->pendingPos) > a->motorSteps / 3) {
    taskYIELD();
  }
}

void waitForPendingPos0(Axis* a) {
  while (a->pendingPos != 0) {
    taskYIELD();
  }
}

bool isContinuousStep() {
  return moveStep == (measure == MEASURE_METRIC ? MOVE_STEP_1 : MOVE_STEP_IMP_1);
}

// For rotational axis the moveStep of 0.1" means 0.1°.
long getMoveStepForAxis(Axis* a) {
  return (a->rotational && measure != MEASURE_METRIC) ? (moveStep / 25.4) : moveStep;
}

long getStepMaxSpeed(Axis* a) {
  return isContinuousStep() ? a->speedManualMove : min(long(a->speedManualMove), abs(getMoveStepForAxis(a)) * 1000 / STEP_TIME_MS);
}

void waitForStep(Axis* a) {
  if (isContinuousStep()) {
    // Move continuously for default step.
    waitForPendingPosNear0(a);
  } else {
    // Move with tiny pauses allowing to stop precisely.
    a->continuous = false;
    waitForPendingPos0(a);
    DELAY(DELAY_BETWEEN_STEPS_MS);
  }
}

int getAndResetPulses(Axis* a) {
  int16_t count;
  pcnt_get_counter_value(a->pulseUnit, &count);
  int delta = count - a->pulseCount;
  if (delta == 0) return 0;
  if (isOn && manualMovesIgnoredWhenOn()) {
    pcnt_counter_clear(a->pulseUnit);
    a->pulseCount = 0;
    return 0;
  }
  if (count >= PCNT_CLEAR || count <= -PCNT_CLEAR) {
    pcnt_counter_clear(a->pulseUnit);
    a->pulseCount = 0;
  } else {
    a->pulseCount = count;
  }
  return delta;
}

// Calculates stepper position from spindle position.
long posFromSpindle(Axis* a, long s, bool respectStops) {
  long newPos = s * a->motorSteps / a->screwPitch / ENCODER_STEPS_FLOAT * dupr * starts;

  // Respect left/right stops.
  if (respectStops) {
    if (newPos < a->rightStop) {
      newPos = a->rightStop;
    } else if (newPos > a->leftStop) {
      newPos = a->leftStop;
    }
  }

  return newPos;
}

// Calculates spindle position from stepper position.
long spindleFromPos(Axis* a, long p) {
  return p * a->screwPitch * ENCODER_STEPS_FLOAT / a->motorSteps / (dupr * starts);
}

bool stepTo(Axis* a, long newPos, bool continuous) {
  if (xSemaphoreTake(a->mutex, 10) == pdTRUE) {
    a->continuous = continuous;
    if (newPos == a->pos) {
      a->pendingPos = 0;
    } else {
      a->pendingPos = newPos - a->motorPos - (newPos > a->pos ? 0 : a->backlashSteps);
    }
    xSemaphoreGive(a->mutex);
    return true;
  }
  return false;
}

// Moves the stepper so that the tool is located at the newPos.
bool stepToContinuous(Axis* a, long newPos) {
  return stepTo(a, newPos, true);
}

bool stepToFinal(Axis* a, long newPos) {
  return stepTo(a, newPos, false);
}

void setDir(Axis* a, bool dir) {
  // Start slow if direction changed.
  if (a->direction != dir || !a->directionInitialized) {
    a->speed = a->speedStart;
    a->direction = dir;
    a->directionInitialized = true;
    digitalWrite(a->dir, dir ^ a->invertStepper);
    delayMicroseconds(DIRECTION_SETUP_DELAY_US);
  }
}

Axis* getAsyncAxis() {
  return mode == MODE_Y ? &y : &z;
}

unsigned int getTimerLimit() {
  if (dupr == 0) {
    return 65535;
  }
  return min(long(65535), long(TIMER_FREQ / (z.motorSteps * abs(dupr) / z.screwPitch)) - 1);
}

void updateAsyncTimerSettings() {
  // dupr and therefore direction can change while we're in async mode.
  setDir(getAsyncAxis(), dupr > 0);

  // dupr can change while we're in async mode, keep updating timer frequency.
  timerAlarm(async_timer, getTimerLimit(), true, 0);
  // without this timer stops working if already above new limit
  timerWrite(async_timer, 0);
}

void taskMoveZ(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    int pulseDelta = getAndResetPulses(&z);
    bool left = buttonLeftPressed;
    bool right = buttonRightPressed;
    if (!left && !right && pulseDelta == 0) {
      taskYIELD();
      continue;
    }
    if (spindlePosSync != 0) {
      // Edge case.
      taskYIELD();
      continue;
    }
    if (isOn && !manualMovesAllowedWhenOn()) {
      setIsOnFromTask(false);
      taskYIELD();
      continue;
    }
    int sign = pulseDelta == 0 ? (left ? 1 : -1) : (pulseDelta > 0 ? 1 : -1);
    bool stepperOn = true;
    stepperEnable(&z, true);
    z.movingManually = true;
    if (isOn && dupr != 0 && mode == MODE_NORMAL) {
      // Move by moveStep in the desired direction but stay in the thread by possibly traveling a little more.
      float fraction = pulseDelta == 0 ? 1.0 : abs(pulseDelta) / PULSE_PER_REVOLUTION;
      float turns = moveStep * fraction / abs(dupr * starts);
      int fullTurns = ceil(turns);
      int diff = fullTurns * ENCODER_STEPS_FLOAT * sign * (dupr > 0 ? 1 : -1);
      long prevSpindlePos = spindlePos;
      bool resting = false;
      do {
        z.speedMax = z.speedManualMove;
        if (xSemaphoreTake(motionMutex, 100) == pdTRUE) {
          if (!resting) {
            spindlePos += diff;
            spindlePosAvg += diff;
          }
          // If spindle is moving, it will be changing spindlePos at the same time. Account for it.
          while (diff > 0 ? (spindlePos < prevSpindlePos) : (spindlePos > prevSpindlePos)) {
            spindlePos += diff;
            spindlePosAvg += diff;
          };
          prevSpindlePos = spindlePos;
          xSemaphoreGive(motionMutex);
        }

        long newPos = posFromSpindle(&z, prevSpindlePos, true);
        if (newPos != z.pos) {
          stepToContinuous(&z, newPos);
          waitForPendingPosNear0(&z);
          getAndResetPulses(&z); // Discard any pulses during movement.
        } else if (z.pos == (left ? z.leftStop : z.rightStop)) {
          // We're standing on a stop with the L/R move button pressed.
          resting = true;
          if (stepperOn) {
            stepperEnable(&z, false);
            stepperOn = false;
          }
          DELAY(200);
        }
      } while (left ? buttonLeftPressed : buttonRightPressed);
    } else {
      z.speedMax = getStepMaxSpeed(&z);
      int delta = 0;
      do {
        float fractionalDelta = (pulseDelta == 0 ? moveStep * sign / z.screwPitch : pulseDelta / PULSE_PER_REVOLUTION) * z.motorSteps + z.fractionalPos;
        delta = round(fractionalDelta);
        // Don't lose fractional steps when moving by 0.01" or 0.001".
        z.fractionalPos = fractionalDelta - delta;
        if (delta == 0) {
          // When moveStep is e.g. 1 micron and MOTOR_STEPS_Z is 200, make delta non-zero.
          delta = sign;
        }

        long posCopy = z.pos + z.pendingPos;
        // Don't left-right move out of stops.
        if (posCopy + delta > z.leftStop) {
          delta = z.leftStop - posCopy;
        } else if (posCopy + delta < z.rightStop) {
          delta = z.rightStop - posCopy;
        }
        z.speedMax = getStepMaxSpeed(&z);
        stepToContinuous(&z, posCopy + delta);
        waitForStep(&z);
      } while (delta != 0 && (left ? buttonLeftPressed : buttonRightPressed));
      z.continuous = false;
      waitForPendingPos0(&z);
      if (isOn && mode == MODE_CONE) {
        if (xSemaphoreTake(motionMutex, 100) != pdTRUE) {
          setEmergencyStop(ESTOP_MARK_ORIGIN);
        } else {
          markOrigin();
          xSemaphoreGive(motionMutex);
        }
      } else if (isOn && mode == MODE_ASYNC) {
        // Restore async direction.
        updateAsyncTimerSettings();
      }
    }
    z.movingManually = false;
    if (stepperOn) {
      stepperEnable(&z, false);
    }
    z.speedMax = LONG_MAX;
    taskYIELD();
  }
  vTaskDelete(NULL);
}

void taskMoveX(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    int pulseDelta = getAndResetPulses(&x);
    bool up = buttonUpPressed || pulseDelta > 0;
    bool down = buttonDownPressed || pulseDelta < 0;
    if (!up && !down) {
      taskYIELD();
      continue;
    }
    if (isOn && !manualMovesAllowedWhenOn()) {
      setIsOnFromTask(false);
      taskYIELD();
      continue;
    }
    x.movingManually = true;
    x.speedMax = getStepMaxSpeed(&x);
    stepperEnable(&x, true);

    int delta = 0;
    int sign = up ? 1 : -1;
    do {
      float fractionalDelta = (pulseDelta == 0 ? moveStep * sign / x.screwPitch : pulseDelta / PULSE_PER_REVOLUTION) * x.motorSteps + x.fractionalPos;
      delta = round(fractionalDelta);
      // Don't lose fractional steps when moving by 0.01" or 0.001".
      x.fractionalPos = fractionalDelta - delta;
      if (delta == 0) {
        // When moveStep is e.g. 1 micron and MOTOR_STEPS_Z is 200, make delta non-zero.
        delta = sign;
      }

      long posCopy = x.pos + x.pendingPos;
      if (posCopy + delta > x.leftStop) {
        delta = x.leftStop - posCopy;
      } else if (posCopy + delta < x.rightStop) {
        delta = x.rightStop - posCopy;
      }
      stepToContinuous(&x, posCopy + delta);
      waitForStep(&x);
      pulseDelta = getAndResetPulses(&x);
    } while (delta != 0 && (pulseDelta != 0 || (up ? buttonUpPressed : buttonDownPressed)));
    x.continuous = false;
    waitForPendingPos0(&x);
    if (isOn && mode == MODE_CONE) {
      if (xSemaphoreTake(motionMutex, 100) != pdTRUE) {
        setEmergencyStop(ESTOP_MARK_ORIGIN);
      } else {
        markOrigin();
        xSemaphoreGive(motionMutex);
      }
    }
    x.movingManually = false;
    x.speedMax = LONG_MAX;
    stepperEnable(&x, false);

    taskYIELD();
  }
  vTaskDelete(NULL);
}

void taskMoveY(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    bool plus = buttonForwardPressed;
    bool minus = buttonBackPressed;
    if (!plus && !minus) {
      taskYIELD();
      continue;
    }
    y.movingManually = true;
    y.speedMax = getStepMaxSpeed(&y);
    stepperEnable(&y, true);

    int delta = 0;
    int sign = plus ? 1 : -1;
    do {
      float fractionalDelta = getMoveStepForAxis(&y) * sign / y.screwPitch * y.motorSteps + y.fractionalPos;
      delta = round(fractionalDelta);
      y.fractionalPos = fractionalDelta - delta;
      if (delta == 0) delta = sign;

      long posCopy = y.pos + y.pendingPos;
      if (posCopy + delta > y.leftStop) {
        delta = y.leftStop - posCopy;
      } else if (posCopy + delta < y.rightStop) {
        delta = y.rightStop - posCopy;
      }
      stepToContinuous(&y, posCopy + delta);
      waitForStep(&y);
    } while (plus ? buttonForwardPressed : buttonBackPressed);
    y.continuous = false;
    waitForPendingPos0(&y);
    // Restore async direction.
    if (isOn && mode == MODE_Y) updateAsyncTimerSettings();
    y.movingManually = false;
    y.speedMax = LONG_MAX;
    stepperEnable(&y, false);
    taskYIELD();
  }
  vTaskDelete(NULL);
}

String getValueString(const String& command, char letter) {
  int index = command.indexOf(letter);
  if (index == -1) {
    return "";
  }
  String valueString;
  for (int i = index + 1; i < command.length(); i++) {
    char c = command.charAt(i);
    if (isDigit(c) || c == '.' || c == '-') {
      valueString += c;
    } else {
      break;
    }
  }
  return valueString;
}

float getFloat(const String& command, char letter) {
  return getValueString(command, letter).toFloat();
}

int getInt(const String& command, char letter) {
  return getValueString(command, letter).toInt();
}

void setFeedRate(const String& command) {
  float feed = getFloat(command, 'F');
  if (feed <= 0) return;
  gcodeFeedDuPerSec = round(feed * (measure == MEASURE_METRIC ? 10000 : 254000) / 60.0);
}

long mmOrInchToAbsolutePos(Axis* a, float mmOrInch) {
  long scaleToDu = measure == MEASURE_METRIC ? 10000 : 254000;
  long part1 = a->gcodeRelativePos;
  long part2 = round(mmOrInch * scaleToDu / a->screwPitch * a->motorSteps);
  return part1 + part2;
}

void updateAxisSpeeds(long diffX, long diffZ, long diffY) {
  if (diffX == 0 && diffZ == 0 && diffY == 0) return;
  long absX = abs(diffX);
  long absZ = abs(diffZ);
  long absC = abs(diffY);
  float stepsPerSecX = gcodeFeedDuPerSec * x.motorSteps / x.screwPitch;
  float minStepsPerSecX = GCODE_FEED_MIN_DU_SEC * x.motorSteps / x.screwPitch;
  if (stepsPerSecX > x.speedManualMove) stepsPerSecX = x.speedManualMove;
  else if (stepsPerSecX < minStepsPerSecX) stepsPerSecX = minStepsPerSecX;
  float stepsPerSecZ = gcodeFeedDuPerSec * z.motorSteps / z.screwPitch;
  float minStepsPerSecZ = GCODE_FEED_MIN_DU_SEC * z.motorSteps / z.screwPitch;
  if (stepsPerSecZ > z.speedManualMove) stepsPerSecZ = z.speedManualMove;
  else if (stepsPerSecZ < minStepsPerSecZ) stepsPerSecZ = minStepsPerSecZ;
  float stepsPerSecY = gcodeFeedDuPerSec * y.motorSteps / y.screwPitch;
  float minStepsPerSecY = GCODE_FEED_MIN_DU_SEC * y.motorSteps / y.screwPitch;
  if (stepsPerSecY > y.speedManualMove) stepsPerSecY = y.speedManualMove;
  else if (stepsPerSecY < minStepsPerSecY) stepsPerSecY = minStepsPerSecY;
  float secX = absX / stepsPerSecX;
  float secZ = absZ / stepsPerSecZ;
  float secY = absC / stepsPerSecY;
  float sec = ACTIVE_Y ? max(max(secX, secZ), secY) : max(secX, secZ);
  x.speedMax = sec > 0 ? absX / sec : x.speedManualMove;
  z.speedMax = sec > 0 ? absZ / sec : z.speedManualMove;
  y.speedMax = sec > 0 ? absC / sec : y.speedManualMove;
  if (x.speedMax < minStepsPerSecX) x.speedMax = minStepsPerSecX;
  if (z.speedMax < minStepsPerSecZ) z.speedMax = minStepsPerSecZ;
  if (y.speedMax < minStepsPerSecY) y.speedMax = minStepsPerSecY;
}

void gcodeWaitEpsilon(int epsilon) {
  while (abs(x.pendingPos) > epsilon || abs(z.pendingPos) > epsilon || abs(y.pendingPos) > epsilon || (SPINDLE_PAUSES_GCODE && getApproxRpm() < GCODE_MIN_RPM)) {
    taskYIELD();
  }
}

void gcodeWaitNear() {
  gcodeWaitEpsilon(GCODE_WAIT_EPSILON_STEPS);
}

void gcodeWaitStop() {
  gcodeWaitEpsilon(0);
}

// Rapid positioning / linear interpolation.
void G00_01(const String& command) {
  long xStart = x.pos;
  long zStart = z.pos;
  long yStart = y.pos;
  long xEnd = command.indexOf(x.name) >= 0 ? mmOrInchToAbsolutePos(&x, getFloat(command, x.name)) : xStart;
  long zEnd = command.indexOf(z.name) >= 0 ? mmOrInchToAbsolutePos(&z, getFloat(command, z.name)) : zStart;
  long yEnd = command.indexOf(y.name) >= 0 ? mmOrInchToAbsolutePos(&y, getFloat(command, y.name)) : yStart;
  long xDiff = xEnd - xStart;
  long zDiff = zEnd - zStart;
  long yDiff = yEnd - yStart;
  updateAxisSpeeds(xDiff, zDiff, yDiff);
  long chunks = round(max(max(abs(xDiff), abs(zDiff)), abs(yDiff)) * LINEAR_INTERPOLATION_PRECISION);
  for (long i = 0; i < chunks; i++) {
    if (!isOn) return;
    float scale = i / float(chunks);
    stepToContinuous(&x, xStart + xDiff * scale);
    stepToContinuous(&z, zStart + zDiff * scale);
    if (ACTIVE_Y) stepToContinuous(&y, yStart + yDiff * scale);
    gcodeWaitNear();
  }
  // To avoid any rounding error, move to precise position.
  stepToFinal(&x, xEnd);
  stepToFinal(&z, zEnd);
  if (ACTIVE_Y) stepToFinal(&y, yEnd);
  gcodeWaitStop();
}

bool handleGcode(const String& command) {
  int op = getInt(command, 'G');
  if (op == 0 || op == 1) { // 0 also covers X and Z commands without G.
    G00_01(command);
  } else if (op == 20 || op == 21) {
    setMeasure(op == 20 ? MEASURE_INCH : MEASURE_METRIC);
  } else if (op == 90 || op == 91) {
    gcodeAbsolutePositioning = op == 90;
  } else if (op == 94) {
    /* no-op feed per minute */
  } else if (op == 18) {
    /* no-op ZX plane selection */
  } else {
    writeBuffer(&outBuffer, "error: unsupported command ");
    writeBuffer(&outBuffer, command);
    writeBuffer(&outBuffer, "\n");
    return false;
  }
  return true;
}

bool handleMcode(const String& command) {
  int op = getInt(command, 'M');
  if (op == 0 || op == 1 || op == 2 || op == 30) {
    setIsOnFromTask(false);
  } else {
    setIsOnFromTask(false);
    writeBuffer(&outBuffer, "error: unsupported command ");
    writeBuffer(&outBuffer, command);
    writeBuffer(&outBuffer, "\n");
    return false;
  }
  return true;
}

// Process one command, return ok flag.
bool handleGcodeCommand(String command) {
  command.trim();
  if (command.length() == 0) return false;

  // Trim N.. prefix.
  char code = command.charAt(0);
  int spaceIndex = command.indexOf(' ');
  if (code == 'N' && spaceIndex > 0) {
    command = command.substring(spaceIndex + 1);
    code = command.charAt(0);
  }

  // Update position for relative calculations right before performing them.
  z.gcodeRelativePos = gcodeAbsolutePositioning ? -z.originPos : z.pos;
  x.gcodeRelativePos = gcodeAbsolutePositioning ? -x.originPos : x.pos;
  y.gcodeRelativePos = gcodeAbsolutePositioning ? -y.originPos : y.pos;

  setFeedRate(command);
  switch (code) {
    case 'G':
    case NAME_Z:
    case NAME_X:
    case NAME_Y: return handleGcode(command);
    case 'F': return true; /* feed already handled above */
    case 'M': return handleMcode(command);
    case 'T': return true; /* ignoring tool changes */
    default: writeBuffer(&outBuffer, "error: unsupported command "); writeBuffer(&outBuffer, code); writeBuffer(&outBuffer, "\n"); return false;
  }
  return false;
}

void reset(); // forward declaration

void taskGcode(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    if (mode != MODE_GCODE) {
      gcodeInitialized = false;
    } else if (!gcodeInitialized) {
      gcodeInitialized = true;
      gcodeCommand = "";
      gcodeAbsolutePositioning = true;
      gcodeFeedDuPerSec = GCODE_FEED_DEFAULT_DU_SEC;
      gcodeInBrace = false;
      gcodeInSemicolon = false;
    }
    // Implementing a relevant subset of RS274 (Gcode) and GRBL (state management) covering basic use cases.
    char receivedChar = '\0';
    bool isWebSocket = false;
    if (mode == MODE_GCODE && isOn && gcodeProgramCharIndex < gcodeProgram.length()) {
      receivedChar = gcodeProgram.charAt(gcodeProgramCharIndex);
      gcodeProgramCharIndex++;
    } else if (bufferAvailable(&inBuffer)) {
      isWebSocket = true;
      receivedChar = shiftBuffer(&inBuffer);
    }
    int charCode = int(receivedChar);
    if (charCode > 0) {
      if (gcodeInBrace) {
        if (receivedChar == ')') gcodeInBrace = false;
      } else if (wsInKeycode) {
        if (charCode < 32) {
          if (wsKeycode == 0) {
            wsKeycode = keycodeCommand.toInt();
            writeBuffer(&outBuffer, String(wsKeycode));
            writeBuffer(&outBuffer, "\n");
          } else {
            writeBuffer(&outBuffer, "slower\n");
          }
          wsInKeycode = false;
          keycodeCommand = "";
        } else {
          keycodeCommand += receivedChar;
        }
      } else if (receivedChar == '(') {
        gcodeInBrace = true;
      } else if (receivedChar == ';' /* start of comment till end of line */) {
        gcodeInSemicolon = true;
      } else if (gcodeInSemicolon && charCode >= 32) {
        // Ignoring comment.
      } else if (receivedChar == 'R' /* reset all stops, positions, settings */) {
        if (!isOn) {
          reset();
          splashScreen = true;
          writeBuffer(&outBuffer, "reset\n");
        } else {
          writeBuffer(&outBuffer, "stop first\n");
        }
      } else if (receivedChar == '!' /* stop */) {
        setIsOnFromTask(false);
      } else if (receivedChar == '~' /* resume */) {
        setIsOnFromTask(true);
      } else if (receivedChar == '%' /* start/end marker */) {
        // Not using % markers in this implementation.
      } else if (receivedChar == '?' /* status */) {
        writeBuffer(&outBuffer, "<");
        writeBuffer(&outBuffer, isOn ? "Run" : "Idle");
        writeBuffer(&outBuffer, "|WPos:");
        float divisor = measure == MEASURE_METRIC ? 10000.0 : 254000.0;
        writeBuffer(&outBuffer, getAxisPosDu(&x) / divisor, 3);
        writeBuffer(&outBuffer, ",0.000,");
        writeBuffer(&outBuffer, getAxisPosDu(&z) / divisor, 3);
        writeBuffer(&outBuffer, "|FS:");
        writeBuffer(&outBuffer, round(gcodeFeedDuPerSec * 60 / 10000.0));
        writeBuffer(&outBuffer, ",");
        writeBuffer(&outBuffer, String(getApproxRpm()));
        writeBuffer(&outBuffer, "|Id:");
        writeBuffer(&outBuffer, "H" + String(HARDWARE_VERSION) + "V" + String(SOFTWARE_VERSION));
        writeBuffer(&outBuffer, ">"); // no new line to allow client to easily cut out the status response
      } else if (gcodeInSave && receivedChar == '"' /* end of saved program */) {
        gcodeInSave = false;
        if (gcodeSaveName.length() == 0) {
          if (removeAllGcode()) writeBuffer(&outBuffer, "ok\n");
        } else if (gcodeSaveValue.length() > 1) {
          if (saveGcode()) writeBuffer(&outBuffer, "ok\n");
        } else if (gcodeSaveName.length() == 1) {
          writeBuffer(&outBuffer, "error: name must be at least 2 chars\n");
        } else {
          removeGcodeByName(gcodeSaveName);
        }
        gcodeSaveName = "";
        gcodeSaveValue = "";
      } else if (!gcodeInSave && receivedChar == '"' /* start of save program */) {
        gcodeInSave = true;
        gcodeInSaveFirstLine = true;
      } else if (gcodeInSaveFirstLine && receivedChar >= 32) {
        gcodeSaveName += receivedChar;
      } else if (gcodeInSaveFirstLine && receivedChar < 32) {
        gcodeInSaveFirstLine = false;
        writeBuffer(&outBuffer, "ok\n");
      } else if (gcodeInSave) {
        gcodeSaveValue += receivedChar;
        if (receivedChar < 32) {
          gcodeInBrace = false;
          gcodeInSemicolon = false;
          writeBuffer(&outBuffer, "ok\n");
        }
      } else if (isOn) {
        if (gcodeInBrace && charCode < 32) {
          writeBuffer(&outBuffer, "error: comment not closed\n");
          setIsOnFromTask(false);
        } else if (charCode < 32 && gcodeCommand.length() > 1) {
          if (handleGcodeCommand(gcodeCommand)) {
            if (isWebSocket) writeBuffer(&outBuffer, "ok\n");
          }
          gcodeCommand = "";
          gcodeInSemicolon = false;
        } else if (charCode < 32) {
          if (isWebSocket) writeBuffer(&outBuffer, "ok\n");
          gcodeCommand = "";
          gcodeInSemicolon = false;
        } else if (charCode >= 32 && (charCode == 'G' || charCode == 'M')) {
          // Split consequent G and M commands on one line.
          // No "ok" for commands in the middle of the line.
          handleGcodeCommand(gcodeCommand);
          gcodeCommand = receivedChar;
        } else if (charCode >= 32) {
          gcodeCommand += receivedChar;
        }
      } else if (receivedChar == '=' /* start of keycode command */) {
        wsInKeycode = true;
        keycodeCommand = "";
      } else {
        // ignoring non-realtime command input when off
        // to flush any commands coming after an error
      }
    }
    if (mode == MODE_GCODE && isOn && gcodeProgramCharIndex > 0 && gcodeProgramCharIndex == gcodeProgram.length()) {
      setIsOnFromTask(false);
    }
    taskYIELD();
  }
  vTaskDelete(NULL);
}

void startPulseCounter(pcnt_unit_t unit, int gpioA, int gpioB) {
  pcnt_config_t pcntConfig;
  pcntConfig.pulse_gpio_num = gpioA;
  pcntConfig.ctrl_gpio_num = gpioB;
  pcntConfig.channel = PCNT_CHANNEL_0;
  pcntConfig.unit = unit;
  pcntConfig.pos_mode = PCNT_COUNT_INC;
  pcntConfig.neg_mode = PCNT_COUNT_DEC;
  pcntConfig.lctrl_mode = PCNT_MODE_REVERSE;
  pcntConfig.hctrl_mode = PCNT_MODE_KEEP;
  pcntConfig.counter_h_lim = PCNT_LIM;
  pcntConfig.counter_l_lim = -PCNT_LIM;
  pcnt_unit_config(&pcntConfig);
  pcnt_set_filter_value(unit, ENCODER_FILTER);
	pcnt_filter_enable(unit);
  pcnt_counter_pause(unit);
  pcnt_counter_clear(unit);
  pcnt_counter_resume(unit);
}

// Attaching interrupt on core 0 to have more time on core 1 where axes are moved.
void taskAttachInterrupts(void *param) {
  startPulseCounter(PCNT_UNIT_0, ENC_A, ENC_B);
  if (Z_PULSE_A >= 0) startPulseCounter(PCNT_UNIT_1, Z_PULSE_A, Z_PULSE_B);
  if (X_PULSE_A >= 0) startPulseCounter(PCNT_UNIT_2, X_PULSE_A, X_PULSE_B);
  if (Y_PULSE_A >= 0) startPulseCounter(PCNT_UNIT_3, Y_PULSE_A, Y_PULSE_B);
  vTaskDelete(NULL);
}

void setDupr(long value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  nextDupr = value;
  nextDuprFlag = true;
}

// Must be called while holding motionMutex.
void applyDupr() {
  if (nextDupr == dupr) {
    return;
  }
  dupr = nextDupr;
  markOrigin();
  if (mode == MODE_ASYNC || mode == MODE_Y) {
    updateAsyncTimerSettings();
  }
}

void setStarts(int value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  nextStarts = value;
  nextStartsFlag = true;
}

// Must be called while holding motionMutex.
void applyStarts() {
  if (starts == nextStarts) {
    return;
  }
  starts = nextStarts;
  markOrigin();
}

// Only used for async movement in ASYNC and Y modes.
// Keep code in this method to absolute minimum to achieve high stepper speeds.
void IRAM_ATTR onAsyncTimer() {
  Axis* a = getAsyncAxis();
  if (!isOn || a->movingManually || (mode != MODE_ASYNC && mode != MODE_Y)) {
    return;
  } else if (dupr > 0 && a->pos < a->leftStop) {
    if (a->pos <= a->motorPos) {
      a->pos++;
    }
    a->motorPos++;
    a->posGlobal++;
  } else if (dupr < 0 && a->pos > a->rightStop) {
    if (a->pos >= a->motorPos + a->backlashSteps) {
      a->pos--;
    }
    a->motorPos--;
    a->posGlobal--;
  } else {
    return;
  }

  DLOW(a->step);
  a->stepStartUs = micros();
  delayMicroseconds(10);
  DHIGH(a->step);
}

void setModeFromTask(int value) {
  nextMode = value;
  nextModeFlag = true;
}

void setModeFromLoop(int value) {
  if (mode == value) {
    return;
  }
  if (isOn) {
    setIsOnFromLoop(false);
  }
  if (mode == MODE_THREAD) {
    setStarts(1);
  } else if (mode == MODE_ASYNC || mode == MODE_Y) {
    setAsyncTimerEnable(false);
  }
  mode = value;
  setupIndex = 0;
  if (mode == MODE_ASYNC || mode == MODE_Y) {
    if (!timerAttached) {
      timerAttached = true;
      timerAttachInterrupt(async_timer, &onAsyncTimer);
    }
    updateAsyncTimerSettings();
    setAsyncTimerEnable(true);
  }
}

void setTurnPasses(int value) {
  if (isOn) {
    beep();
  } else {
    turnPasses = value;
  }
}

void setConeRatio(float value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  nextConeRatio = value;
  nextConeRatioFlag = true;
}

void applyConeRatio() {
  if (nextConeRatio == coneRatio) {
    return;
  }
  coneRatio = nextConeRatio;
  markOrigin();
}

void resetAxis(Axis *a) {
  a->leftStop = LONG_MAX;
  a->nextLeftStopFlag = false;
  a->rightStop = LONG_MIN;
  a->nextRightStopFlag = false;
  a->pos = 0;
  a->originPos = 0;
  a->posGlobal = 0;
  a->motorPos = 0;
  a->pendingPos = 0;
  a->disabled = false;
  a->stepperEnableCounter = 0;
}

void reset() {
  resetAxis(&z);
  resetAxis(&x);
  resetAxis(&y);
  setDupr(0);
  setStarts(1);
  moveStep = MOVE_STEP_1;
  setModeFromTask(MODE_NORMAL);
  measure = MEASURE_METRIC;
  showTacho = false;
  showAngle = false;
  setConeRatio(1);
  auxForward = true;
  setTurnPasses(3);
  setupIndex = 0;
  opIndex = 0;
  opSubIndex = 0;
  opIndexAdvanceFlag = false;
  inNumpad = false;
  numpadIndex = 0;
  spindlePos = 0;
  spindlePosAvg = 0;
  spindlePosSync = 0;
  spindlePosGlobal = 0;
}

long normalizePitch(long pitch) {
  int scale = 1;
  if (measure == MEASURE_METRIC) {
    // Drop the 3rd and 4th precision point if any.
    scale = 100;
  } else if (measure == MEASURE_INCH) {
    // Always drop the 4th precision point in inch representation if any.
    scale = 254;
  }
  return round(pitch / scale) * scale;
}

void buttonPlusMinusPress(bool plus) {
  // Mutex is aquired in setDupr() and setStarts().
  bool minus = !plus;
  if (mode == MODE_THREAD && setupIndex == 2) {
    if (minus && starts > 2) {
      setStarts(starts - 1);
    } else if (plus && starts < STARTS_MAX) {
      setStarts(starts + 1);
    }
  } else if (isPassMode() && setupIndex == 1 && getNumpadResult() == 0) {
    if (minus && turnPasses > 1) {
      setTurnPasses(turnPasses - 1);
    } else if (plus && turnPasses < PASSES_MAX) {
      setTurnPasses(turnPasses + 1);
    }
  } else if (measure != MEASURE_TPI) {
    int delta = measure == MEASURE_METRIC ? MOVE_STEP_3 : MOVE_STEP_IMP_3;
    // Switching between mm/inch/tpi often results in getting non-0 3rd and 4th
    // precision points that can't be easily controlled. Remove them.
    long normalizedDupr = normalizePitch(dupr);
    if (minus && dupr > -DUPR_MAX) {
      setDupr(max(-DUPR_MAX, normalizedDupr - delta));
    } else if (plus && dupr < DUPR_MAX) {
      setDupr(min(DUPR_MAX, normalizedDupr + delta));
    }
  } else { // TPI
    if (dupr == 0) {
      setDupr(plus ? 1 : -1);
    } else {
      long currentTpi = round(254000.0 / dupr);
      long tpi = currentTpi + (plus ? 1 : -1);
      long newDupr = tpi == 0 ? (plus ? DUPR_MAX : -DUPR_MAX) : round(254000.0 / tpi);
      // Happens for small pitches like 0.01mm.
      if (newDupr == dupr) {
        newDupr += plus ? -1 : 1;
      }
      if (newDupr != dupr && newDupr < DUPR_MAX && newDupr > -DUPR_MAX) {
        setDupr(newDupr);
      }
    }
  }
}

void buttonOnOffPress(bool on) {
  resetMillis = millis();
  bool missingZStops = needZStops() && (z.leftStop == LONG_MAX || z.rightStop == LONG_MIN);
  if (on && isPassMode() && (missingZStops || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN)) {
    beep();
  } else if (!isOn && on && mode == MODE_GCODE && gcodeProgramIndex >= gcodeProgramCount && setupIndex == 1) {
    beep();
  } else if (!isOn && on && setupIndex < getLastSetupIndex()) {
    if (mode == MODE_THREAD && setupIndex == 3) setConeRatio(0);
    // Move to the next setup step.
    setupIndex++;
  } else if (isOn && on && (mode == MODE_TURN || mode == MODE_FACE || mode == MODE_THREAD)) {
    // Move to the next pass.
    opIndexAdvanceFlag = true;
  } else if (!on && (z.movingManually || x.movingManually || x.movingManually)) {
    setEmergencyStop(ESTOP_OFF_MANUAL_MOVE);
  } else if (!isOn && on && mode == MODE_GCODE && gcodeProgramIndex >= gcodeProgramCount) {
    beep();
  } else if (!isOn && on && mode == MODE_GCODE) {
    String name = getCurrentGcodeProgramName();
    if (name.length() == 0) {
      beep();
    } else {
      gcodeProgramCharIndex = 0;
      gcodeProgram = readGcodeProgram(name);
      if (gcodeProgram.length() == 0) {
        beep();
      } else {
        gcodeProgram += '\n'; // ensures the last line is executed
        setIsOnFromTask(true);
      }
    }
  } else {
    setIsOnFromTask(on);
  }
}

void buttonOffRelease() {
  if (millis() - resetMillis > 3000) {
    reset();
    splashScreen = true;
  }
}

void setLeftStop(Axis* a, long value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  a->nextLeftStop = value;
  a->nextLeftStopFlag = true;
}

void leaveStop(Axis* a, long oldStop) {
  if (mode == MODE_CONE) {
    // To avoid rushing to a far away position if standing on limit.
    markOrigin();
  } else if (mode == MODE_NORMAL && a == getPitchAxis() && a->pos == oldStop) {
    // Spindle is most likely out of sync with the stepper because
    // it was spinning while the lead screw was on the stop.
    spindlePosSync = spindleModulo(spindlePos - spindleFromPos(a, a->pos));
  }
}

void applyLeftStop(Axis* a) {
  // Accept left stop even if it's lower than pos.
  // Stop button press processing takes time during which motor could have moved.
  long oldStop = a->leftStop;
  a->leftStop = a->nextLeftStop;
  leaveStop(a, oldStop);
}

void setRightStop(Axis* a, long value) {
  // Can't apply changes right away since we might be in the middle of motion logic.
  a->nextRightStop = value;
  a->nextRightStopFlag = true;
}

void applyRightStop(Axis* a) {
  // Accept right stop even if it's higher than pos.
  // Stop button press processing takes time during which motor could have moved.
  long oldStop = a->rightStop;
  a->rightStop = a->nextRightStop;
  leaveStop(a, oldStop);
}

void buttonLeftStopPress(Axis* a) {
  setLeftStop(a, a->leftStop == LONG_MAX ? a->pos : LONG_MAX);
}

void buttonRightStopPress(Axis* a) {
  setRightStop(a, a->rightStop == LONG_MIN ? a->pos : LONG_MIN);
}

void buttonDisplayPress() {
  if (!showAngle && !showTacho) {
    showAngle = true;
  } else if (showAngle) {
    showAngle = false;
    showTacho = true;
  } else {
    showTacho = false;
  }
}

void buttonMoveStepPress() {
  if (measure == MEASURE_METRIC) {
    if (moveStep == MOVE_STEP_1) {
      moveStep = MOVE_STEP_2;
    } else if (moveStep == MOVE_STEP_2) {
      moveStep = MOVE_STEP_3;
    } else {
      moveStep = MOVE_STEP_1;
    }
  } else {
    if (moveStep == MOVE_STEP_IMP_1) {
      moveStep = MOVE_STEP_IMP_2;
    } else if (moveStep == MOVE_STEP_IMP_2) {
      moveStep = MOVE_STEP_IMP_3;
    } else {
      moveStep = MOVE_STEP_IMP_1;
    }
  }
}

void buttonMeasurePress() {
  if (measure == MEASURE_METRIC) {
    setMeasure(MEASURE_INCH);
  } else if (measure == MEASURE_INCH) {
    setMeasure(MEASURE_TPI);
  } else {
    setMeasure(MEASURE_METRIC);
  }
}

void buttonReversePress() {
  setDupr(-dupr);
}

void numpadPress(int digit) {
  if (!inNumpad) {
    numpadIndex = 0;
  }
  numpadDigits[numpadIndex] = digit;
  if (numpadIndex < 7) {
    numpadIndex++;
  } else {
    numpadIndex = 0;
  }
}

void numpadBackspace() {
  if (inNumpad && numpadIndex > 0) {
    numpadIndex--;
  }
}

void resetNumpad() {
  numpadIndex = 0;
}

void numpadPlusMinus(bool plus) {
  if (numpadDigits[numpadIndex - 1] < 9 && plus) {
    numpadDigits[numpadIndex - 1]++;
  } else if (numpadDigits[numpadIndex - 1] > 1 && !plus) {
    numpadDigits[numpadIndex - 1]--;
  }
  // TODO: implement going over 9 and below 1.
}

unsigned long multistartPressMillis = 0;

void buttonMultistartPress() {
  if (millis() - multistartPressMillis > 3000 && starts > 1) {
    setStarts(1);
  } else {
    setStarts(starts + 1);
  }
  multistartPressMillis = millis();
}

bool processNumpadResult(int keyCode) {
  long newDu = numpadToDeciMicrons();
  float newConeRatio = numpadToConeRatio();
  long numpadResult = getNumpadResult();
  resetNumpad();
  // Ignore numpad input unless confirmed with ON.
  if (keyCode == B_ON) {
    if (isPassMode() && setupIndex == 1) {
      setTurnPasses(int(min(PASSES_MAX, numpadResult)));
      setupIndex++;
    } else if (mode == MODE_THREAD && setupIndex == 3) {
      setConeRatio(newConeRatio);
      setupIndex++;
    } else if (mode == MODE_CONE && setupIndex == 1) {
      setConeRatio(newConeRatio);
      setupIndex++;
    } else {
      if (abs(newDu) <= DUPR_MAX) {
        setDupr(newDu);
      }
    }
    // Don't use this ON press for starting the motion.
    return true;
  }

  // Shared piece for stops and moves.
  Axis* a = (keyCode == B_STOPL || keyCode == B_STOPR || keyCode == B_LEFT || keyCode == B_RIGHT || keyCode == B_Z) ? &z : &x;
  int sign = ((keyCode == B_STOPL || keyCode == B_STOPU || keyCode == B_LEFT || keyCode == B_UP || keyCode == B_Z || keyCode == B_X || keyCode == B_X_ENA) ? 1 : -1);
  if (keyCode == B_STOPF || keyCode == B_STOPB || keyCode == B_FORWARD || keyCode == B_BACK || keyCode == B_Y) {
    a = &y;
    sign = (keyCode == B_BACK || keyCode == B_STOPB) ? -1 : 1;
  }
  long posDiffAbs = (long)((long long)(a->rotational ? numpadResult * 10 : newDu) * a->motorSteps / a->screwPitch);
  long pos = a->pos + posDiffAbs * sign;

  // Potentially assign a new value to a limit. Treat newDu as a relative distance from current position.
  if (keyCode == B_STOPL) {
    setLeftStop(&z, pos);
    return true;
  } else if (keyCode == B_STOPR) {
    setRightStop(&z, pos);
    return true;
  } else if (keyCode == B_STOPU) {
    setLeftStop(&x, pos);
    return true;
  } else if (keyCode == B_STOPD) {
    setRightStop(&x, pos);
    return true;
  } else if (keyCode == B_STOPF && ACTIVE_Y) {
    setLeftStop(&y, pos);
    return true;
  } else if (keyCode == B_STOPB && ACTIVE_Y) {
    setRightStop(&y, pos);
    return true;
  }

  // Potentially move by newDu in the given direction.
  // We don't support precision manual moves when ON yet. Can't stay in the thread for most modes.
  if (!isOn && (keyCode == B_LEFT || keyCode == B_RIGHT || keyCode == B_UP || keyCode == B_DOWN || keyCode == B_FORWARD || keyCode == B_BACK)) {
    if (pos < a->rightStop) {
      pos = a->rightStop;
      beep();
    } else if (pos > a->leftStop) {
      pos = a->leftStop;
      beep();
    } else if (abs(pos - a->pos) > a->estopSteps) {
      beep();
      return true;
    }
    a->speedMax = a->speedManualMove;
    stepToFinal(a, pos);
    return true;
  }

  // Set axis 0 newDu ahead.
  if (keyCode == B_Z || keyCode == B_X || keyCode == B_Y) {
    a->originPos = -pos;
    return true;
  }

  // Set X axis 0 from diameter.
  if (keyCode == B_DIAMETER || keyCode == B_X_ENA) {
    a->originPos = -a->pos - posDiffAbs / 2;
    return true;
  }

  if (keyCode == B_STEP) {
    if (newDu > 0) {
      moveStep = newDu;
    } else {
      beep();
    }
    return true;
  }

  return false;
}

bool processNumpad(int keyCode) {
  if (keyCode == B_0) {
    numpadPress(0);
    inNumpad = true;
  } else if (keyCode == B_1) {
    numpadPress(1);
    inNumpad = true;
  } else if (keyCode == B_2) {
    numpadPress(2);
    inNumpad = true;
  } else if (keyCode == B_3) {
    numpadPress(3);
    inNumpad = true;
  } else if (keyCode == B_4) {
    numpadPress(4);
    inNumpad = true;
  } else if (keyCode == B_5) {
    numpadPress(5);
    inNumpad = true;
  } else if (keyCode == B_6) {
    numpadPress(6);
    inNumpad = true;
  } else if (keyCode == B_7) {
    numpadPress(7);
    inNumpad = true;
  } else if (keyCode == B_8) {
    numpadPress(8);
    inNumpad = true;
  } else if (keyCode == B_9) {
    numpadPress(9);
    inNumpad = true;
  } else if (keyCode == B_BACKSPACE) {
    numpadBackspace();
    inNumpad = true;
  } else if (inNumpad && (keyCode == B_PLUS || keyCode == B_MINUS)) {
    numpadPlusMinus(keyCode == B_PLUS);
    return true;
  } else if (inNumpad) {
    inNumpad = false;
    return processNumpadResult(keyCode);
  }
  return inNumpad;
}

void setModeFromUi(int modeToSet) {
  setModeFromTask(modeToSet);
}

void processKeypadEvent() {
  int event = 0;
  if (wsKeycode != 0) {
    int code = wsKeycode;
    wsKeycode = 0;
    if (code < 0) {
      // Negative keycode = release event (for jog buttons)
      event = (-code) | PS2_BREAK;
    } else {
      event = code;
    }
  }
  if (event == 0) return;
  int keyCode = event & 0xFF;
  bool isPress = !(event & PS2_BREAK);
  keypadTimeUs = micros();

  // Off button always gets handled.
  if (keyCode == B_OFF) {
    buttonOffPressed = isPress;
    isPress ? buttonOnOffPress(false) : buttonOffRelease();
  }

  if (mode == MODE_GCODE && isOn) {
    // Not allowed to interfere other than turn off.
    if (isPress && keyCode != B_OFF) beep();
    return;
  }

  // Releases don't matter in numpad but it has to run before LRUD since it might handle those keys.
  if (isPress && processNumpad(keyCode)) {
    return;
  }

  // Keyboard may not send release event if another button is pressed before first one is released.
  buttonLeftPressed = false;
  buttonRightPressed = false;
  buttonUpPressed = false;
  buttonDownPressed = false;
  buttonBackPressed = false;
  buttonForwardPressed = false;

  // Setup wizard navigation.
  if (isPress && setupIndex == 2 && (keyCode == B_LEFT || keyCode == B_RIGHT)) {
    auxForward = !auxForward;
  } else if (isPress && mode == MODE_GCODE && setupIndex == 1 && (keyCode == B_UP || keyCode == B_DOWN)) {
    if (gcodeProgramIndex > 0 && keyCode == B_UP) gcodeProgramIndex--;
    else if (gcodeProgramIndex == 0 && gcodeProgramCount > 0 && keyCode == B_UP) gcodeProgramIndex = gcodeProgramCount - 1;
    else if ((gcodeProgramIndex < gcodeProgramCount - 1) && keyCode == B_DOWN) gcodeProgramIndex++;
    else if (keyCode == B_DOWN) gcodeProgramIndex = 0;
  } else if (isPress && mode == MODE_GCODE && setupIndex == 1 && keyCode == B_MINUS) {
    removeGcodeByName(getCurrentGcodeProgramName());
    return;
  } else if (keyCode == B_LEFT) { // Make sure isPress=false propagates to motion flags.
    buttonLeftPressed = isPress;
  } else if (keyCode == B_RIGHT) {
    buttonRightPressed = isPress;
  } else if (keyCode == B_UP) {
    buttonUpPressed = isPress;
  } else if (keyCode == B_DOWN) {
    buttonDownPressed = isPress;
  } else if (keyCode == B_FORWARD) {
    buttonForwardPressed = isPress;
  } else if (keyCode == B_BACK) {
    buttonBackPressed = isPress;
  }

  // For all other keys we have no "release" logic.
  if (!isPress) {
    return;
  }

  // Rest of the buttons.
  if (keyCode == B_PLUS) {
    buttonPlusMinusPress(true);
  } else if (keyCode == B_MINUS) {
    buttonPlusMinusPress(false);
  } else if (keyCode == B_ON) {
    buttonOnOffPress(true);
  } else if (keyCode == B_STOPL) {
    buttonLeftStopPress(&z);
  } else if (keyCode == B_STOPR) {
    buttonRightStopPress(&z);
  } else if (keyCode == B_STOPU) {
    buttonLeftStopPress(&x);
  } else if (keyCode == B_STOPD) {
    buttonRightStopPress(&x);
  } else if (keyCode == B_STOPF && ACTIVE_Y) {
    buttonLeftStopPress(&y);
  } else if (keyCode == B_STOPB && ACTIVE_Y) {
    buttonRightStopPress(&y);
  } else if (keyCode == B_MODE_Y && ACTIVE_Y) {
    setModeFromUi(MODE_Y);
  } else if (keyCode == B_MODE_ELLIPSE) {
    setModeFromUi(MODE_ELLIPSE);
  } else if (keyCode == B_MODE_GCODE) {
    setModeFromUi(MODE_GCODE);
  } else if (keyCode == B_MODE_ASYNC) {
    setModeFromUi(MODE_ASYNC);
  } else if (keyCode == B_MULTISTART) {
    buttonMultistartPress();
  } else if (keyCode == B_DISPL) {
    buttonDisplayPress();
  } else if (keyCode == B_X) {
    markAxis0(&x);
  } else if (keyCode == B_Z) {
    markAxis0(&z);
  } else if (keyCode == B_Y && ACTIVE_Y) {
    markAxis0(&y);
  } else if (keyCode == B_X_ENA) {
    x.disabled = !x.disabled;
    updateEnable(&x);
  } else if (keyCode == B_Z_ENA) {
    z.disabled = !z.disabled;
    updateEnable(&z);
  } else if (keyCode == B_Y_ENA && ACTIVE_Y) {
    y.disabled = !y.disabled;
    updateEnable(&y);
  } else if (keyCode == B_STEP) {
    buttonMoveStepPress();
  } else if (keyCode == B_REVERSE) {
    buttonReversePress();
  } else if (keyCode == B_MEASURE) {
    buttonMeasurePress();
  } else if (keyCode == B_MODE_GEARS) {
    setModeFromUi(MODE_NORMAL);
  } else if (keyCode == B_MODE_TURN) {
    setModeFromUi(MODE_TURN);
  } else if (keyCode == B_MODE) {
    if (mode == MODE_NORMAL) setModeFromTask(MODE_TURN);
    else if (mode == MODE_TURN) setModeFromTask(MODE_FACE);
    else if (mode == MODE_FACE) setModeFromTask(MODE_CONE);
    else if (mode == MODE_CONE) setModeFromTask(MODE_CUT);
    else if (mode == MODE_CUT) setModeFromTask(MODE_THREAD);
    else if (mode == MODE_THREAD) setModeFromTask(MODE_ELLIPSE);
    else if (mode == MODE_ELLIPSE) setModeFromTask(MODE_GCODE);
    else if (mode == MODE_GCODE) setModeFromTask(MODE_ASYNC);
    else if (mode == MODE_ASYNC) setModeFromTask(y.active ? MODE_Y : MODE_NORMAL);
    else if (mode == MODE_Y) setModeFromTask(MODE_NORMAL);
  } else if (keyCode == B_MODE_FACE) {
    setModeFromUi(MODE_FACE);
  } else if (keyCode == B_MODE_CONE) {
    setModeFromUi(MODE_CONE);
  } else if (keyCode == B_MODE_CUT) {
    setModeFromUi(MODE_CUT);
  } else if (keyCode == B_MODE_THREAD) {
    setModeFromUi(MODE_THREAD);
  }
}

void taskKeypad(void *param) {
  while (emergencyStop == ESTOP_NONE) {
    processKeypadEvent();
    taskYIELD();
  }
  vTaskDelete(NULL);
}

void moveAxis(Axis* a) {
  // Most of the time a step isn't needed.
  if (a->pendingPos == 0) {
    if (a->speed > a->speedStart) {
      a->speed--;
    }
    return;
  }

  unsigned long nowUs = micros();
  float delayUs = 1000000.0 / a->speed;
  if (nowUs - a->stepStartUs < delayUs - 5) {
    // Not enough time has passed to issue this step.
    return;
  }

  if (xSemaphoreTake(a->mutex, 1) == pdTRUE) {
    // Check pendingPos again now that we have the mutex.
    if (a->pendingPos != 0) {
      bool dir = a->pendingPos > 0;
      setDir(a, dir);

      DLOW(a->step);
      int delta = dir ? 1 : -1;
      a->pendingPos -= delta;
      if (dir && a->motorPos >= a->pos) {
        a->pos++;
      } else if (!dir && a->motorPos <= (a->pos - a->backlashSteps)) {
        a->pos--;
      }
      a->motorPos += delta;
      a->posGlobal += delta;

      bool accelerate = a->continuous || a->pendingPos >= a->decelerateSteps || a->pendingPos <= -a->decelerateSteps;
      a->speed += (accelerate ? 1 : -1) * a->acceleration * delayUs / 1000000.0;
      if (a->speed > a->speedMax) {
        a->speed = a->speedMax;
      } else if (a->speed < a->speedStart) {
        a->speed = a->speedStart;
      }
      a->stepStartUs = nowUs;

      DHIGH(a->step);
    }
    xSemaphoreGive(a->mutex);
  }
}

void modeGearbox() {
  if (z.movingManually) {
    return;
  }
  z.speedMax = LONG_MAX;
  stepToContinuous(&z, posFromSpindle(&z, spindlePosAvg, true));
}

long auxSafeDistance, startOffset;
void modeTurn(Axis* main, Axis* aux) {
  if (main->movingManually || aux->movingManually || turnPasses <= 0 ||
      main->leftStop == LONG_MAX || main->rightStop == LONG_MIN ||
      aux->leftStop == LONG_MAX || aux->rightStop == LONG_MIN ||
      dupr == 0 || (dupr * opDuprSign < 0) || starts < 1) {
    setIsOnFromLoop(false);
    return;
  }

  // Variables below have to be re-calculated every time because origin can change
  // while TURN is running e.g. due to dupr change.
  long mainStartStop = opDuprSign > 0 ? main->rightStop : main->leftStop;
  long mainEndStop = opDuprSign > 0 ? main->leftStop : main->rightStop;
  long auxStartStop = auxForward ? aux->rightStop : aux->leftStop;
  long auxEndStop = auxForward ? aux->leftStop : aux->rightStop;

  // opIndex 0 is only executed once, do setup calculations here.
  auxSafeDistance = (auxForward ? -1 : 1) * SAFE_DISTANCE_DU * aux->motorSteps / aux->screwPitch;
  if (opIndex == 0) {
    startOffset = starts == 1 ? 0 : round(ENCODER_STEPS_FLOAT / starts);

    // Move to right-bottom limit.
    main->speedMax = main->speedManualMove;
    aux->speedMax = aux->speedManualMove;
    long auxPos = auxStartStop;
    // Overstep by 1 so that "main" backlash is taken out before "opSubIndex == 1".
    long mainPos = mainStartStop + (opDuprSign > 0 ? -1 : 1);
    stepToFinal(main, mainPos);
    stepToFinal(aux, auxPos);
    if (main->pos == mainPos && aux->pos == auxPos) {
      stepToFinal(main, mainStartStop);
      opIndex = 1;
      opSubIndex = 0;
    }
  } else if (opIndex <= turnPasses * starts) {
    if (opIndexAdvanceFlag && (opIndex + starts) < turnPasses * starts) {
      opIndexAdvanceFlag = false;
      opIndex += starts;
    }
    float fraction = (turnPasses - ceil(opIndex / float(starts))) / turnPasses;
    if (mode == MODE_THREAD) fraction = fraction * fraction; // make initial passed larger, final passes smaller
    long auxPos = auxEndStop - (auxEndStop - auxStartStop) * fraction;
    // Bringing X to starting position.
    if (opSubIndex == 0) {
      stepToFinal(aux, auxPos);
      if (aux->pos == auxPos) {
        opSubIndex = 1;
        spindlePosSync = spindleModulo(spindlePosGlobal - spindleFromPos(main, main->posGlobal) + startOffset * (opIndex - 1));
        return; // Instead of jumping to the next step, let spindlePosSync get to 0 first.
      }
    }
    // spindlePosSync counted down to 0, start thread from here.
    if (opSubIndex == 1) {
      markOrigin();
      main->speedMax = LONG_MAX;
      opSubIndex = 2;
      // markOrigin() changed Start/EndStop values, re-calculate them.
      return;
    }
    // Doing the pass cut.
    if (opSubIndex == 2) {
      // In case we were pushed to the next opIndex before finishing the current one.
      long mainTargetPos = posFromSpindle(main, spindlePosAvg, true);
      long auxTargetPos = auxPos;
      if (mode == MODE_THREAD && coneRatio != 0) {
        float coneEffectRatio = -coneRatio / 2 / main->motorSteps * aux->motorSteps / aux->screwPitch * main->screwPitch * (auxForward ? 1 : -1);
        auxTargetPos = auxPos + round(mainTargetPos * coneEffectRatio);
      }

      if (auxTargetPos > aux->leftStop) auxTargetPos = aux->leftStop;
      if (auxTargetPos < aux->rightStop) auxTargetPos = aux->rightStop;

      stepToContinuous(main, mainTargetPos);
      stepToContinuous(aux, auxTargetPos);
      if (main->pos == mainEndStop || (coneRatio != 0 && aux->pos == (opDuprSign > 0 ? auxStartStop : auxEndStop))) {
        opSubIndex = 3;
      }
    }
    // Retracting the tool
    if (opSubIndex == 3) {
      long auxTargetPos = (mode == MODE_THREAD ? auxStartStop : auxPos) + auxSafeDistance;
      stepToFinal(aux, auxTargetPos);
      if (aux->pos == auxTargetPos) {
        opSubIndex = 4;
      }
    }
    // Returning to start of main.
    if (opSubIndex == 4) {
      main->speedMax = main->speedManualMove;
      // Overstep by 1 so that "main" backlash is taken out before "opSubIndex == 2".
      long mainPos = mainStartStop + (opDuprSign > 0 ? -1 : 1);
      stepToFinal(main, mainPos);
      if (main->pos == mainPos) {
        stepToFinal(main, mainStartStop);
        opSubIndex = 0;
        opIndex++;
      }
    }
  } else {
    // Move to right-bottom limit.
    main->speedMax = main->speedManualMove;
    long auxPos = auxStartStop;
    long mainPos = mainStartStop;
    stepToFinal(main, mainPos);
    stepToFinal(aux, auxPos);
    if (main->pos == mainPos && aux->pos == auxPos) {
      setIsOnFromLoop(false);
      beep();
    }
  }
}

void modeCone() {
  if (z.movingManually || x.movingManually || coneRatio == 0) {
    return;
  }

  float zToXRatio = -coneRatio / 2 / z.motorSteps * x.motorSteps / x.screwPitch * z.screwPitch * (auxForward ? 1 : -1);
  if (zToXRatio == 0) {
    return;
  }

  // TODO: calculate maximum speeds and accelerations to avoid potential desync.
  x.speedMax = LONG_MAX;
  z.speedMax = LONG_MAX;

  // Respect limits of both axis by translating them into limits on spindlePos value.
  long spindle = spindlePosAvg;
  long spindleMin = LONG_MIN;
  long spindleMax = LONG_MAX;
  if (z.leftStop != LONG_MAX) {
    (dupr > 0 ? spindleMax : spindleMin) = spindleFromPos(&z, z.leftStop);
  }
  if (z.rightStop != LONG_MIN) {
    (dupr > 0 ? spindleMin: spindleMax) = spindleFromPos(&z, z.rightStop);
  }
  if (x.leftStop != LONG_MAX) {
    long lim = spindleFromPos(&z, round(x.leftStop / zToXRatio));
    if (zToXRatio < 0) {
      (dupr > 0 ? spindleMin: spindleMax) = lim;
    } else {
      (dupr > 0 ? spindleMax : spindleMin) = lim;
    }
  }
  if (x.rightStop != LONG_MIN) {
    long lim = spindleFromPos(&z, round(x.rightStop / zToXRatio));
    if (zToXRatio < 0) {
      (dupr > 0 ? spindleMax : spindleMin) = lim;
    } else {
      (dupr > 0 ? spindleMin: spindleMax) = lim;
    }
  }
  if (spindle > spindleMax) {
    spindle = spindleMax;
  } else if (spindle < spindleMin) {
    spindle = spindleMin;
  }

  stepToContinuous(&z, posFromSpindle(&z, spindle, true));
  stepToContinuous(&x, round(z.pos * zToXRatio));
}

void modeCut() {
  if (x.movingManually || turnPasses <= 0 || x.leftStop == LONG_MAX || x.rightStop == LONG_MIN || dupr == 0 || dupr * opDuprSign < 0) {
    setIsOnFromLoop(false);
    return;
  }

  long startStop = opDuprSign > 0 ? x.rightStop : x.leftStop;
  long endStop = opDuprSign > 0 ? x.leftStop : x.rightStop;

  if (opIndex == 0) {
    // Move to back limit.
    x.speedMax = x.speedManualMove;
    long xPos = startStop;
    stepToFinal(&x, xPos);
    if (x.pos == xPos) {
      opIndex = 1;
      opSubIndex = 0;
    }
  } else if (opIndex <= turnPasses) {
    // Set spindlePos and x.pos in sync.
    if (opSubIndex == 0) {
      spindlePosAvg = spindlePos = spindleFromPos(&x, x.pos);
      opSubIndex = 1;
    }
    // Doing the pass cut.
    if (opSubIndex == 1) {
      x.speedMax = LONG_MAX;
      long endPos = endStop - (endStop - startStop) / turnPasses * (turnPasses - opIndex);
      long xPos = posFromSpindle(&x, spindlePosAvg, true);
      if (dupr > 0 && xPos > endPos) xPos = endPos;
      else if (dupr < 0 && xPos < endPos) xPos = endPos;
      stepToContinuous(&x, xPos);
      if (x.pos == endPos) {
        opSubIndex = 2;
      }
    }
    // Returning to start.
    if (opSubIndex == 2) {
      x.speedMax = x.speedManualMove;
      stepToFinal(&x, startStop);
      if (x.pos == startStop) {
        opSubIndex = 0;
        opIndex++;
      }
    }
  } else {
    setIsOnFromLoop(false);
    beep();
  }
}

void modeEllipse(Axis* main, Axis* aux) {
  if (main->movingManually || aux->movingManually || turnPasses <= 0 ||
      main->leftStop == LONG_MAX || main->rightStop == LONG_MIN ||
      aux->leftStop == LONG_MAX || aux->rightStop == LONG_MIN ||
      main->leftStop == main->rightStop ||
      aux->leftStop == aux->rightStop ||
      dupr == 0 || dupr != opDupr) {
    setIsOnFromLoop(false);
    return;
  }

  // Start from left or right depending on the pitch.
  long mainStartStop = opDuprSign > 0 ? main->rightStop : main->leftStop;
  long mainEndStop = opDuprSign > 0 ? main->leftStop : main->rightStop;
  long auxStartStop = aux->rightStop;
  long auxEndStop = aux->leftStop;

  main->speedMax = main->speedManualMove;
  aux->speedMax = aux->speedManualMove;

  if (opIndex == 0) {
    opIndex = 1;
    opSubIndex = 0;
    spindlePos = 0;
    spindlePosAvg = 0;
  } else if (opIndex <= turnPasses) {
    float pass0to1 = opIndex / float(turnPasses);
    long mainDelta = round(pass0to1 * (mainEndStop - mainStartStop));
    long auxDelta = round(pass0to1 * (auxEndStop - auxStartStop));
    long spindleDelta = spindleFromPos(main, mainDelta);

    // Move to starting position.
    if (opSubIndex == 0) {
      long auxPos = auxStartStop;
      stepToFinal(aux, auxPos);
      if (aux->pos == auxPos) {
        opSubIndex = 1;
      }
    } else if (opSubIndex == 1) {
      long mainPos = mainEndStop - mainDelta;
      stepToFinal(main, mainPos);
      if (main->pos == mainPos) {
        opSubIndex = 2;
        spindlePos = 0;
        spindlePosAvg = 0;
      }
    } else if (opSubIndex == 2) {
      float progress0to1 = 0;
      if ((spindleDelta > 0 && spindlePosAvg >= spindleDelta) || (spindleDelta < 0 && spindlePosAvg <= spindleDelta)) {
        progress0to1 = 1;
      } else {
        progress0to1 = spindlePosAvg / float(spindleDelta);
      }
      float mainCoeff = auxForward ? cos(HALF_PI * (3 + progress0to1)) : (1 + sin(HALF_PI * (progress0to1 - 1)));
      long mainPos = mainEndStop - mainDelta + round(mainDelta * mainCoeff);
      float auxCoeff = auxForward ? (1 + sin(HALF_PI * (3 + progress0to1))) : sin(HALF_PI * progress0to1);
      long auxPos = auxStartStop + round(auxDelta * auxCoeff);
      stepToContinuous(main, mainPos);
      stepToContinuous(aux, auxPos);
      if (progress0to1 == 1 && main->pos == mainPos && aux->pos == auxPos) {
        opIndex++;
        opSubIndex = 0;
      }
    }
  } else if (opIndex == turnPasses + 1) {
    stepToFinal(aux, auxStartStop);
    if (aux->pos == auxStartStop) {
      setIsOnFromLoop(false);
      beep();
    }
  }
}

void discountFullSpindleTurns() {
  // When standing at the stop, ignore full spindle turns.
  // This allows to avoid waiting when spindle direction reverses
  // and reduces the chance of the skipped stepper steps since
  // after a reverse the spindle starts slow.
  if (dupr != 0 && !stepperIsRunning(&z) && (mode == MODE_NORMAL || mode == MODE_CONE)) {
    int spindlePosDiff = 0;
    if (z.pos == z.rightStop) {
      long stopSpindlePos = spindleFromPos(&z, z.rightStop);
      if (dupr > 0) {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) {
          spindlePosDiff = ENCODER_STEPS_INT;
        }
      } else {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) {
          spindlePosDiff = -ENCODER_STEPS_INT;
        }
      }
    } else if (z.pos == z.leftStop) {
      long stopSpindlePos = spindleFromPos(&z, z.leftStop);
      if (dupr > 0) {
        if (spindlePos > stopSpindlePos + ENCODER_STEPS_INT) {
          spindlePosDiff = -ENCODER_STEPS_INT;
        }
      } else {
        if (spindlePos < stopSpindlePos - ENCODER_STEPS_INT) {
          spindlePosDiff = ENCODER_STEPS_INT;
        }
      }
    }
    if (spindlePosDiff != 0) {
      spindlePos += spindlePosDiff;
      spindlePosAvg += spindlePosDiff;
    }
  }
}

void processSpindleCounter() {
  int16_t count;
  pcnt_get_counter_value(PCNT_UNIT_0, &count);
  int delta = count - spindleCount;
  if (delta == 0) {
    return;
  }
  if (count >= PCNT_CLEAR || count <= -PCNT_CLEAR) {
    pcnt_counter_clear(PCNT_UNIT_0);
    spindleCount = 0;
  } else {
    spindleCount = count;
  }

  unsigned long microsNow = micros();
  if (spindleEncTimeIndex >= RPM_BULK) {
    spindleEncTimeDiffBulk = microsNow - spindleEncTimeAtIndex0;
    spindleEncTimeAtIndex0 = microsNow;
    spindleEncTimeIndex = 0;
  }
  spindleEncTimeIndex += abs(delta);

  spindlePos += delta;
  spindlePosGlobal += delta;
  if (spindlePosGlobal > ENCODER_STEPS_INT) {
    spindlePosGlobal -= ENCODER_STEPS_INT;
  } else if (spindlePosGlobal < 0) {
    spindlePosGlobal += ENCODER_STEPS_INT;
  }
  if (spindlePos > spindlePosAvg) {
    spindlePosAvg = spindlePos;
  } else if (spindlePos < spindlePosAvg - ENCODER_BACKLASH) {
    spindlePosAvg = spindlePos + ENCODER_BACKLASH;
  }
  spindleEncTime = microsNow;

  if (spindlePosSync != 0) {
    spindlePosSync += delta;
    if (spindlePosSync % ENCODER_STEPS_INT == 0) {
      spindlePosSync = 0;
      Axis* a = getPitchAxis();
      spindlePosAvg = spindlePos = spindleFromPos(a, a->pos);
    }
  }
}

// Apply changes requested by the keyboard thread.
void applySettings() {
  if (nextDuprFlag) {
    applyDupr();
    nextDuprFlag = false;
  }
  if (nextStartsFlag) {
    applyStarts();
    nextStartsFlag = false;
  }
  if (z.nextLeftStopFlag) {
    applyLeftStop(&z);
    z.nextLeftStopFlag = false;
  }
  if (z.nextRightStopFlag) {
    applyRightStop(&z);
    z.nextRightStopFlag = false;
  }
  if (x.nextLeftStopFlag) {
    applyLeftStop(&x);
    x.nextLeftStopFlag = false;
  }
  if (x.nextRightStopFlag) {
    applyRightStop(&x);
    x.nextRightStopFlag = false;
  }
  if (y.nextLeftStopFlag) {
    applyLeftStop(&y);
    y.nextLeftStopFlag = false;
  }
  if (y.nextRightStopFlag) {
    applyRightStop(&y);
    y.nextRightStopFlag = false;
  }
  if (nextConeRatioFlag) {
    applyConeRatio();
    nextConeRatioFlag = false;
  }
  if (nextIsOnFlag) {
    setIsOnFromLoop(nextIsOn);
    nextIsOnFlag = false;
  }
  if (nextModeFlag) {
    setModeFromLoop(nextMode);
    nextModeFlag = false;
  }
}

// OLED RPM display — low priority, updates every 100ms
void taskOled(void *param) {
  Wire.begin(OLED_SDA, OLED_SCL, 400000);
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
  u8g2.begin();
  u8g2.setContrast(200);
  int lastRpm = -1;
  bool lastXdis = false;

  while (emergencyStop == ESTOP_NONE) {
    int rpm = getApproxRpm();
    bool xdis = x.disabled;
    if (rpm != lastRpm || xdis != lastXdis) {
      lastRpm = rpm;
      lastXdis = xdis;
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_logisoso42_tn);
      String rpmStr = String(rpm);
      int w = u8g2.getStrWidth(rpmStr.c_str());
      u8g2.drawStr(128 - w, 52, rpmStr.c_str());
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(0, 10, "RPM");
      u8g2.drawStr(80, 10, xdis ? "X FREE" : "X LOCK");
      u8g2.sendBuffer();
    }
    DELAY(100);
  }
  vTaskDelete(NULL);
}

// Physical button handler — short press = ON/advance, long press (>1.5s) = OFF/abort
#define LONG_PRESS_MS 1500

void taskButton(void *param) {
  bool lastState = HIGH;
  unsigned long pressStart = 0;
  bool longHandled = false;

  while (emergencyStop == ESTOP_NONE) {
    bool state = digitalRead(BUTTON_PIN);
    if (state == LOW && lastState == HIGH) {
      pressStart = millis();
      longHandled = false;
    } else if (state == LOW && !longHandled && (millis() - pressStart >= LONG_PRESS_MS)) {
      longHandled = true;
      if (wsKeycode == 0) wsKeycode = B_OFF;
    } else if (state == HIGH && lastState == LOW) {
      if (!longHandled && wsKeycode == 0) {
        wsKeycode = B_X_ENA;
      }
    }
    lastState = state;
    DELAY(20);
  }
  vTaskDelete(NULL);
}

void setup() {
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);

  pinMode(Z_DIR, OUTPUT);
  pinMode(Z_STEP, OUTPUT);
  pinMode(Z_ENA, OUTPUT);
  DHIGH(Z_STEP);

  pinMode(X_DIR, OUTPUT);
  pinMode(X_STEP, OUTPUT);
  pinMode(X_ENA, OUTPUT);
  // X starts released: drive the inactive ENA level before loading preferences.
  digitalWrite(X_ENA, INVERT_X_ENABLE ? HIGH : LOW);
  DHIGH(X_STEP);

  if (ACTIVE_Y) {
    pinMode(Y_DIR, OUTPUT);
    pinMode(Y_STEP, OUTPUT);
    pinMode(Y_ENA, OUTPUT);
    DHIGH(Y_STEP);
  }

  Preferences pref;
  pref.begin(PREF_NAMESPACE);
  if (pref.getInt(PREF_VERSION) != PREFERENCES_VERSION) {
    pref.clear();
    pref.putInt(PREF_VERSION, PREFERENCES_VERSION);
  }

  initAxis(&z, NAME_Z, true, false, MOTOR_STEPS_Z, SCREW_Z_DU, SPEED_START_Z, SPEED_MANUAL_MOVE_Z, ACCELERATION_Z, INVERT_Z, INVERT_Z_ENABLE, NEEDS_REST_Z, MAX_TRAVEL_MM_Z, BACKLASH_DU_Z, Z_ENA, Z_DIR, Z_STEP, Z_PULSE_A, Z_PULSE_B, PCNT_UNIT_1);
  initAxis(&x, NAME_X, true, false, MOTOR_STEPS_X, SCREW_X_DU, SPEED_START_X, SPEED_MANUAL_MOVE_X, ACCELERATION_X, INVERT_X, INVERT_X_ENABLE, NEEDS_REST_X, MAX_TRAVEL_MM_X, BACKLASH_DU_X, X_ENA, X_DIR, X_STEP, X_PULSE_A, X_PULSE_B, PCNT_UNIT_2);
  initAxis(&y, NAME_Y, ACTIVE_Y, ROTARY_Y, MOTOR_STEPS_Y, SCREW_Y_DU, SPEED_START_Y, SPEED_MANUAL_MOVE_Y, ACCELERATION_Y, INVERT_Y, INVERT_Y_ENABLE, NEEDS_REST_Y, MAX_TRAVEL_MM_Y, BACKLASH_DU_Y, Y_ENA, Y_DIR, Y_STEP, Y_PULSE_A, Y_PULSE_B, PCNT_UNIT_3);

  isOn = false;
  savedDupr = dupr = pref.getLong(PREF_DUPR);
  motionMutex = xSemaphoreCreateMutex();
  savedStarts = starts = min(STARTS_MAX, max(static_cast<int32_t>(1), pref.getInt(PREF_STARTS)));
  z.savedPos = z.pos = pref.getLong(PREF_POS_Z);
  z.savedPosGlobal = z.posGlobal = pref.getLong(PREF_POS_GLOBAL_Z);
  z.savedOriginPos = z.originPos = pref.getLong(PREF_ORIGIN_POS_Z);
  z.savedMotorPos = z.motorPos = pref.getLong(PREF_MOTOR_POS_Z);
  z.savedLeftStop = z.leftStop = pref.getLong(PREF_LEFT_STOP_Z, LONG_MAX);
  z.savedRightStop = z.rightStop = pref.getLong(PREF_RIGHT_STOP_Z, LONG_MIN);
  z.savedDisabled = z.disabled = pref.getBool(PREF_DISABLED_Z, false);
  x.savedPos = x.pos = pref.getLong(PREF_POS_X);
  x.savedPosGlobal = x.posGlobal = pref.getLong(PREF_POS_GLOBAL_X);
  x.savedOriginPos = x.originPos = pref.getLong(PREF_ORIGIN_POS_X);
  x.savedMotorPos = x.motorPos = pref.getLong(PREF_MOTOR_POS_X);
  x.savedLeftStop = x.leftStop = pref.getLong(PREF_LEFT_STOP_X, LONG_MAX);
  x.savedRightStop = x.rightStop = pref.getLong(PREF_RIGHT_STOP_X, LONG_MIN);
  x.savedDisabled = x.disabled = true;
  y.savedPos = y.pos = pref.getLong(PREF_POS_Y);
  y.savedPosGlobal = y.posGlobal = pref.getLong(PREF_POS_GLOBAL_Y);
  y.savedOriginPos = y.originPos = pref.getLong(PREF_ORIGIN_POS_Y);
  y.savedMotorPos = y.motorPos = pref.getLong(PREF_MOTOR_POS_Y);
  y.savedLeftStop = y.leftStop = pref.getLong(PREF_LEFT_STOP_Y, LONG_MAX);
  y.savedRightStop = y.rightStop = pref.getLong(PREF_RIGHT_STOP_Y, LONG_MIN);
  y.savedDisabled = y.disabled = pref.getBool(PREF_DISABLED_Y, false);
  savedSpindlePos = spindlePos = pref.getLong(PREF_SPINDLE_POS);
  savedSpindlePosAvg = spindlePosAvg = pref.getLong(PREF_SPINDLE_POS_AVG);
  savedSpindlePosSync = spindlePosSync = pref.getInt(PREF_OUT_OF_SYNC);
  savedSpindlePosGlobal = spindlePosGlobal = pref.getLong(PREF_SPINDLE_POS_GLOBAL);
  savedShowAngle = showAngle = pref.getBool(PREF_SHOW_ANGLE);
  savedShowTacho = showTacho = pref.getBool(PREF_SHOW_TACHO);
  savedMoveStep = moveStep = pref.getLong(PREF_MOVE_STEP, MOVE_STEP_1);
  setModeFromLoop(savedMode = pref.getInt(PREF_MODE));
  savedMeasure = measure = pref.getInt(PREF_MEASURE);
  savedConeRatio = coneRatio = pref.getFloat(PREF_CONE_RATIO, coneRatio);
  savedTurnPasses = turnPasses = pref.getInt(PREF_TURN_PASSES, turnPasses);
  savedAuxForward = auxForward = pref.getBool(PREF_AUX_FORWARD, true);
  pref.end();

  updateEnable(&z);
  updateEnable(&x);
  if (y.active) updateEnable(&y);

  if (LittleFS.begin(true)) {
    gcodeProgramCount = getGcodeProgramCount();
  }

  // Debug.
  Serial.begin(115200);

  // Physical button disabled — GPIO 12 interferes with X stepper enable.
  // pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Keypad task processes wsKeycode from WebSocket.
  xTaskCreatePinnedToCore(taskKeypad, "taskKeypad", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);

  // OLED RPM display — low priority, non-blocking.
  xTaskCreatePinnedToCore(taskOled, "taskOled", 4096 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);

  // Non-time-sensitive tasks on core 0.
  xTaskCreatePinnedToCore(taskAttachInterrupts, "taskAttachInterrupts", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskDisplay, "taskDisplay", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskMoveZ, "taskMoveZ", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskMoveX, "taskMoveX", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  if (ACTIVE_Y) xTaskCreatePinnedToCore(taskMoveY, "taskMoveY", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  xTaskCreatePinnedToCore(taskGcode, "taskGcode", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
  if (WIFI_ENABLED) xTaskCreatePinnedToCore(taskWiFi, "taskWiFI", 10000 /* stack size */, NULL, 0 /* priority */, NULL, 0 /* core */);
}

void loop() {
  if (emergencyStop != ESTOP_NONE) {
    return;
  }
  if (xSemaphoreTake(motionMutex, 1) != pdTRUE) {
    return;
  }
  applySettings();
  processSpindleCounter();
  discountFullSpindleTurns();
  if (!isOn || dupr == 0 || spindlePosSync != 0) {
    // None of the modes work.
  } else if (mode == MODE_NORMAL) {
    modeGearbox();
  } else if (mode == MODE_TURN) {
    modeTurn(&z, &x);
  } else if (mode == MODE_FACE) {
    modeTurn(&x, &z);
  } else if (mode == MODE_CUT) {
    modeCut();
  } else if (mode == MODE_CONE) {
    modeCone();
  } else if (mode == MODE_THREAD) {
    modeTurn(&z, &x);
  } else if (mode == MODE_ELLIPSE) {
    modeEllipse(&z, &x);
  }
  moveAxis(&z);
  moveAxis(&x);
  if (ACTIVE_Y) moveAxis(&y);
  xSemaphoreGive(motionMutex);
}
