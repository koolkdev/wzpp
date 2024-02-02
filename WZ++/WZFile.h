#ifndef WZFILE_H
#define WZFILE_H


#include "File.h"

class WZFile : public File {
private:
	std::string wzName;

	// Header:

	std::string header;
	std::string copyright;
	unsigned __int64 fileSize;
	unsigned char version;
	size_t versionSum;
public:
	WZFile(std::string fileName);
	void open();

	void extract();

	void parseHeader();

	size_t readOffset();

	void findVersionSum();
};

#endif