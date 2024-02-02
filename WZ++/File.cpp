#include "File.h"
#include "AES.h"

#include <algorithm>
#include <sstream>

AES* File::aes;

const unsigned char AESkey[32] = {0x13,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0xB4,0x00,0x00,0x00,0x1B,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x33,0x00,0x00,0x00,0x52,0x00,0x00,0x00};

const unsigned char IV[4] = {0x4D, 0x23, 0xc7, 0x2b};

unsigned char keys[_UI16_MAX] = {0};

//#define AES_PROTECTION //GMS KMS


void File::initialize(){
	aes = new AES();
	
	aes->SetParameters(256);
	aes->StartEncryption(AESkey);
#ifdef AES_PROTECTION
	aes->DecryptOFB(keys, (unsigned char*)IV, _UI16_MAX);
#endif
}


File::File(std::string fileName){
	fileStart = 0;

	std::transform(fileName.begin(), fileName.end(), fileName.begin(), tolower);

	this->fileName = fileName;

}

void File::open() {
	file.open(fileName.c_str(), std::ios::in|std::ios::binary);

	if(file == NULL) throw "Can't open " + fileName;
}

void File::close(){
	file.close();
}

std::string File::decodeString(size_t size) {
	unsigned char *bytes = new unsigned char[size+1];

	file.read((char*)bytes, size);

	unsigned char key = 0xAA;

	for(size_t i=0; i<size; i++)
		*bytes++ ^= keys[i] ^ key++;
	
	bytes -= size;

	bytes[size] = '\0';

	std::string str = std::string((const char*)bytes);
	
	delete [] bytes;

	return str;
}

std::string File::decodeUnicodeString(size_t size) {
	wchar_t *bytes = new wchar_t[size+1];

	file.read((char*)bytes, size*2);

	std::string str;

	unsigned short key = 0xAAAA;

	for(size_t i=0; i<size; i++) {
		bytes[i] ^= *((unsigned short*)(keys+(i*2))) ^ key++;
		std::stringstream s;
		s << (int)bytes[i];
		str += "&#" + s.str() + ";";
	}

	bytes[size] = '\0';

	//std::wstring wstr = std::wstring(bytes);
	
	delete [] bytes;

	return str;
}

int File::readValue(){
	char byte = readByte();
	if(byte == _I8_MIN) return readInt();
	return byte;
}

float File::readPacketFloat(){
	char byte = readByte();
	if(byte == _I8_MIN) return readFloat();
	else if(byte == 0) return 0;
	else throw "Invaild packed float type.";
}

std::string File::readString() {
	char size = readByte();

	int fsize = 0;

	if(size > 0) {
		if(size == _I8_MAX) fsize = readInt();
		else fsize = size;
		return decodeUnicodeString(fsize);
	}
	else if(size < 0) {
		if(size == _I8_MIN) fsize = readInt();
		else fsize = -size;
		return decodeString(fsize);
	}

	return "";
}


std::string File::readString(size_t size) {
	unsigned char *bytes = new unsigned char[size+1];

	file.read((char*)bytes, size);

	bytes[size] = '\0';

	std::string str = std::string((char*)bytes);

	delete [] bytes;

	return str;
}

std::string File::readStringAt(bool flag){
	return readStringAt(fileStart, flag);
}

std::string File::readStringAt(size_t baseOffset, bool flag){
	size_t offset = readInt();
	size_t curr = filePos();
	
	file.seekg(baseOffset + offset);

	if(flag) readByte();

	std::string str = readString();

	file.seekg(curr);

	return str;
}

int File::readInt() {
	return readValue<int>();
}
short File::readShort() {
	return readValue<short>();
}
char File::readByte() {
	return readValue<char>();
}
double File::readDouble() {
	return readValue<double>();
}
float File::readFloat() {
	return readValue<float>();
}
__int64 File::readLong() {
	return readValue<__int64>();
}