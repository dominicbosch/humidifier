/*
  AppState.cpp - AppState control
*/
#include "Arduino.h"
#include "AppState.h"

AppState::AppState(int pinSwitch) {
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
      if (_appState > MAX_STATE) _appState = 0;
    }
  }
  return stateSwitched;
}

String AppState::getStateText() {
  String text = "Unknown state...";
  switch (_appState) {
    case STATE_RUNNING: text = "AppState Active!"; break;
    case STATE_SET_RUNMODE: text = "Set Run Mode!"; break;
    case STATE_SET_TEMP: text = "Set Temperature!"; break;
    case STATE_SET_HUMI: text = "Set Humidity!"; break;
    case STATE_SET_SPRAYTIME: text = "Set Spray time!"; break;
    case STATE_SET_TIMER: text = "Set Timer!"; break;
  }
  return text;
}
