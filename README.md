# Dexterous Hand: 2x LV-TTL Pinch + 3x PWM Follower Fingers (Button Control)

A lightweight 5-finger robotic hand controller on **Arduino Mega2560**, featuring:
- **Thumb + Index (LV-TTL / MD20)**: load-threshold pinch (auto-stop on contact)
- **3 follower fingers (PWM servos)**: follow index position to increase friction & grasp stability
- **Fine adjustment** (in/out) for both LV-TTL motors
- **Low-latency button control** (no Bluetooth delay), plus serial fallback commands

> Author: Kaiyang Deng  
> Board: Mega2560 Pro (or Mega2560)

---

## Demo

- **Quick demo GIF**: `media/gifs/demo.gif`  
  ![demo](media/gifs/demo.gif)

- Full grasp test videos: **TODO: paste link (Bilibili/YouTube/Drive)**

---

## System Overview

**Control idea**
1. Use **LV-TTL motors (thumb & index)** to perform pinch grasp with **load feedback**  
2. After pinch contact is detected, command **3 PWM fingers** to follow the index position -> turn pinch into full-hand grasp  
3. Use **fine adjust** to precisely tune finger pose for small objects  
4. Use **physical buttons** to reduce latency and improve robustness

**Block diagram (TODO: add image in docs/wiring/)**
- Buttons -> Arduino -> (Serial1/Serial2 -> MD20 motors) + (PWM -> 3 servos)

---

## Hardware

### Controller & Motors
- MCU: **Arduino Mega2560 Pro** (works with Mega2560)
- LV-TTL motors: **2x MD20**  
  - Serial1: Motor1 (thumb) bus  
  - Serial2: Motor2 (index) bus
- PWM motors: **3x servo/PWM motors** for follower fingers

### Power (IMPORTANT)
- **TODO**: specify your supply (voltage/current), and whether servos share ground with Arduino
- Ensure **common GND** between Arduino and motor power system.

---

## Wiring & Pin Map

### Serial buses (Mega2560)
| Function | Arduino Port | Pins | Notes |
|---|---|---|---|
| USB debug | Serial | TX0/RX0 | 115200 |
| LV-TTL Motor1 | Serial1 | TX1=18, RX1=19 | MD20 bus #1 |
| LV-TTL Motor2 | Serial2 | TX2=16, RX2=17 | MD20 bus #2 |

### PWM motors
| PWM Motor | Pin |
|---|---|
| PWM #1 | D2 |
| PWM #2 | D6 |
| PWM #3 | D10 |

### Buttons (INPUT_PULLUP: released=HIGH, pressed=LOW)
> Wire each button between the pin and **GND** (no external resistor needed).

| Function | Pin | Action |
|---|---:|---|
| OPEN | D40 | Open both LV-TTL motors to `kPosMin` |
| GRIP | D42 | Close both LV-TTL motors to `kPosMax` until `load > kLoadThreshold` |
| STOP | D20 | Emergency stop (hold current positions) |
| SYNC | D22 | PWM follower fingers follow **Motor2 (index)** position |
| Fine IN M1 | D25 | Thumb inward fine adjust |
| Fine OUT M1 | D27 | Thumb outward fine adjust |
| Fine IN M2 | D29 | Index inward fine adjust |
| Fine OUT M2 | D31 | Index outward fine adjust |

---

## Key Parameters (What they mean & how to tune)

These parameters are defined in the top of the firmware file:

### LV-TTL motion limits
- `kPosMin = 80`, `kPosMax = 320`  
  Define safe open/close endpoints (avoid hitting hard stops).  
  **Tune**: calibrate with the hand unloaded first.

### Load threshold grasping
- `kLoadThreshold = 40`  
  When closing, if `load > threshold`, that finger stops -> pinch grasp finishes when both sides contact.  
  **Tune**: record typical load for "empty close" vs "grasp object", choose a margin.

### Monitoring rate
- `kMonitorIntervalMs = 100`  
  Read LV-TTL pos/load every 100 ms.  
  **Tune**: smaller = more responsive, but more serial traffic.

### PWM follower calibration
- `kPwmPulseAtPosMin = {1080,680,680}`  
- `kPwmPulseAtPosMax = {1300,1060,1120}`  
  Map index (Motor2) position ratio to PWM pulses.  
  **Tune**: align follower fingers at pos=min/max and record pulse widths.

### Fine adjustment
- `kFineStep = 40`, `kFineFastRegionEnd = 280`  
  Coarse-to-fine adjustment for small objects.  
  **Tune**: smaller step = finer but slower; larger = faster but may overshoot.

---

## How to Run

### 1) Flash firmware
1. Install **Arduino IDE**
2. Open: `firmware/arduino/final/final.ino`
3. Select board: `Arduino Mega or Mega 2560`
4. Select port and upload

### 2) Use buttons (recommended)
- Press OPEN -> hand opens (pinch pair to `kPosMin`)
- Press GRIP -> pinch closes until load threshold reached
- Press SYNC -> follower fingers wrap around object (PWM follow Motor2)
- Fine adjust if needed -> precise pose alignment
- STOP anytime -> emergency hold

### 3) Serial fallback commands (USB Serial @115200)
| Command | Action |
|---|---|
| `1` | OPEN |
| `2` | GRIP |
| `3` | STOP |
| `4` | SYNC |

---

## Results (Grasp Tests)

A simple benchmark is stored in: `results/grasp_tests.csv`.

**Summary (TODO)**
- Objects tested: TODO
- Success rate: TODO
- Typical failure cases: TODO (slip / misalignment / insufficient friction)

---

## Repo Structure

- `firmware/` Arduino firmware
- `hardware/` CAD (STEP/DWG), BOM, assembly notes
- `docs/` wiring, calibration guide
- `media/` photos and demo GIFs
- `results/` grasp test logs & summary

---

## Safety Notes

- Always test with **low speed** first (`kSpeedOpen/kSpeedClose` small).
- Ensure `kPosMin/kPosMax` do not hit hard stops.
- PWM servos must stay within safe pulse bounds (`500~2500us`).
- Use **STOP (D20)** for emergency hold.

---

## License

**TODO** choose a license (MIT / Apache-2.0 / GPL-3.0).  
If your hardware files have restrictions, mention them clearly here.