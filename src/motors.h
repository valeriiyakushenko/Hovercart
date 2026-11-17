#pragma once
#include <Arduino.h>

void motorsBegin();

// limit: share of full PWM duty at full throttle (0..1). Rear-wheel drive leaves the front wheels coasting.
void motorsSetMode(float limit, bool rearWheelDrive);

void motorsUpdate(uint8_t forward, uint8_t reverse, bool brakeFront, bool brakeRear);
