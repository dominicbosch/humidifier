/*
  Display.cpp - OLED display
*/
#include "Arduino.h"
#include "Display.h"
#include <U8x8lib.h>

Display::Display(int clockPin, int dataPin) {
  _u8x8 = new U8X8_SSD1306_128X64_NONAME_SW_I2C(clockPin, dataPin);
  _u8x8->begin();
  _u8x8->setFlipMode(1);
  _u8x8->setFont(u8x8_font_pxplusibmcgathin_f);
  _u8x8->draw2x2String(0, 2, "Starting");
  _u8x8->draw2x2String(0, 4, "   up!");
};
void Display::clear() {
  _u8x8->clear();
  // memset(oledLines, "", sizeof(oledLines[0][0]*4*16));
};
// void Display::clearLine(int num) {
//   memset(oledLines[num], "", sizeof(oledLines[0][0]*16));
// };
// void Display::writeLine(int num) {
//   _u8x8->drawUTF8(0, num, oledLines[num]);
// };

// FIXME use char array: const char *s
// void Display::writeString(int num, String txt) {
// 	char *buffer = &txt[0];
// 	_u8x8->drawString(0, num, buffer);
// };
void Display::writeString(int num, const char *buffer) {
	_u8x8->drawString(0, num, buffer);
};
// FIXME use char array: const char *s
// void Display::writeUTF8(int num, String txt) {
// 	char *buffer = &txt[0];
// 	_u8x8->drawUTF8(0, num, buffer);
// };
void Display::writeUTF8(int num, const char *buffer) {
	_u8x8->drawUTF8(0, num, buffer);
};



// printVariable(int row, String form, int val) {
//  char buffer[16] = "";
//  snprintf(buffer, form, val);
//  _displ.writeString(row, buffer);
// }
