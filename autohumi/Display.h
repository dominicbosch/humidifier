/*
  Display.h - Interface to digital input switches.
*/
#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include <U8x8lib.h>

class Display {
  public:
    Display(int clockPin, int dataPin);
	writeString(int num, String txt);
	writeString(int num, char *buffer);
	writeUTF8(int num, String txt);
	writeUTF8(int num, char *buffer);
    clear();
    
  private:
  	U8X8_SSD1306_128X64_NONAME_SW_I2C *_u8x8;
};

#endif
