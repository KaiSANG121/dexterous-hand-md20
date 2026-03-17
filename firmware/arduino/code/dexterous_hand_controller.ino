#include <Arduino.h>
#include <Servo.h>  

/*
  ==========================================================
  Project: 2x MD20 (LV-TTL) Pinch + 3x PWM Follower Fingers
  Board  : Mega2560 Pro (or Mega2560)
  Author : Kaiyang Deng (repo versioned)

  Features:
  - LV-TTL motors (thumb & index): torque/load-threshold pinch (auto stop on contact)
  - 3x PWM motors: follower fingers track index motor position to increase friction
  - Fine adjustment for both LV-TTL motors (in/out)
  - Button-based control (low latency), plus serial command fallback

  Serial Ports:
    Serial  -> USB monitor (115200)
    Serial1 -> MD20 Motor#1 bus (TX1=18 RX1=19)
    Serial2 -> MD20 Motor#2 bus (TX2=16 RX2=17)
  ==========================================================
*/

// ==========================================================
// ==================== configuration ========================
// ==========================================================

// ---------- LV-TTL motors ----------
static const uint8_t  kMotorCount = 2;

// Note: Both IDs are 0x01 because each motor is on a separate UART bus.
// If you put them on the SAME bus, you must use different IDs.
static const uint8_t  kMotorId1 = 0x01;
static const uint8_t  kMotorId2 = 0x01;

static const uint16_t kPosMin = 80;    // open
static const uint16_t kPosMax = 320;   // close
static const uint16_t kSpeedOpen  = 10;
static const uint16_t kSpeedClose = 10;

static const uint16_t kLoadThreshold = 40;     // contact threshold
static const uint32_t kMonitorIntervalMs = 100;

// Fine adjust parameters
static const uint16_t kFineStep = 40;          // coarse fine-adjust step
static const uint16_t kFineFastRegionEnd = 280; // <= this: step by kFineStep; > this: jump to end

// ---------- PWM motors ----------
static const uint8_t kPwmCount = 3;
static const uint8_t kPwmPins[kPwmCount] = {2, 6, 10};
static Servo pwmServos[kPwmCount];

// Calibration: pulse width at LV-TTL pos endpoints (pos=80 and pos=320)
static const int kPwmPulseAtPosMin[kPwmCount] = {1080,  680,  680};
static const int kPwmPulseAtPosMax[kPwmCount] = {1300, 1060, 1120};

// PWM safe bounds
static const int kPwmPulseSafeMin = 500;
static const int kPwmPulseSafeMax = 2500;

// ---------- Buttons (INPUT_PULLUP: released=HIGH, pressed=LOW) ----------
static const int kBtnOpen  = 40;  // OPEN  -> open gripper
static const int kBtnGrip  = 42;  // GRIP  -> auto close until contact
static const int kBtnStop  = 20;  // STOP  -> emergency stop
static const int kBtnSync  = 22;  // SYNC  -> PWM follow motor2 position

static const int kBtnFineInM1  = 25;
static const int kBtnFineOutM1 = 27;
static const int kBtnFineInM2  = 29;
static const int kBtnFineOutM2 = 31;

// ==========================================================
// ================= hardware interface ======================
// ==========================================================

struct GripperMotor {
  uint8_t id;
  HardwareSerial* port;
  bool closing;     // is currently commanded to close
  bool contact;     // has reached contact threshold
  uint16_t pos;     // cached position
  uint16_t load;    // cached load
};

static GripperMotor motors[kMotorCount] = {
  {kMotorId1, &Serial1, false, false, 0, 0},  // Motor 1
  {kMotorId2, &Serial2, false, false, 0, 0}   // Motor 2 (index finger in your design)
};

static bool     gGlobalClosing = false;
static uint32_t gLastMonitorMs = 0;

// Button edge detection
static bool lastOpenState      = HIGH;
static bool lastGripState      = HIGH;
static bool lastStopState      = HIGH;
static bool lastSyncState      = HIGH;
static bool lastFineInM1State  = HIGH;
static bool lastFineOutM1State = HIGH;
static bool lastFineInM2State  = HIGH;
static bool lastFineOutM2State = HIGH;

// ==========================================================
// ========= hardware interface: MD20 LV-TTL protocol =========
// ==========================================================

static uint8_t md20Checksum(uint8_t id, uint8_t len, uint8_t func,
                            const uint8_t* params, uint8_t paramLen) {
  uint16_t sum = id + len + func;
  for (uint8_t i = 0; i < paramLen; i++) sum += params[i];
  return (uint8_t)(~(sum & 0xFF));
}

static void md20SendPacket(HardwareSerial& port,
                           uint8_t id, uint8_t func,
                           const uint8_t* params, uint8_t paramLen) {
  const uint8_t len = paramLen + 2;
  const uint8_t checksum = md20Checksum(id, len, func, params, paramLen);

  port.write(0xFF);
  port.write(0xFF);
  port.write(id);
  port.write(len);
  port.write(func);
  for (uint8_t i = 0; i < paramLen; i++) port.write(params[i]);
  port.write(checksum);
}

static bool md20ReadPosSpdLoad(HardwareSerial& port, uint8_t id,
                               uint16_t& pos,
                               uint16_t& spd,
                               uint16_t& load,
                               uint8_t&  statusOut) {
  // Flush old bytes
  while (port.available() > 0) (void)port.read();

  // Read 8 bytes from start address 0x38
  uint8_t params[2];
  params[0] = 0x38;
  params[1] = 0x08;
  md20SendPacket(port, id, 0x02, params, 2);

  const uint8_t expectedBytes = 14; // header(2) + id + len + status + data(8) + checksum
  const uint32_t timeoutMs = 100;

  const uint32_t t0 = millis();
  while (port.available() < expectedBytes && (millis() - t0) < timeoutMs) {}

  if (port.available() < expectedBytes) return false;

  const uint8_t h1 = port.read();
  const uint8_t h2 = port.read();
  if (h1 != 0xDD || h2 != 0xDD) return false;

  const uint8_t recvId = port.read();
  (void)port.read();          // len (unused)
  const uint8_t status = port.read();
  if (recvId != id) return false;

  uint8_t data[8];
  for (uint8_t i = 0; i < 8; i++) data[i] = port.read();
  (void)port.read();          // checksum (not verified)

  statusOut = status;
  pos  = ((uint16_t)data[0] << 8) | data[1];
  spd  = ((uint16_t)data[2] << 8) | data[3];
  load = ((uint16_t)data[4] << 8) | data[5];
  return true;
}

static void md20WritePosSpeed(HardwareSerial& port, uint8_t id,
                              uint16_t pos, uint16_t speed) {
  uint8_t params[5];
  params[0] = 0x2A;
  params[1] = (pos >> 8) & 0xFF;
  params[2] = pos & 0xFF;
  params[3] = (speed >> 8) & 0xFF;
  params[4] = speed & 0xFF;
  md20SendPacket(port, id, 0x03, params, sizeof(params));
}

static void md20StopAtCurrent(GripperMotor& m) {
  uint16_t pos, spd, load;
  uint8_t status;
  if (!md20ReadPosSpdLoad(*m.port, m.id, pos, spd, load, status)) {
    pos = m.pos; // fallback to cached pos
  }
  md20WritePosSpeed(*m.port, m.id, pos, 0);
}

// ==========================================================
// ================== motion commands ========================
// ==========================================================

static void openGripper() {
  Serial.println(F("[GRIPPER] Open -> POS_MIN"));
  for (uint8_t i = 0; i < kMotorCount; i++) {
    motors[i].closing = false;
    motors[i].contact = false;
    md20WritePosSpeed(*motors[i].port, motors[i].id, kPosMin, kSpeedOpen);
  }
  gGlobalClosing = false;
}

static void startCloseGripper() {
  Serial.println(F("[GRIPPER] Close -> POS_MAX (stop on load threshold)"));
  for (uint8_t i = 0; i < kMotorCount; i++) {
    motors[i].closing = true;
    motors[i].contact = false;
    md20WritePosSpeed(*motors[i].port, motors[i].id, kPosMax, kSpeedClose);
  }
  gGlobalClosing = true;
}

// ==========================================================
// ================ safety / stop logic =====================
// ==========================================================

static void emergencyStopAll() {
  Serial.println(F("[GRIPPER] EMERGENCY STOP!"));
  for (uint8_t i = 0; i < kMotorCount; i++) {
    md20StopAtCurrent(motors[i]);
    motors[i].closing = false;
    motors[i].contact = true; // treat as "done" to stop logic
  }
  gGlobalClosing = false;
}

// ==========================================================
// ===================== sync logic ==========================
// ==========================================================

// PWM follower: map Motor2 position ratio -> 3 PWM pulses
static int clampInt(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static uint16_t clampU16(uint16_t v, uint16_t lo, uint16_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static void pwmFollowMotor2() {
  // IMPORTANT: "Motor2" is motors[1]
  uint16_t pos2 = motors[1].pos;
  pos2 = clampU16(pos2, kPosMin, kPosMax);

  Serial.print(F("[PWM] Follow Motor2 pos="));
  Serial.println(pos2);

  const long denom = (long)(kPosMax - kPosMin);
  if (denom == 0) return;

  for (uint8_t i = 0; i < kPwmCount; i++) {
    const long num = (long)(pos2 - kPosMin) * (kPwmPulseAtPosMax[i] - kPwmPulseAtPosMin[i]);
    int target = kPwmPulseAtPosMin[i] + (int)(num / denom);
    target = clampInt(target, kPwmPulseSafeMin, kPwmPulseSafeMax);

    pwmServos[i].writeMicroseconds(target);

    Serial.print(F("[PWM] Motor"));
    Serial.print(i + 1);
    Serial.print(F(" pulse="));
    Serial.println(target);
  }
}

// ==========================================================
// ================== fine adjustment ========================
// ==========================================================

static void fineAdjust(uint8_t motorIndex, bool inward) {
  if (motorIndex >= kMotorCount) return;

  GripperMotor& m = motors[motorIndex];
  uint16_t pos = clampU16(m.pos, kPosMin, kPosMax);

  Serial.print(F("[FINE] M"));
  Serial.print(motorIndex + 1);
  Serial.print(inward ? F(" IN  ") : F(" OUT "));
  Serial.print(F("pos="));
  Serial.println(pos);

  if (inward) {
    if (pos >= kPosMax) {
      Serial.println(F("[FINE] Already at POS_MAX."));
      return;
    }
    uint16_t target = (pos <= kFineFastRegionEnd) ? (pos + kFineStep) : kPosMax;
    if (target > kPosMax) target = kPosMax;

    Serial.print(F("[FINE] target="));
    Serial.println(target);

    m.closing = false;
    md20WritePosSpeed(*m.port, m.id, target, kSpeedClose);
  } else {
    if (pos <= kPosMin) {
      Serial.println(F("[FINE] Already at POS_MIN."));
      return;
    }
    uint16_t target = (pos > (kPosMin + kFineStep)) ? (pos - kFineStep) : kPosMin;
    if (target < kPosMin) target = kPosMin;

    Serial.print(F("[FINE] target="));
    Serial.println(target);

    m.closing = false;
    md20WritePosSpeed(*m.port, m.id, target, kSpeedOpen);
  }
}

// Convenience wrappers (keep your original semantics)
static void fineAdjustInMotor1()  { fineAdjust(0, true);  }
static void fineAdjustOutMotor1() { fineAdjust(0, false); }
static void fineAdjustInMotor2()  { fineAdjust(1, true);  }
static void fineAdjustOutMotor2() { fineAdjust(1, false); }

// ==========================================================
// ================== motion commands: button handler =========
// ==========================================================

static void handleButtons() {
  const bool openState      = digitalRead(kBtnOpen);
  const bool gripState      = digitalRead(kBtnGrip);
  const bool stopState      = digitalRead(kBtnStop);
  const bool syncState      = digitalRead(kBtnSync);
  const bool fineInM1State  = digitalRead(kBtnFineInM1);
  const bool fineOutM1State = digitalRead(kBtnFineOutM1);
  const bool fineInM2State  = digitalRead(kBtnFineInM2);
  const bool fineOutM2State = digitalRead(kBtnFineOutM2);

  // Detect HIGH -> LOW edge (pressed)
  if (lastOpenState == HIGH && openState == LOW) {
    Serial.println(F("[BTN] OPEN"));
    openGripper();
  }
  if (lastGripState == HIGH && gripState == LOW) {
    Serial.println(F("[BTN] GRIP"));
    startCloseGripper();
  }
  if (lastStopState == HIGH && stopState == LOW) {
    Serial.println(F("[BTN] STOP"));
    emergencyStopAll();
  }
  if (lastSyncState == HIGH && syncState == LOW) {
    Serial.println(F("[BTN] SYNC (PWM follow)"));
    pwmFollowMotor2();
  }
  if (lastFineInM1State == HIGH && fineInM1State == LOW) {
    Serial.println(F("[BTN] FINE IN M1"));
    fineAdjustInMotor1();
  }
  if (lastFineOutM1State == HIGH && fineOutM1State == LOW) {
    Serial.println(F("[BTN] FINE OUT M1"));
    fineAdjustOutMotor1();
  }
  if (lastFineInM2State == HIGH && fineInM2State == LOW) {
    Serial.println(F("[BTN] FINE IN M2"));
    fineAdjustInMotor2();
  }
  if (lastFineOutM2State == HIGH && fineOutM2State == LOW) {
    Serial.println(F("[BTN] FINE OUT M2"));
    fineAdjustOutMotor2();
  }

  lastOpenState      = openState;
  lastGripState      = gripState;
  lastStopState      = stopState;
  lastSyncState      = syncState;
  lastFineInM1State  = fineInM1State;
  lastFineOutM1State = fineOutM1State;
  lastFineInM2State  = fineInM2State;
  lastFineOutM2State = fineOutM2State;
}

// ==========================================================
// ==================== serial debug =========================
// ==========================================================

static void printMotorTelemetry(uint8_t motorIndex, uint16_t pos, uint16_t load, uint8_t status) {
  Serial.print(F("[M"));
  Serial.print(motorIndex + 1);
  Serial.print(F("] Pos="));
  Serial.print(pos);
  Serial.print(F(" Load="));
  Serial.print(load);
  Serial.print(F(" Status="));
  Serial.println(status);
}

// ==========================================================
// ================== Arduino entry points ===================
// ==========================================================

static void printPinMap() {
  Serial.println(F("=== Pin Map (buttons, INPUT_PULLUP) ==="));
  Serial.print(F(" OPEN  : D")); Serial.println(kBtnOpen);
  Serial.print(F(" GRIP  : D")); Serial.println(kBtnGrip);
  Serial.print(F(" STOP  : D")); Serial.println(kBtnStop);
  Serial.print(F(" SYNC  : D")); Serial.println(kBtnSync);
  Serial.print(F(" FINE IN  M1 : D")); Serial.println(kBtnFineInM1);
  Serial.print(F(" FINE OUT M1 : D")); Serial.println(kBtnFineOutM1);
  Serial.print(F(" FINE IN  M2 : D")); Serial.println(kBtnFineInM2);
  Serial.print(F(" FINE OUT M2 : D")); Serial.println(kBtnFineOutM2);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial2.begin(115200);

  // Attach PWM servos, init to "pos=min" pulse
  for (uint8_t i = 0; i < kPwmCount; i++) {
    pwmServos[i].attach(kPwmPins[i]);
    pwmServos[i].writeMicroseconds(kPwmPulseAtPosMin[i]);
  }

  // Buttons
  pinMode(kBtnOpen,        INPUT_PULLUP);
  pinMode(kBtnGrip,        INPUT_PULLUP);
  pinMode(kBtnStop,        INPUT_PULLUP);
  pinMode(kBtnSync,        INPUT_PULLUP);
  pinMode(kBtnFineInM1,    INPUT_PULLUP);
  pinMode(kBtnFineOutM1,   INPUT_PULLUP);
  pinMode(kBtnFineInM2,    INPUT_PULLUP);
  pinMode(kBtnFineOutM2,   INPUT_PULLUP);

  Serial.println(F("=== Dual MD20 (LV-TTL) + 3xPWM + Buttons + Fine Adjust Ready ==="));
  printPinMap();

  openGripper();
}

void loop() {
  // 1) Physical buttons
  handleButtons();

  // 2) Serial debug command fallback
  if (Serial.available() > 0) {
    const char c = (char)Serial.read();
    if      (c == '1') openGripper();
    else if (c == '2') startCloseGripper();
    else if (c == '3') emergencyStopAll();
    else if (c == '4') pwmFollowMotor2();
  }

  // 3) LV-TTL monitoring + safety / stop logic
  const uint32_t now = millis();
  if (now - gLastMonitorMs >= kMonitorIntervalMs) {
    gLastMonitorMs = now;

    bool allContact = true;

    for (uint8_t i = 0; i < kMotorCount; i++) {
      uint16_t pos, spd, load;
      uint8_t status;
      const bool ok = md20ReadPosSpdLoad(*motors[i].port, motors[i].id, pos, spd, load, status);

      if (ok) {
        motors[i].pos  = pos;
        motors[i].load = load;

        printMotorTelemetry(i, pos, load, status);

        if (gGlobalClosing && motors[i].closing && !motors[i].contact) {
          if (load > kLoadThreshold) {
            Serial.print(F("  -> M"));
            Serial.print(i + 1);
            Serial.println(F(" reached load threshold, stop."));
            motors[i].contact = true;
            motors[i].closing = false;
            md20StopAtCurrent(motors[i]);
          }
        }
      } else {
        Serial.print(F("[M"));
        Serial.print(i + 1);
        Serial.println(F("] read failed."));
      }

      if (!motors[i].contact) allContact = false;
    }

    if (gGlobalClosing && allContact) {
      Serial.println(F("[GRIPPER] Grip complete (both sides contact)."));
      gGlobalClosing = false;
    }
  }
}
