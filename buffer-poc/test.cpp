#include <stdio.h>

#define BUFFER_LENGTH 16

int main (void) {
	printf ("Hello, world!\n");
	char buffer[4][BUFFER_LENGTH] = {{""}};
	snprintf(buffer[0], BUFFER_LENGTH, "Testing!");
	printf ("%s", buffer[0]);
	return 0;
}