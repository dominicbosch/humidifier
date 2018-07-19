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
    // writeLine(int num);
    writeLine(int num, String txt);
    // clearLine(int num);
    clear();
  private:
  	U8X8_SSD1306_128X64_NONAME_SW_I2C *_u8x8;
    virtual void f() {};
};

#endif
