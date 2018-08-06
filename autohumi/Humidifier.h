/*
  Humidifier.h - The main object for the humidifier
*/
#ifndef Humidifier_h
#define Humidifier_h

#include <DHT.h>
#include "Arduino.h"
#include "AppState.h"
#include "SprayState.h"
#include "Display.h"

#define DHTTYPE DHT11

#define LINE_TEMP 0
#define LINE_HUMI 1
#define LINE_STATE 3
#define LINE_SETTING 4
#define LINE_SPRAYSTATE 6
#define LINE_SPRAYCOUNT 7

class Humidifier {
  public:
    Humidifier();
    void loop();

  private:
    DHT *_dht;
    Display *_displ;
    AppState *_appState;
    SprayState *_sprayState;

    int _minTemp = 10;
    int _maxTemp = 40;
    int _nowTemp = 10;
    int _nowHumidity = 0;
    int _nowPoti = 0;
    int _aPotiPin = 7;
    bool _potiChanged = 0;
    int _initialPotiVal = 0;
    unsigned long _lastTempCheck = 0;
    unsigned long _lastSprayCheck = 0;

    void _updateTempAndHumi();
    void _printTempAndHumi();
    bool _potiHasChanged(bool stateSwitched);
    void _updateSettingValue();
    int _readStablePotiValue();
};

#endif
