# Calibration Guide (MD20 LV-TTL + PWM follower fingers)

This document explains how the key parameters in `firmware/arduino/code/code.ino` were calibrated.

## 1. Safety checklist (before calibration)
- Use **low speed** first: `kSpeedOpen/kSpeedClose = 5~10`
- Keep fingers away from hard stops; avoid long time stall
- Ensure **common GND** between Arduino and motor power supply
- Prepare **STOP button (D20)** for emergency hold

---

## 2. LV-TTL motion limits: `kPosMin` / `kPosMax`
**Purpose:** define safe open/close endpoints (avoid hard mechanical stops)

### Procedure
1. With no object in hand, press **OPEN** and observe the thumb+index pair.
2. Slowly move toward open direction and find the **mechanically safe open pose**.
3. Record the corresponding MD20 position as `kPosMin` (current: `80`).
4. Slowly move toward close direction (empty) and find the **safe close pose** (no hard stall).
5. Record the MD20 position as `kPosMax` (current: `320`).

### Tuning tips
- If fingers hit hard stops → **reduce** the limit range (increase `kPosMin` or decrease `kPosMax`)
- If open is not wide enough → **decrease** `kPosMin` carefully (only if still safe)

---

## 3. Load-threshold pinch: `kLoadThreshold`
**Purpose:** stop each LV-TTL finger when contact is detected (load/torque proxy from MD20)

### Observed load range (example)
- **Ping-pong ball grasp:** load ≈ **35 ~ 55**
- We set `kLoadThreshold = 40` so that:
  - empty-close load stays mostly below threshold
  - object contact reliably crosses threshold

### Procedure (recommended)
1. Empty close (no object): press **GRIP**, observe `Load=` prints in Serial Monitor.
2. Grasp typical objects (e.g., ping-pong ball): press **GRIP**, observe contact load.
3. Choose threshold:
   - slightly above empty-close baseline
   - below typical contact load
4. Verify: both sides should stop and hold without continuous stalling.

### Tuning tips
- If it stops too early (weak pinch) → **increase** `kLoadThreshold`
- If it crushes objects / stalls too hard → **decrease** `kLoadThreshold`
- If load readings fluctuate a lot → increase `kMonitorIntervalMs` or reduce speed

---

## 4. PWM follower mapping: `kPwmPulseAtPosMin/Max`
**Purpose:** after pinch, let follower fingers wrap around the object for more friction and stability.

### Finger mapping (this project)
- PWM #1 (D2)  → **Middle finger**
- PWM #2 (D6)  → **Ring finger**
- PWM #3 (D10) → **Pinky**

### Endpoint calibration logic
The firmware does a linear mapping from **Motor2 (index)** position ratio to PWM pulse width:

- At `kPosMin` (open): `kPwmPulseAtPosMin = {1080, 680, 680}`
- At `kPosMax` (close): `kPwmPulseAtPosMax = {1300, 1060, 1120}`

### Procedure
1. Put the index finger (Motor2) at `kPosMin`.  
2. For each PWM finger, adjust pulse until it reaches the desired “open” pose. Record as `kPwmPulseAtPosMin[i]`.
3. Put Motor2 at `kPosMax`.
4. For each PWM finger, adjust pulse until it reaches the desired “wrap/close” pose. Record as `kPwmPulseAtPosMax[i]`.
5. Test **SYNC**: follower fingers should track smoothly as Motor2 moves.

### Tuning tips
- If a PWM finger moves opposite direction → swap min/max pulses for that finger
- If a finger saturates too early → widen pulse range slightly (within safe bounds)
- If servo jitters → ensure stable power supply and common ground

---

## 5. Fine adjustment: `kFineStep` / `kFineFastRegionEnd`
**Purpose:** improve precision for small objects (e.g., thumbtack) without sacrificing speed.

Current values:
- `kFineStep = 40`
- `kFineFastRegionEnd = 280`

### Tuning tips
- For small objects, reduce step size (e.g., `20`) for higher precision
- If fine adjust is too slow, increase step size (e.g., `50~60`)
- Keep the region split to avoid many tiny moves near the endpoint