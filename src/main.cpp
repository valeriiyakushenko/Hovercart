#include <Arduino.h>
#include "controller.h"
#include "motors.h"
#include "steering.h"
#include "wiring.h"

constexpr uint32_t BLINK_MS = 300;
constexpr uint32_t MENU_HOLD_MS  = 3000;
constexpr uint32_t MENU_BLINK_MS = 500;

struct Mode {
  uint8_t r, g, b;
  float limit;
  bool rearOnly;
};
constexpr Mode MODES[] = {
  //  r    g    b   limit  rearOnly
  {   0, 255,   0, 0.25f, false },   // standard
  { 128,   0, 255, 1.0f,  false },   // performance
  { 255,   0,   0, 1.0f,  true  },   // drift
};
constexpr size_t MODE_COUNT = sizeof(MODES) / sizeof(MODES[0]);

static size_t mode = 0;
static bool menuOpen = false;
static uint32_t holdStartMs;
static bool prevLeft, prevRight;

// Returns true while the mode menu is open; the car must stay braked then.
// idle = both triggers released: the menu only opens and closes at zero throttle.
static bool updateModeMenu(bool idle) {
  uint32_t now = millis();
  bool left = controllerLeft(), right = controllerRight();
  if (!menuOpen) {
    if (!controllerSticksPressed() || !idle) holdStartMs = now;
    if (now - holdStartMs >= MENU_HOLD_MS) menuOpen = true;
  } else {
    if (left && !prevLeft)   mode = (mode + MODE_COUNT - 1) % MODE_COUNT;
    if (right && !prevRight) mode = (mode + 1) % MODE_COUNT;
    if (controllerCross() && idle) {
      menuOpen = false;
      holdStartMs = now;
    }
  }
  prevLeft = left;
  prevRight = right;

  const Mode& m = MODES[mode];
  bool lit = !menuOpen || (now / MENU_BLINK_MS) % 2;
  controllerSetLed(lit ? m.r : 0, lit ? m.g : 0, lit ? m.b : 0);
  motorsSetMode(m.limit, m.rearOnly);
  return menuOpen;
}

static void updateStatusLed(bool connected) {
  bool blink = (millis() / BLINK_MS) % 2;
  digitalWrite(STATUS_LED, connected || blink);
}

void setup() {
  Serial.begin(115200);
  pinMode(STATUS_LED, OUTPUT);
  steeringBegin();
  motorsBegin();
  controllerBegin();
}

void loop() {
  bool connected = controllerConnected();
  updateStatusLed(connected);

  if (connected) {
    uint8_t throttle = controllerThrottle(), reverse = controllerReverse();
    bool menu = updateModeMenu(throttle == 0 && reverse == 0);
    // Drift: L1 brakes the front axle (burnout), R1 the rear. Other modes: R1 brakes all wheels.
    bool r1 = controllerBrake();
    bool frontBrake = MODES[mode].rearOnly ? controllerFrontBrake() : r1;
    motorsUpdate(throttle, reverse, frontBrake || menu, r1 || menu);
    steeringUpdate(controllerSteer());
  } else {
    motorsUpdate(0, 0, true, true);
    steeringUpdate(0);
  }
  delay(10);
}