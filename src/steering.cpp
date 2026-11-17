#include "steering.h"
#include "wiring.h"
#include <ESP32Servo.h>

constexpr int CENTER_ANGLE = 90;
constexpr int MAX_TURN     = 85;
constexpr float DEADZONE_PERCENT = 2.5f;

static Servo servo;

void steeringBegin() {
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(CENTER_ANGLE);
}

void steeringUpdate(int8_t stickX) {
  int x = abs(stickX) < 128 * DEADZONE_PERCENT / 100.0f ? 0 : stickX;
  servo.write(map(x, -128, 127, CENTER_ANGLE + MAX_TURN, CENTER_ANGLE - MAX_TURN));
}
