/*
  OutputSwitch.cpp - Interface to digital output switches.
*/
#include "Arduino.h"
#include "OutputSwitch.h"

OutputSwitch::OutputSwitch(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
}

void OutputSwitch::on() {
  digitalWrite(_pin, HIGH);
}

void OutputSwitch::off() {
  digitalWrite(_pin, LOW);
}
