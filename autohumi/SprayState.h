/*
	SprayState.h - SprayState control
*/
#ifndef SprayState_h
#define SprayState_h

#include "Arduino.h"
#include "Display.h"
#include "OutputSwitch.h"

#define SPRAY_STATE_OFF 0
#define SPRAY_STATE_SPRAYING 1
#define SPRAY_STATE_FLUSHING 2

class SprayState {
	public:
		SprayState(int dInletPin = 9, int dOutletPin = 10,
			int threshTemp = 25, int threshHumi = 40,
			int sprayTime = 120, int sprayInterval = 300);
		bool update(int temp, int humi);
		String getStateText();
		void setHumidityThresh(int newVal);
		void setTemperatureThresh(int newVal);
		void setSprayTime(int newVal);

	private:
		OutputSwitch *_inletSwitch;
		OutputSwitch *_outletSwitch;

		int _threshTemp;
		int _threshHumi;
		int _sprayTime;
		int _sprayInterval;
		int _sprayState = 0; // init the spray state

		// times:
		unsigned long _lastSprayCheck = 0;
		unsigned long _sprayStart = 0;
};

#endif
