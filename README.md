# Dexterous Hand (Arduino Mega2560)
**2× LV-TTL (MD20) pinch + 3× PWM follower fingers + button control + fine adjust**

This project implements a lightweight 5-finger hand controller:
- **Thumb + Index (LV-TTL / MD20)**: pinch grasp with **load-threshold contact stop**
- **3 PWM fingers**: follow index motor position to convert pinch → full-hand wrap (more friction, higher success)
- **Fine adjustment**: precise in/out micro-moves for small objects
- **Buttons over Bluetooth**: lower latency and more reliable control (plus serial fallback)

**Board:** Arduino Mega2560 Pro (works with Mega2560)  
**Author:** Kaiyang Deng

---

## Demo

![demo](media/gifs/demo.gif)

This GIF shows a **pinch grasp & safe release** using the LV-TTL thumb+index pair.  
After a successful pinch, press **SYNC** to let the 3 PWM fingers **wrap and increase friction** (full-hand grasp).

## Video Demos

- **Overview (hardware + control modes)**: https://youtube.com/shorts/v7Af31TAydM?feature=share
- **Manipulation: Bottle cap loosening (strategy iteration)**: https://youtu.be/Ag10vxxGfRg
- **Small object grasp: Thumbtack with fine adjustment**: https://youtu.be/Tc2J1AXXHpU

- See full list: [media/videos.md](media/videos.md)
---

## System Overview

### Control Strategy (Why this design)
1. **Pinch (thumb + index)**: close both LV-TTL motors toward `kPosMax`  
2. **Contact detection**: stop each side when `load > kLoadThreshold` (torque/load proxy from MD20)  
3. **Full-hand wrap (SYNC)**: map index (Motor2) position ratio → PWM pulses → 3 fingers follow and stabilize grasp  
4. **Fine adjust**: quick coarse move + fine in/out steps for small objects
5. **Buttons**: reduce wireless latency and simplify field use (one button = one function)

---

## Quick Start (60 seconds)

### 1) Flash firmware
1. Open `firmware/arduino/code/code.ino` with Arduino IDE
2. Select board: **Arduino Mega or Mega 2560**
3. Upload
4. (Optional) Open Serial Monitor @ **115200**

### 2) Use buttons (recommended)
- **OPEN (D40)**: open gripper to `kPosMin`
- **GRIP (D42)**: close until `load > kLoadThreshold` (auto-stop on contact)
- **SYNC (D22)**: PWM fingers follow **Motor2 (index)** position
- **STOP (D20)**: emergency stop (hold current pose)
- Fine adjust:
  - **M1 (thumb):** IN D25 / OUT D27
  - **M2 (index):** IN D29 / OUT D31

### 3) Serial fallback commands (USB Serial @115200)
| Key | Action |
|---|---|
| `1` | OPEN |
| `2` | GRIP (stop on load threshold) |
| `3` | STOP |
| `4` | SYNC (PWM follow Motor2) |

---

## Wiring & Pin Map

### Serial buses (Mega2560)
| Function | Port | Pins | Notes |
|---|---|---|---|
| Debug | Serial | TX0/RX0 | 115200 |
| LV-TTL Motor1 (thumb) | Serial1 | TX1=18, RX1=19 | MD20 bus #1 |
| LV-TTL Motor2 (index) | Serial2 | TX2=16, RX2=17 | MD20 bus #2 |

> Note: both motor IDs are `0x01` because motors are on **separate UART buses**.  
> If you put motors on the **same bus**, IDs must be unique.

### PWM motors
| PWM Motor | Arduino Pin |
|---|---|
| PWM #1 | D2 |
| PWM #2 | D6 |
| PWM #3 | D10 |

- Mapping: PWM#1 = Middle, PWM#2 = Ring, PWM#3 = Pinky

### Buttons (INPUT_PULLUP)
- Released = HIGH, Pressed = LOW
- Wire each button between the pin and **GND** (no external resistor required)

| Function | Pin |
|---|---:|
| OPEN | D40 |
| GRIP | D42 |
| STOP | D20 |
| SYNC | D22 |
| Fine IN M1 | D25 |
| Fine OUT M1 | D27 |
| Fine IN M2 | D29 |
| Fine OUT M2 | D31 |

---

## Key Parameters (What they mean & how to tune)

These are defined at the top of `code.ino`.

### LV-TTL travel limits
- `kPosMin = 80`, `kPosMax = 320`  
  Safe open/close endpoints (avoid hard mechanical stops).  
  **Tune:** adjust so fingers never hit hard stops under load.

### Load-threshold grasping
- `kLoadThreshold = 40`  
  When closing, if `load > threshold`, that side is considered “in contact” and stops.  
  **Tune method:** record load while empty-closing vs grasping typical objects; set threshold with margin.

### Monitoring
- `kMonitorIntervalMs = 100`  
  Read pos/load every 100ms.  
  **Tune:** smaller = more responsive, but more serial traffic.

### PWM follower calibration
- `kPwmPulseAtPosMin = {1080, 680, 680}`  
- `kPwmPulseAtPosMax = {1300, 1060, 1120}`  
  Linear mapping from Motor2 position ratio → PWM pulse width (µs).  
  **Tune:** align follower fingers at pos=min/max, record pulse widths.

### Fine adjustment
- `kFineStep = 40`, `kFineFastRegionEnd = 280`  
  Coarse-to-fine adjustment for small objects.  
  **Tune:** smaller step = finer but slower; larger = faster but may overshoot.

---

## Calibration (recommended)

See: [docs/calibration.md](docs/calibration.md)

It should include:
- How to determine `kPosMin/kPosMax` safely
- How to select `kLoadThreshold` based on observed load values
- How to calibrate PWM pulses at endpoints

---

## Results (Grasp Tests)

- Raw log: [results/grasp_tests.csv](results/grasp_tests.csv)
- Summary: [results/grasp_summary.md](results/grasp_summary.md)

- Headline: medium/large objects ~100% success; small objects (thumbtack) ~30% success (requires fine adjustment).
- Manipulation demos: bottle-cap loosening + card insertion (see videos).
---

## Repository Structure

- `firmware/` Arduino code
- `hardware/`
  - `cad/` STEP/DWG (tracked by Git LFS)
  - `bom/` BOM.xlsx
- `media/` demo GIF / photos
- `docs/` wiring + calibration guides
- `results/` grasp benchmarks

---

## Safety Notes

- Start with low speeds (`kSpeedOpen/kSpeedClose`) and verify motion direction
- Ensure `kPosMin/kPosMax` do not hit hard stops
- PWM pulse bounds are clamped to `500~2500µs` for safety
- Use **STOP (D20)** for emergency hold
- Ensure common **GND** between Arduino and motor power

---

## License
- Code: Licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
- Hardware: Licensed under the [CERN Open Hardware License](https://www.ohwr.org/projects/cernohl).