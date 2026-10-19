#include "controller.h"
#include <ps5Controller.h>

void controllerBegin() {
  ps5.begin(CONTROLLER_MAC);
}

bool controllerConnected() { return ps5.isConnected(); }

uint8_t controllerThrottle() { return ps5.R2Value(); }
uint8_t controllerReverse()  { return ps5.L2Value(); }
bool    controllerBrake()    { return ps5.R1(); }
int8_t  controllerSteer()    { return ps5.LStickX(); }
