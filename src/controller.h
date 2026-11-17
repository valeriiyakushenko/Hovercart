#pragma once
#include <Arduino.h>

void controllerBegin();
bool controllerConnected();

uint8_t controllerThrottle();
uint8_t controllerReverse();
bool    controllerBrake();       // R1
bool    controllerFrontBrake();  // L1
int8_t  controllerSteer();

bool controllerSticksPressed();  // L3 and R3 together
bool controllerLeft();
bool controllerRight();
bool controllerCross();

// Lightbar colour; cheap to call every loop, only sends on change.
void controllerSetLed(uint8_t r, uint8_t g, uint8_t b);
