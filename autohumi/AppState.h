/*
  AppState.h - AppState control
*/
#ifndef AppState_h
#define AppState_h

#include "Arduino.h"
#include "Display.h"
#include "InputSwitch.h"

#define STATE_RUNNING 0
#define STATE_SET_TEMP 1
#define STATE_SET_HUMI 2
#define STATE_SET_SPRAYTIME 3
#define STATE_SET_RUNMODE 4
#define STATE_SET_TIMER 5

class AppState {
	public:
		AppState(Display *displ, int pinSwitch = 4);
		int updateState();
		int getState();
		void printStateText(int line, const char *runmode = "");

	private:
		Display *_displ;
		InputSwitch *_toggle;

		bool _lastToggleState = 0;
		int _state = 0;
};

#endif
