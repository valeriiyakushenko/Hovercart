#include "motors.h"
#include "wiring.h"

constexpr uint32_t PWM_FREQ        = 15000;
constexpr uint8_t  PWM_BITS        = 10;
constexpr uint32_t PWM_MAX         = (1 << PWM_BITS) - 1;
constexpr uint8_t  PWM_CHANNEL_0   = 8;
constexpr uint32_t STOP_TIMEOUT_MS = 200;
constexpr uint32_t SETTLE_MS       = 100;

enum class Direction { Forward, Reverse };
enum class Phase { Drive, Braking, Settling };

static Direction currentDir = Direction::Forward;
static Phase phase = Phase::Drive;
static uint32_t settleStartMs;
static volatile uint32_t lastPulseMs[MOTOR_COUNT];

static void IRAM_ATTR onPulse(void* arg) {
  *static_cast<volatile uint32_t*>(arg) = millis();
}

static bool allWheelsStopped() {
  for (size_t i = 0; i < MOTOR_COUNT; i++) {
    uint32_t last = lastPulseMs[i];
    if (millis() - last < STOP_TIMEOUT_MS) return false;
  }
  return true;
}

static void setDirection(Direction dir) {
  for (const MotorPins& m : MOTORS) {
    bool forward = (dir == Direction::Forward) != m.inverted;
    digitalWrite(m.dir, forward ? DIR_FORWARD_LEVEL : !DIR_FORWARD_LEVEL);
  }
  currentDir = dir;
}

static void setBrake(bool on) {
  for (const MotorPins& m : MOTORS) {
    digitalWrite(m.brake, on ? BRAKE_ACTIVE_LEVEL : !BRAKE_ACTIVE_LEVEL);
  }
}

static void setPower(uint8_t value) {
  float t = value / 255.0f;
  uint32_t duty = t * t * PWM_MAX;
  for (size_t i = 0; i < MOTOR_COUNT; i++) {
    ledcWrite(PWM_CHANNEL_0 + i, duty);
  }
}

void motorsBegin() {
  for (size_t i = 0; i < MOTOR_COUNT; i++) {
    const MotorPins& m = MOTORS[i];
    ledcSetup(PWM_CHANNEL_0 + i, PWM_FREQ, PWM_BITS);
    ledcAttachPin(m.pwm, PWM_CHANNEL_0 + i);
    pinMode(m.dir, OUTPUT);
    pinMode(m.brake, OUTPUT);
    pinMode(m.speed, INPUT);
    attachInterruptArg(m.speed, onPulse, (void*)&lastPulseMs[i], CHANGE);
  }
  setPower(0);
  setDirection(Direction::Forward);
  setBrake(false);
}

void motorsUpdate(uint8_t forward, uint8_t reverse, bool brake) {
  if (brake) {
    phase = Phase::Drive;
    setBrake(true);
    setPower(0);
    return;
  }

  bool wantForward = forward > 0;
  uint8_t throttle = wantForward ? forward : reverse;
  Direction wanted = wantForward ? Direction::Forward : Direction::Reverse;

  if (throttle == 0 || wanted == currentDir) {
    phase = Phase::Drive;
    setBrake(false);
    setPower(throttle);
    return;
  }

  setPower(0);
  if (phase == Phase::Drive) phase = Phase::Braking;

  if (phase == Phase::Braking) {
    setBrake(true);
    if (!allWheelsStopped()) return;
    setBrake(false);
    settleStartMs = millis();
    phase = Phase::Settling;
    return;
  }

  if (millis() - settleStartMs < SETTLE_MS) return;
  setDirection(wanted);
  phase = Phase::Drive;
  setPower(throttle);
}
