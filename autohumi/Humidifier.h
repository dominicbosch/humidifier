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

    void _updateMeasurements();
    void _readTempAndHumi();
    void _printDHTonOLED();
    void _updateStateValue();
    int _readStablePotiValue();
};

#endif
