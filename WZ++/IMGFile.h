#ifndef IMGFILE_H
#define IMGFILE_H

#include <string>

class File;
class WZObject;
class Property;

class IMGFile {
private:
	size_t baseOffset;
	File* file;
	Property* obj;
	std::string name;
public:
	IMGFile(File* file, size_t baseOffset = 0) : file(file), baseOffset(baseOffset), obj(NULL) { }
	IMGFile(std::string name) : name(name) {}
	~IMGFile();

	std::string& getName(){
		return name;
	}

	std::string readString();
	WZObject* getType();
	WZObject* getObject();
	Property* getProperty(){
		return obj;
	}
	void load();
};

#endif