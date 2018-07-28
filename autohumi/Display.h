/*
  Display.h - Interface to digital input switches.
*/
#ifndef Display_h
#define Display_h

#include "Arduino.h"
#include <U8x8lib.h>

class Display {
  public:
    Display(int clockPin = 19, int dataPin = 18);
    void writeString(int num, String txt);
    void writeString(int num, char *buffer);
    void writeUTF8(int num, String txt);
    void writeUTF8(int num, char *buffer);
    void clear();

  private:
    U8X8_SSD1306_128X64_NONAME_SW_I2C *_u8x8;
};

#endif
