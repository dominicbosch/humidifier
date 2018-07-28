/*
  SprayState.cpp - SprayState control
*/
#include "Arduino.h"
#include "SprayState.h"
#include "Display.h"
#include "OutputSwitch.h"

SprayState::SprayState(int dInletPin, int dOutletPin, int threshTemp,
    int threshHumi, int sprayTime, int sprayInterval) {
  
  _threshTemp = threshTemp;
  _threshHumi = threshHumi;
  _sprayTime = sprayTime;
  _sprayInterval = sprayInterval;
  _inletSwitch = new OutputSwitch(dInletPin);
  _outletSwitch = new OutputSwitch(dOutletPin);
};

bool weShouldCheckSprayState() {
  int nowTime = millis();
  // it has been a while since last check
  if (abs(nowTime - _lastSprayCheck) > 500) {
    // we are either spraying or flushing
    if ( _sprayState == SPRAY_STATE_SPRAYING 
        || _sprayState == SPRAY_STATE_FLUSHING) {
      return true;
    }
  }
  return false;
}

SprayState::update(int temp, int humi) {
  int nowTime = millis();
  _lastSprayCheck = nowTime;

  // if we are idle and the thresholds are crossed, we start spraying 
  if (_sprayState == SPRAY_STATE_OFF && (_temperature > _threshTemp || _humidity < _threshHumi)) {
    _inletSwitch->on();
    _sprayStart = nowTime;
    _sprayState = SPRAY_STATE_SPRAYING;
    _displ->drawString(3, "Spraying!  ");
    
  // if we are spraying and the time is over, we stop spraying and flush
  } else if (_sprayState == SPRAY_STATE_SPRAYING && abs(nowTime - _sprayStart)/1000 > _sprayTime) {
    _inletSwitch->off();
    _outletSwitch->on();
    _sprayState = SPRAY_STATE_FLUSHING;
    _displ->drawString(3, "Flushing!  ");

  // we flush the pipeline for 30 seconds
  } else if (_sprayState == SPRAY_STATE_FLUSHING && abs(nowTime - _sprayStart)/1000 > (_sprayTime+30)) {
    _outletSwitch->off();
    _sprayState = SPRAY_STATE_OFF;
    _displ->drawString(3, "Watching...");
    _displ->drawString(4, "");
  }
  
  if (_sprayState == SPRAY_STATE_SPRAYING) {
    sprintf(oledLine, "Ends in %3ds", _sprayStart/1000 + _sprayTime - nowTime/1000);
    _displ->drawString(4, oledLine);
  } else if (_sprayState == SPRAY_STATE_FLUSHING) {
    sprintf(oledLine, "Ends in %3ds", _sprayStart/1000 + (_sprayTime+30) - nowTime/1000);
    _displ->drawString(4, oledLine);
  }
}


String SprayState::getStateText() {
  String text = "Unknown state...";
  switch (_appState) {
    case SPRAY_STATE_SPRAYING: text = "Spraying!"; break;
    case SPRAY_STATE_FLUSHING: text = "Flushing!"; break;
    case SPRAY_STATE_OFF: text = "Watching!"; break;
  }
  return text;
}
