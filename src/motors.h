#pragma once
#include <Arduino.h>

void motorsBegin();

void motorsUpdate(uint8_t forward, uint8_t reverse, bool brake);
