/*
  AppState.h - AppState control
*/
#ifndef AppState_h
#define AppState_h

#include "Arduino.h"
#include "InputSwitch.h"

#define STATE_RUNNING 0
#define STATE_SET_RUNMODE 1
#define STATE_SET_TEMP 2
#define STATE_SET_HUMI 3
#define STATE_SET_SPRAYTIME 4
#define STATE_SET_TIMER 5

class AppState {
	public:
		AppState(int pinSwitch = 4);
		int updateState();
		String getStateText();

	private:
		InputSwitch *_toggle;

		bool _lastToggleState = 0;
		int _appState = 0;
};

#endif
