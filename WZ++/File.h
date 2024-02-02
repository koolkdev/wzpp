#ifndef FILE_H
#define FILE_H

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

class AES;

class File {
protected:
	std::ifstream file;

	static AES* aes;

	size_t fileStart;

	std::string fileName;
public:
	File(){}
	File(std::string fileName);

	void open();
	void close();

	static void initialize();
	std::string decodeString(size_t size);
	std::string decodeUnicodeString(size_t size);

	int readValue(); // Packet integer
	float readPacketFloat();

	void readBytes(char* str, size_t num){
		file.read(str, num);
	}

	template <typename T>
	T readValue() {
		T value;
		file.read((char*)(&value), sizeof(T));
		return value;
	}

/*	int readSignedInt() {
		return readValue<int>();
	}
	char readSignedByte() {
		return readValue<char>();
	}*/
	int readInt();
	short readShort();
	char readByte();
	double readDouble();
	float readFloat();
	__int64 readLong();
	std::string readString();
	std::string readString(size_t size);
	std::string readStringAt(bool flag = false);
	std::string readStringAt(size_t baseOffset, bool flag = false);

	size_t filePos(){
		return file.tellg();
	}
	void fileSeek(size_t offset){
		file.seekg(offset);
	}
	void fileSeekNext(size_t offset){
		file.seekg(offset, std::ios_base::cur);
	}

	size_t getFileStart(){
		return fileStart;
	}
};

#endif