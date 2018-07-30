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

// FIXME use char array: const char *s
void AppState::printStateText(int line) {
  // char buffer[16] = "Unknown state..."; // FIXME, buffer needs to be declared by caller
  // switch (_appState) {
  //   case STATE_RUNNING: strncpy(buffer, "Running State!"); break;
  //   case STATE_SET_RUNMODE: strncpy(buffer, "Set Run Mode!"); break;
  //   case STATE_SET_TEMP: strncpy(buffer, "Set Temperature!"); break;
  //   case STATE_SET_HUMI: strncpy(buffer, "Set Humidity!"); break;
  //   case STATE_SET_SPRAYTIME: strncpy(buffer, "Set Spray time!"); break;
  //   case STATE_SET_TIMER: strncpy(buffer, "Set Timer!"); break;
  // }
  // return buffer;
}
