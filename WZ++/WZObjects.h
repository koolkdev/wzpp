#ifndef WZOBJECTS_H
#define WZOBJECTS_H

#include "WZObject.h"
#include "File.h"
#include "../gd/gd.h"

class WZSimpleObject : public WZObject {
public:
	WZSimpleObject(std::string name) : name(name) {}
	WZSimpleObject(){}
	std::string name;	
};

class WZObjects : public WZObject {
public:
	std::vector <WZObject*> objects;
	std::vector <WZObject*>* getObjects(){ return &objects; }
};

template <typename T>
class WZTypeObject : public WZSimpleObject {
public:
	WZTypeObject(std::string name, T value) : WZSimpleObject(name), value(value) {}
	WZTypeObject(T value) : value(value) {}
	T value;
};

class Empty : public WZSimpleObject {
public:
	Empty(std::string name) : WZSimpleObject(name) { type = WZ_EMPTY; }
	Empty(){ type = WZ_EMPTY; }
};

class Short : public WZTypeObject<short> {
public:
	Short(std::string name, short value) : WZTypeObject(name, value) { type = WZ_SHORT; }
	Short(short value) : WZTypeObject(value) { type = WZ_SHORT; }
};

class Integer : public WZTypeObject<int> {
public:
	Integer(std::string name, int value) : WZTypeObject(name, value) { type = WZ_INTEGER; }
	Integer(int value) : WZTypeObject(value) { type = WZ_INTEGER; }
};

class Float : public WZTypeObject<float> {
public:
	Float(std::string name, float value) : WZTypeObject(name, value) { type = WZ_FLOAT; }
	Float(float value) : WZTypeObject(value) { type = WZ_FLOAT; }
};

class Double : public WZTypeObject<double> {
public:
	Double(std::string name, double value) : WZTypeObject(name, value) { type = WZ_DOUBLE; }
	Double(double value) : WZTypeObject(value) { type = WZ_DOUBLE; }
};

class String : public WZTypeObject<std::string> {
public:
	String(std::string name, std::string value) : WZTypeObject(name, value) { type = WZ_STRING; }
	String(std::string value) : WZTypeObject(value) { type = WZ_STRING; }
};

class Object : public WZTypeObject<WZObject*> {
public:
	Object(std::string name, WZObject* value) : WZTypeObject(name, value) { type = WZ_OBJECT; }
	~Object(){ delete value; }
};

class Property : public WZObjects {
public:
	Property() { type = WZ_PROPERTY; }
	~Property(){ for(size_t i=0; i<objects.size(); i++) delete objects[i]; }
};


class Convex : public WZObjects {
public:
	Convex() { type = WZ_CONVEX; }
	~Convex(){ for(size_t i=0; i<objects.size(); i++) delete objects[i]; }
};

class Canvas : public WZObjects {
public:
	Canvas() { type = WZ_CANVAS; }
	Canvas(size_t width, size_t height, size_t size, size_t offset, ImageFormat format) : width(width), height(height), size(size), offset(offset), format(format) { type = WZ_CANVAS; }
	~Canvas(){ for(size_t i=0; i<objects.size(); i++) delete objects[i]; }
	ImageFormat format;
	size_t width, height, size, offset;
	std::string file;
	void save(std::string filename, File* file);
	gdImagePtr Canvas::getImage(File* file);
};

class Vector : public WZObject {
public:
	Vector(int x, int y) : x(x), y(y) { type = WZ_VECTOR; }
	int x, y;
};

class UOL : public WZObject {
public:
	UOL(std::string value) : value(value) { type = WZ_UOL; }
	std::string value;
};

class Sound : public WZObject {
public:
	Sound() { type = WZ_SOUND; }
	Sound(int offset, int size) : offset(offset), size(size) { type = WZ_SOUND; }
	int offset, size;
	int bytesOffset, bytesSize;
	std::string file, bytesFile;
	void save(std::string filename, File* file);
};


#endif