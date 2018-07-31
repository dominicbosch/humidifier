
#ifndef Disp_h
#define Disp_h

#define BUFFER_SIZE 16

class Disp {
	public:
		Disp();
		char *getBufferLine(int line);
		void printBufferLine(int line);

	private:
		char _buffer[4][BUFFER_SIZE];
};

#endif