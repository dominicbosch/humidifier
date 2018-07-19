/*
  Humidifier.h - Humidifier control
*/
#ifndef Humidifier_h
#define Humidifier_h

#include "Arduino.h"
#include <DHT.h>
#include "Display.h"
#include "InputSwitch.h"
#include "OutputSwitch.h"

#define DHTTYPE DHT11

#define STATE_RUNNING 0
#define STATE_SET_RUNMODE 1
#define STATE_SET_TEMP 2
#define STATE_SET_HUMI 3
#define STATE_SET_SPRAYTIME 4
#define STATE_SET_TIMER 5
#define MAX_STATE 5

class Humidifier {
  public:
    Humidifier(int aPotiPin=7, int dDhtPin=2, int dSwitchPin=4, int dInletPin=9,
    	int dOutletPin=10, int dOledClockPin=19, int dOledDataPin=18, int minTemp=10,
    	int maxTemp=40, int threshTemp=25, int threshHumi=40, int sprayTime=120);
    void loop();
   private:

	Display *displ;
	InputSwitch *toggle;
	OutputSwitch *inletSwitch;
	OutputSwitch *outletSwitch;

   	bool _lastToggleState = 0;
	int _temperature = 0;
	int _humidity = 0;
	int _appState = 0; // init the app state
	int _sprayState = 0; // init the spray state

	// times:
	unsigned long _nowTime = 0;
	unsigned long _lastSprayCheck = 0;
	unsigned long _lastTempCheck = 0;
	unsigned long _sprayStart = 0;

	// OLED line buffers
	//char oledLines[4][16] = {""};

};

#endif
