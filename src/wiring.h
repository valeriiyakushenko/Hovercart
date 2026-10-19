#pragma once
#include <Arduino.h>

struct MotorPins {
  uint8_t pwm;
  uint8_t dir;
  uint8_t brake;
  uint8_t speed;
  bool inverted;
};

constexpr MotorPins MOTORS[] = {
  //  pwm  dir  brake  speed  inverted
  {   25,  17,   16,    34,   true  },   // LF
  {   26,  13,    4,    35,   false },   // RF
  {   27,  19,   18,    36,   true  },   // LB
  {   33,  32,   23,    39,   false },   // RB
};
constexpr size_t MOTOR_COUNT = sizeof(MOTORS) / sizeof(MOTORS[0]);

constexpr uint8_t SERVO_PIN = 14;

constexpr uint8_t STATUS_LED = 2;

constexpr bool DIR_FORWARD_LEVEL  = HIGH;
constexpr bool BRAKE_ACTIVE_LEVEL = LOW;