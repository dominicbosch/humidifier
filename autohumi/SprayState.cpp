/*
  SprayState.cpp - SprayState control
*/
#include "Arduino.h"
#include "SprayState.h"
#include "Display.h"
#include "OutputSwitch.h"

SprayState::SprayState(Display *displ, int dInletPin, int dOutletPin, int threshTemp,
    int threshHumi, int sprayTime, int sprayInterval) {
  
  _displ = displ;
  _threshTemp = threshTemp;
  _threshHumi = threshHumi;
  _sprayTime = sprayTime;
  _sprayInterval = sprayInterval;
  _inletSwitch = new OutputSwitch(dInletPin);
  _outletSwitch = new OutputSwitch(dOutletPin);
};

bool SprayState::update(int temperature, int humidity) {
  unsigned long nowTime = millis();
  bool hasChanged = false;

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
  return hasChanged;
}

void SprayState::printStateText(int line) {
  char *buffer = _displ->clearAndGetBufferLine(line);
  switch (_sprayState) {
    case SPRAY_STATE_SPRAYING: snprintf(buffer, BUFFER_LENGTH, "Spraying!"); break;
    case SPRAY_STATE_FLUSHING: snprintf(buffer, BUFFER_LENGTH, "Flushing!"); break;
    case SPRAY_STATE_OFF: snprintf(buffer, BUFFER_LENGTH, "Checking Sensors"); break;
  }
  _displ->printBufferLineAsString(line);
}

void SprayState::printCountdown(int line) {
  unsigned long nowTime = millis();
  char *buffer = _displ->clearAndGetBufferLine(line);
  if (_sprayState == SPRAY_STATE_SPRAYING) {
    snprintf(buffer, BUFFER_LENGTH, "Ends in %3ds", _sprayStart/1000 + _sprayTime - nowTime/1000);
  } else if (_sprayState == SPRAY_STATE_FLUSHING) {
    snprintf(buffer, BUFFER_LENGTH, "Ends in %3ds", _sprayStart/1000 + (_sprayTime+30) - nowTime/1000);
  }
  _displ->printBufferLineAsString(line);
}

int SprayState::getHumiThresh() { return _threshHumi; }
void SprayState::setHumiThresh(int newVal) { _threshHumi = newVal; }

int SprayState::getTempThresh() { return _threshTemp; }
void SprayState::setTempThresh(int newVal) { _threshTemp = newVal; }

int SprayState::getSprayTime() { return _sprayTime; }
void SprayState::setSprayTime(int newVal) { _sprayTime = newVal; }

