#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <string>

#define PATH_SPLIT "/"
#define NEW_LINE "\r\n<BR />"

class File;
class IMGFile;
class WZObject;
class Empty; class Short; class Integer; class Float; class Double; class String; class Object; class Property; class Convex; class Vector; class Canvas; class UOL; class Sound;

class Extractor {
private:
	File* file;
public:
	Extractor(){};
	Extractor(File* file) : file(file) {}

	void extractFolder(size_t baseOffset);
	void extractIMG(size_t baseOffset);
	void saveIMG(std::string name, size_t baseOffset, size_t size);

	void saveIMG(IMGFile* img);
	template <typename T>
	void writeValue(std::ofstream& file, std::string& baseStr, T* obj);
	template <typename T>
	void writeObjects(std::ofstream& file, std::string& baseStr, T* obj);
	void write(std::ofstream& file, std::string& baseStr, WZObject* obj);
	void write(std::ofstream& file, std::string& baseStr, Empty* obj);
	void write(std::ofstream& file, std::string& baseStr, Short* obj);
	void write(std::ofstream& file, std::string& baseStr, Integer* obj);
	void write(std::ofstream& file, std::string& baseStr, Float* obj);
	void write(std::ofstream& file, std::string& baseStr, Double* obj);
	void write(std::ofstream& file, std::string& baseStr, String* obj);
	void write(std::ofstream& file, std::string& baseStr, Object* obj);
	void write(std::ofstream& file, std::string& baseStr, Property* obj);
	void write(std::ofstream& file, std::string& baseStr, Convex* obj);
	void write(std::ofstream& file, std::string& baseStr, Vector* obj);
	void write(std::ofstream& file, std::string& baseStr, Canvas* obj);
	void write(std::ofstream& file, std::string& baseStr, UOL* obj);
	void write(std::ofstream& file, std::string& baseStr, Sound* obj);

	template <typename T>
	void writeValue(T* obj);
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
	void writeType(WZObject* obj);

	void createDirectory(std::string& name);
	void enterDirectory(std::string& name);
	void leaveDirectory();
};

#endif