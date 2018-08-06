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
char *Display::clearAndGetBufferLine(int line) {
	if (line >= BUFFER_HEIGHT) return NULL;
	else {
		memset(_displayBuffer[line], ' ', BUFFER_LENGTH);
		return _displayBuffer[line];
	}
}

void Display::clear() {
	_u8x8->clear();
};
void Display::clearBufferLine(int line) {
	char *buffer = clearAndGetBufferLine(line);
	snprintf(buffer, BUFFER_LENGTH, "");
	_fillBlanksAtLine(line);
	_u8x8->drawString(0, line, buffer);
};
// void Display::clearLine(int num) {
//   memset(oledLines[num], "", sizeof(oledLines[0][0]*16));
// };

void Display::printString(int line, const char *n_buffer) {
	char *buffer = clearAndGetBufferLine(line);
	snprintf(buffer, BUFFER_LENGTH, n_buffer);
	_fillBlanksAtLine(line);
	_u8x8->drawString(0, line, buffer);
};
void Display::printBufferLineAsString(int line) {
	_fillBlanksAtLine(line);
	_u8x8->drawString(0, line, _displayBuffer[line]);
};
void Display::printBufferLineAsUTF8(int line) {
	_fillBlanksAtLine(line);
	_u8x8->drawUTF8(0, line, _displayBuffer[line]);
};

void Display::_fillBlanksAtLine(int line) {
	int i = 0;
	bool atEnd = false;
	while (i < BUFFER_LENGTH) {
		// Serial.println(_displayBuffer[line][i]);
		// Serial.println(_displayBuffer[line][i] == '\0');
		if (_displayBuffer[line][i] == '\0') atEnd = true;
		if (atEnd) _displayBuffer[line][i] = ' ';
		i++;
	}
	_displayBuffer[line][BUFFER_LENGTH-1] = '\0';
}