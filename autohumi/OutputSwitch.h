/*
  OutputSwitch.h - Interface to digital output switches.
*/
#ifndef OutputSwitch_h
#define OutputSwitch_h

#include "Arduino.h"

class OutputSwitch {
  public:
    OutputSwitch(int pin);
    void on();
    void off();
  private:
    int _pin;
};

#endif