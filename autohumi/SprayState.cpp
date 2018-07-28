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

bool SprayState::update(int temperature, int humidity) {
  int nowTime = millis();
  bool hasChanged = false;

  // we check spray state only every half second
  if (abs(nowTime - _lastSprayCheck) > 500) {
    // if we are idle and the thresholds are crossed, we start spraying 
    if (_sprayState == SPRAY_STATE_OFF && (temperature > _threshTemp || humidity < _threshHumi)) {
      _inletSwitch->on();
      _sprayStart = nowTime;
      _sprayState = SPRAY_STATE_SPRAYING;
      hasChanged = true;
      
    // if we are spraying and the time is over, we stop spraying and flush
    } else if (_sprayState == SPRAY_STATE_SPRAYING && abs(nowTime - _sprayStart)/1000 > _sprayTime) {
      _inletSwitch->off();
      _outletSwitch->on();
      _sprayState = SPRAY_STATE_FLUSHING;
      hasChanged = true;

    // we flush the pipeline for 30 seconds
    } else if (_sprayState == SPRAY_STATE_FLUSHING && abs(nowTime - _sprayStart)/1000 > (_sprayTime+30)) {
      _outletSwitch->off();
      _sprayState = SPRAY_STATE_OFF;
      hasChanged = true;
    }

    _lastSprayCheck = nowTime;
  }
  return hasChanged;
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

void SprayState::setHumidityThresh(int newVal) {
  _threshHumi = newVal;
}

void SprayState::setTemperatureThresh(int newVal) {
  _threshTemp = newVal;
}

void SprayState::setSprayTime(int newVal) {
  _sprayTime = newVal;
}

