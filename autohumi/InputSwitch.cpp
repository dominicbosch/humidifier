/*
  InputSwitch.cpp - Interface to digital input switches.
*/
#include "Arduino.h"
#include "InputSwitch.h"

InputSwitch::InputSwitch(int pin) {
  pinMode(pin, INPUT);
  _pin = pin;
}

// Since the switches vibrate physically when used, we have to wait for an accurate value
bool InputSwitch::getState() {
  bool state;
  bool previousState = digitalRead(_pin);
  int i = 0;
  // if we read 10 times the same value we are quite sure the button is stable
  while (i++ < 10) {
    delay(1);
    state = digitalRead(_pin);
    if (state != previousState) {
      i = 0;
      previousState = state;
    }
  }
  return state;
}