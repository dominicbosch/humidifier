#include <stdio.h>
#include <stdlib.h>
#include "Disp.h"

#define BUFFER_LENGTH 16

int main (void) {
	printf ("Hello, world 2!\n");
	Disp *d = new Disp();

	char *buff = d->clearAndGetBufferLine(0);
	snprintf(buff, BUFFER_LENGTH, "Testing Two!");
	d->printBufferLine(0);

	free(d);
	return 0;
}