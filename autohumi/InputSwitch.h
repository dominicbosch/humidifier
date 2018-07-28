/*
  InputSwitch.h - Interface to digital input switches.
*/
#ifndef InputSwitch_h
#define InputSwitch_h

#include "Arduino.h"

class InputSwitch {
	public:
		static const int STATE_RELEASED = 0;
		static const int STATE_PRESSED = 1;
		InputSwitch(int pin);
		bool getState();
	private:
		int _pin;
};

#endif