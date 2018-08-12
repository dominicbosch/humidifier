/*
  Display.h - Interface to digital input switches.
*/
#ifndef Display_h
#define Display_h

#define BUFFER_LENGTH 17
#define BUFFER_HEIGHT 8

#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

#include "Arduino.h"
#include <U8x8lib.h>

class Display {
  public:
    Display(int clockPin = 18, int dataPin = 19);
    char *clearAndGetBufferLine(int line);
    void printBufferLineAsString(int line);
    void printBufferLineAsUTF8(int line);
    void printString(int line, const char *buffer);
    void clear();
    void clearBufferLine(int line);

  private:
    U8X8_SSD1306_128X64_NONAME_SW_I2C *_u8x8;
    char _displayBuffer[BUFFER_HEIGHT][BUFFER_LENGTH] = {{""}};
    void _fillBlanksAtLine(int line);
};

#endif
