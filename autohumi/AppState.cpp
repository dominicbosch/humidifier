/*
  AppState.cpp - AppState control
*/
#include "Arduino.h"
#include "Display.h"
#include "AppState.h"
#include "InputSwitch.h"

AppState::AppState(Display *displ, int pinSwitch) {
  _displ = displ;
  _toggle = new InputSwitch(pinSwitch);
};

// Check whether the state button has been pressed
int AppState::updateState() {
  bool stateSwitched = false;
  // Check whether the user whishes to switch the state
  bool buttonState = _toggle->getState();
  if (buttonState != _lastToggleState) {
    _lastToggleState = buttonState; 
    if (buttonState == InputSwitch::STATE_PRESSED) {
      stateSwitched = true;
      _state++;
      if (_state > STATE_SET_TIMER) _state = STATE_RUNNING;
    }
  }
  return stateSwitched;
}

int AppState::getState() { return _state; }

void AppState::printStateText(int line, const char *runmode) {
  char *buffer = _displ->clearAndGetBufferLine(line);
  switch (_state) {
    case STATE_RUNNING: snprintf(buffer, BUFFER_LENGTH, runmode); break;
    case STATE_SET_RUNMODE: snprintf(buffer, BUFFER_LENGTH, "Set Run Mode!"); break;
    case STATE_SET_TEMP: snprintf(buffer, BUFFER_LENGTH, "Set Temperature!"); break;
    case STATE_SET_HUMI: snprintf(buffer, BUFFER_LENGTH, "Set Humidity!"); break;
    case STATE_SET_SPRAYTIME: snprintf(buffer, BUFFER_LENGTH, "Set Spray time!"); break;
    case STATE_SET_TIMER: snprintf(buffer, BUFFER_LENGTH, "Set Timer!"); break;
  }
  _displ->printBufferLineAsString(line);
}
