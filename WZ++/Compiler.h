#ifndef COMPILER_H
#define COMPILER_H

#include <hash_map>
#include <string>
#include <fstream>
#include <iostream>

class WZDirectory;
class IMGFile;
class WZObject;
class Empty; class Short; class Integer; class Float; class Double; class String; class Object; class Property; class Convex; class Vector; class Canvas; class UOL; class Sound;

class Compiler {
private:
	stdext::hash_map<WZDirectory*, size_t> dirsoff;
	stdext::hash_map<IMGFile*, size_t> imgsoff;

	stdext::hash_map<std::string, size_t> tstrings;

	WZDirectory* base;

	size_t baseOffset;
	
	size_t vSum;

	int version;

	std::string wzname;
	std::ofstream* file;
public:
	Compiler(std::string wzname, int version) : wzname(wzname), version(version) {}
	~Compiler(){}
	void compile();

	char encodeVersion();
	void writeDirectoryInfo(WZDirectory* base);
	void writeIMGs(WZDirectory* base);
	void loadDirectory(WZDirectory* base);
	size_t getIMGsSize();
	size_t getIMGSize(std::string name);
	void compileIMG();

	template <typename T>
	T getValue_IMG(std::string name);
	template <typename T>
	T getValue_IMG();
	std::string Compiler::getTypeFile();
	WZObject* getType_IMG();
	WZObject* getObject_IMG();

	std::string getFolderName();

	template <typename T>
	void writeValue(T value) {
		file->write((char*)(&value), sizeof(T));
	}
	void writePackedInt(int value);
	void writePackedFloat(float value);

	void writeIntAt(size_t at, int value);
	void writeLongAt(size_t at, __int64 value);

	void writeInt(int value);
	void writeShort(short value);
	void writeByte(char value);
	void writeDouble(double value);
	void writeFloat(float value);
	void writeLong(__int64 value);
	void writeOffsetAt(size_t at, size_t offset);
	void writeString(std::string str, int len);
	void writeString(std::string str);
	void writeString(std::string str, int n, int e, bool wflag = false);

	template <typename T>
	void writeType(T* obj, char type);
	template <typename T>
	void writeObjects(T* obj);
	void write(WZObject* obj);
	void write(Empty* obj);
	void write(Short* obj);
	void write(Integer* obj);
	void write(Float* obj);
	void write(Double* obj);
	void write(String* obj);
	void write(Object* obj);
	void write(Property* obj);
	void write(Convex* obj);
	void write(Vector* obj);
	void write(Canvas* obj);
	void write(UOL* obj);
	void write(Sound* obj);

	void enterDirectory(std::string name);
	void leaveDirectory();
};

#endif