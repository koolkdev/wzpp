#ifndef IMGENTRY_H
#define IMGENTRY_H

#include <string>
#include <hash_map>

#include "WZObject.h"

class Property;
class Empty; class Short; class Integer; class Float; class Double; class String; class Object; class Property; class Convex; class Vector; class Canvas; class UOL; class Sound;

class IMGEntry : public WZObject {
private:
	stdext::hash_map<std::string, IMGEntry*> childs;
	WZObject* value;
	IMGEntry* parent;
	std::string name;
public:
	IMGEntry(IMGEntry* parent);
	IMGEntry(IMGEntry* parent, std::string name, WZObject* value);
	IMGEntry(Property* p);
	~IMGEntry();
	IMGEntry* getParent(){
		return parent;
	}
	void add(IMGEntry* child){
		childs[child->name] = child;
	}
	WZObject* getValue(){
		return value;
	}
	void setValue(WZObject* value){
		this->value = value;
	}
	IMGEntry* getChild(std::string name);
	std::string getName(){
		return name;
	}
	void setName(std::string name){
		this->name = name;
	}
	stdext::hash_map<std::string, IMGEntry*>* getEntries(){
		return &childs;
	}

	template <typename T>
	static void createEntryValue(IMGEntry* parent, T* obj);
	template <typename T>
	static void createEntryChilds(IMGEntry* parent, T* obj);
	static void createEntry(IMGEntry* parent, WZObject* obj);
	static void createEntry(IMGEntry* parent, Empty* obj);
	static void createEntry(IMGEntry* parent, Short* obj);
	static void createEntry(IMGEntry* parent, Integer* obj);
	static void createEntry(IMGEntry* parent, Float* obj);
	static void createEntry(IMGEntry* parent, Double* obj);
	static void createEntry(IMGEntry* parent, String* obj);
	static void createEntry(IMGEntry* parent, Object* obj);
	static void createEntry(IMGEntry* parent, Property* obj);
	static void createEntry(IMGEntry* parent, Convex* obj);
	static void createEntry(IMGEntry* parent, Vector* obj);
	static void createEntry(IMGEntry* parent, Canvas* obj);
	static void createEntry(IMGEntry* parent, UOL* obj);
	static void createEntry(IMGEntry* parent, Sound* obj);

	short getShort();
	int getInt();
	double getDouble();
	float getFloat();
	std::string getString();
	Vector* getVector();
	Canvas* getCanvas();
	
	short getShort(std::string name);
	int getInt(std::string name);
	double getDouble(std::string name);
	float getFloat(std::string name);
	std::string getString(std::string name);
	Vector* getVector(std::string name);
	Canvas* getCanvas(std::string name);



};

#endif