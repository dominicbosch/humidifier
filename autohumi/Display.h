/*
  Display.h - Interface to digital input switches.
*/
#ifndef Display_h
#define Display_h

#define BUFFER_SIZE 16
#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

#include "Arduino.h"
#include <U8x8lib.h>

class Display {
  public:
    Display(int clockPin = 19, int dataPin = 18);
    char *getBufferLine(int line);
    void printBufferLineAsString(int line);
    void printBufferLineAsUTF8(int line);
    void printString(int line, const char *buffer);
    void clear();

  private:
    U8X8_SSD1306_128X64_NONAME_SW_I2C *_u8x8;
    char _displayBuffer[4][BUFFER_SIZE] = {{""}};
};

#endif
