#include <Arduino.h>
#include "controller.h"
#include "motors.h"
#include "steering.h"
#include "wiring.h"

constexpr uint32_t BLINK_MS = 300;

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
    motorsUpdate(controllerThrottle(), controllerReverse(), controllerBrake());
    steeringUpdate(controllerSteer());
  } else {
    motorsUpdate(0, 0, true);
    steeringUpdate(0);
  }
  delay(10);
}