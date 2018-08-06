#include <stdio.h>
#include "Disp.h"

Disp::Disp() {}

char *Disp::clearAndGetBufferLine(int line) {
	return _buffer[line];
}

void Disp::printBufferLine(int line) {
	printf ("%s", _buffer[line]);
}