#include "WZFile.h"
#include "Extractor.h"
#include "Animation.h"

#include <algorithm>
#include <sstream>

WZFile::WZFile(std::string fileName) {

	std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);

	this->fileName = fileName;
	
	int fileNameStart = fileName.find("\\");
	fileNameStart = (fileNameStart == -1) ? 0 : fileNameStart;
	int fileNameEnd = fileName.find(".wz");

	if(fileNameEnd == -1 || fileNameStart > fileNameEnd) throw fileName + "not WZ file";

	wzName = fileName.substr(fileNameStart, fileNameEnd);

	std::transform(wzName.begin(), ++wzName.begin(), wzName.begin(), toupper);
}

void WZFile::open() {
	file.open(fileName.c_str(), std::ios::in|std::ios::binary);

	if(file == NULL) throw "Can't open " + fileName;

	parseHeader();

	//dir = new WZDirectory(this, wzName);
}

void WZFile::extract(){
	//Animation ex(this, "MapEff.img");
	Extractor ex(this);
	//ex.createDirectory(std::string("data"));
	//ex.enterDirectory(std::string("data"));
	//ex.createDirectory(wzName + ".wz");
	//ex.enterDirectory(wzName + ".wz");
	ex.createDirectory(wzName);
	ex.enterDirectory(wzName);
	ex.extractFolder(2); // the version - 2 bytes
	ex.leaveDirectory();
	ex.leaveDirectory();
}

void WZFile::parseHeader(){
	file.seekg(std::ios_base::beg);

	header = readString(4);
	fileSize = readLong();
	fileStart = readInt();
	copyright = readString(fileStart - filePos());
	version = readByte();
	readByte();

	findVersionSum();
}

void WZFile::findVersionSum(){
	// TODO: fix
	
	char ver[5] = {0};

	unsigned char s1, s2, s3, s4;

	for(unsigned char i=0; i<UCHAR_MAX; i++){
		size_t sum = 0;
	
		char* cv = (char *)ver;

		_itoa_s(i, cv, 5, 10);

		while(*cv){
			sum <<= 5;
			sum += (unsigned char)(*cv++ +1);
		}
		
		s1 = (sum >> 24) & 0xFF;
		s2 = (sum >> 16) & 0xFF;
		s3 = (sum >> 8) & 0xFF;
		s4 = (sum >> 0) & 0xFF;

		if((unsigned char)(~(s1^s2^s3^s4)) == version){
			versionSum = sum;
			return;
		}
	}
	versionSum = 0;
}

size_t WZFile::readOffset(){
	size_t offset =  (~(filePos() - fileStart))*versionSum - 0x581C3F6D;
	offset = _lrotl(offset, offset & 0x1F) ^ readInt();
	return offset + fileStart;
}
