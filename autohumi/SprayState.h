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

#define RUNMODE_SENSOR 0
#define RUNMODE_TIMER 1

class SprayState {
	public:
		SprayState(Display *displ, int dInletPin = 9, int dOutletPin = 10,
			int threshTemp = 25, int threshHumi = 40,
			int sprayTime = 120, int sprayInterval = 300);
		bool update(int temp, int humi);
		void printStateText(int line);
		void printCountdown(int line);
		void setRunMode(bool runMode);
		int getHumiThresh();
		void setHumiThresh(int newVal);
		int getTempThresh();
		void setTempThresh(int newVal);
		int getSprayTime();
		void setSprayTime(int newVal);
		int getSprayInterval();
		void setSprayInterval(int newVal);

	private:
		Display *_displ;
		OutputSwitch *_inletSwitch;
		OutputSwitch *_outletSwitch;

		bool _runMode;
		int _threshTemp;
		int _threshHumi;
		int _sprayTime;
		int _sprayInterval;
		int _sprayState = 0; // init the spray state

		// times:
		unsigned long _sprayStart = 0;
};

#endif
