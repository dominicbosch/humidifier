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
      _appState++;
      if (_appState > STATE_SET_TIMER) _appState = STATE_RUNNING;
    }
  }
  return stateSwitched;
}

int AppState::getState() { return _appState; }

void AppState::printStateText(int line) {
  char *buffer = _displ->getBufferLine(line);
  switch (_appState) {
    case STATE_RUNNING: snprintf(buffer, BUFFER_SIZE, "Running State!"); break;
    case STATE_SET_RUNMODE: snprintf(buffer, BUFFER_SIZE, "Set Run Mode!"); break;
    case STATE_SET_TEMP: snprintf(buffer, BUFFER_SIZE, "Set Temperature!"); break;
    case STATE_SET_HUMI: snprintf(buffer, BUFFER_SIZE, "Set Humidity!"); break;
    case STATE_SET_SPRAYTIME: snprintf(buffer, BUFFER_SIZE, "Set Spray time!"); break;
    case STATE_SET_TIMER: snprintf(buffer, BUFFER_SIZE, "Set Timer!"); break;
  }
  _displ->printBufferLineAsString(line);
}
