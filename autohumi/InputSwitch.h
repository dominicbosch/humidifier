/*
  InputSwitch.h - Interface to digital input switches.
*/
#ifndef InputSwitch_h
#define InputSwitch_h

#include "Arduino.h"

class InputSwitch {
  public:
    InputSwitch(int pin);
    bool getState();
  private:
    int _pin;
};

#endif