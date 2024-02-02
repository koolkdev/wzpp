#ifndef GIFENCODER_H
#define GIFENCODER_H

struct GIFFrame {
	unsigned char* data;
	size_t length;
	int delay;
};

class GIFEncoder {
private:
	FILE* f;
	unsigned char** bytes;
	size_t* length;
public:
	GIFEncoder
};
#endif