#pragma once
#include <Arduino.h>

void controllerBegin();
bool controllerConnected();

uint8_t controllerThrottle();
uint8_t controllerReverse();
bool    controllerBrake();
int8_t  controllerSteer();
