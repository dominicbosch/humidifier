#include <stdio.h>

#define BUFFER_SIZE 16

int main (void) {
	printf ("Hello, world!\n");
	char buffer[4][BUFFER_SIZE] = {{""}};
	snprintf(buffer[0], BUFFER_SIZE, "Testing!");
	printf ("%s", buffer[0]);
	return 0;
}