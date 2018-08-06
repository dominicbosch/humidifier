
#ifndef Disp_h
#define Disp_h

#define BUFFER_LENGTH 16

class Disp {
	public:
		Disp();
		char *clearAndGetBufferLine(int line);
		void printBufferLine(int line);

	private:
		char _buffer[4][BUFFER_LENGTH];
};

#endif