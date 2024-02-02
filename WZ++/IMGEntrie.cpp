#include "IMGEntry.h"
#include "WZObjects.h"


IMGEntry::IMGEntry(IMGEntry* parent){
	type = WZ_ENTRY;
	value = NULL;
	this->parent = parent;
}

IMGEntry::IMGEntry(IMGEntry* parent, std::string name, WZObject* value){
	type = WZ_ENTRY;
	this->name = name;
	this->value = value;
	this->parent = parent;
}

IMGEntry::IMGEntry(Property* p){
	type = WZ_ENTRY;
	value = NULL;
	createEntry(this, p);
	parent = NULL;
}
IMGEntry::~IMGEntry(){
	if(value != NULL)
		delete value;

	for(stdext::hash_map<std::string, IMGEntry*>::iterator iter = childs.begin(); iter != childs.end(); iter++){
		delete iter->second;
	}
}

IMGEntry* IMGEntry::getChild(std::string name){
	int find = name.find("/");
	if(find != -1){
		std::string tof = name.substr(0, find);
		if(childs.find(tof) != childs.end()){
			return childs[tof]->getChild(name.substr(find+1));
		}
		else if(tof == ".."){
			if(parent != NULL)
				return parent->getChild(name.substr(3));
		}
	}
	else if(childs.find(name) != childs.end()){
		return childs[name];
	}
	return NULL;
}

void IMGEntry::createEntry(IMGEntry* parent, WZObject* obj){
	switch(obj->type){
		case WZ_EMPTY: createEntry(parent, (Empty*)obj); break;
		case WZ_SHORT: createEntry(parent, (Short*)obj); break;
		case WZ_INTEGER: createEntry(parent, (Integer*)obj); break;
		case WZ_FLOAT: createEntry(parent, (Float*)obj); break;
		case WZ_DOUBLE: createEntry(parent, (Double*)obj); break;
		case WZ_STRING: createEntry(parent, (String*)obj); break;
		case WZ_OBJECT: createEntry(parent, (Object*)obj); break;
		case WZ_PROPERTY: createEntry(parent, (Property*)obj); break;
		case WZ_CONVEX: createEntry(parent, (Convex*)obj); break;
		case WZ_VECTOR: createEntry(parent, (Vector*)obj); break;
		case WZ_CANVAS: createEntry(parent, (Canvas*)obj); break;
		case WZ_UOL: createEntry(parent, (UOL*)obj); break;
		case WZ_SOUND: createEntry(parent, (Sound*)obj); break;
	}
}

template <typename T>
void IMGEntry::createEntryValue(IMGEntry* parent, T* obj){
	parent->setName(obj->name);
	parent->setValue(new T(obj->value));
}

template <typename T>
void IMGEntry::createEntryChilds(IMGEntry* parent, T* obj){
	for(size_t i=0; i<obj->getObjects()->size(); i++){
		IMGEntry* e = new IMGEntry(parent);
		createEntry(e, (*obj->getObjects())[i]);
		parent->add(e);
	}
}

void IMGEntry::createEntry(IMGEntry* parent, Empty* obj){
	parent->setName(obj->name);
	parent->setValue(new Empty());
}

void IMGEntry::createEntry(IMGEntry* parent, Short* obj){
	createEntryValue(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, Integer* obj){
	createEntryValue(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, Float* obj){
	createEntryValue(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, Double* obj){
	createEntryValue(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, String* obj){
	createEntryValue(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, Object* obj){
	parent->setName(obj->name);
	createEntry(parent, obj->value);
}

void IMGEntry::createEntry(IMGEntry* parent, Property* obj){
	createEntryChilds(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, Convex* obj){
	createEntryChilds(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, Vector* obj){
	parent->setValue(new Vector(obj->x, obj->y));
}

void IMGEntry::createEntry(IMGEntry* parent, Canvas* obj){
	parent->setValue(new Canvas(obj->width, obj->height, obj->size, obj->offset, obj->format));
	createEntryChilds(parent, obj);
}

void IMGEntry::createEntry(IMGEntry* parent, UOL* obj){
	parent->setValue(new UOL(obj->value));
}

void IMGEntry::createEntry(IMGEntry* parent, Sound* obj){
	parent->setValue(new Sound(obj->offset, obj->size));
}

short IMGEntry::getShort(){
	if(value == NULL) return 0;
	switch(value->type){
		case WZ_SHORT: return (short)(((Short*)value)->value); break;
		case WZ_INTEGER: return (short)(((Integer*)value)->value); break;
		case WZ_FLOAT: return (short)(((Float*)value)->value); break;
		case WZ_DOUBLE: return (short)(((Double*)value)->value); break;
		default: return 0;
	}
}
int IMGEntry::getInt(){
	if(value == NULL) return 0;
	switch(value->type){
		case WZ_SHORT: return (int)(((Short*)value)->value); break;
		case WZ_INTEGER: return (int)(((Integer*)value)->value); break;
		case WZ_FLOAT: return (int)(((Float*)value)->value); break;
		case WZ_DOUBLE: return (int)(((Double*)value)->value); break;
		default: return 0;
	}
}
double IMGEntry::getDouble(){
	if(value == NULL) return 0;
	switch(value->type){
		case WZ_SHORT: return (double)(((Short*)value)->value); break;
		case WZ_INTEGER: return (double)(((Integer*)value)->value); break;
		case WZ_FLOAT: return (double)(((Float*)value)->value); break;
		case WZ_DOUBLE: return (double)(((Double*)value)->value); break;
		default: return 0;
	}
}
float IMGEntry::getFloat(){
	if(value == NULL) return 0;
	switch(value->type){
		case WZ_SHORT: return (float)(((Short*)value)->value); break;
		case WZ_INTEGER: return (float)(((Integer*)value)->value); break;
		case WZ_FLOAT: return (float)(((Float*)value)->value); break;
		case WZ_DOUBLE: return (float)(((Double*)value)->value); break;
		default: return 0;
	}
}
std::string IMGEntry::getString(){
	if(value == NULL) return "";
	if(value->type == WZ_STRING) return ((String*)value)->value;
	return "";
}

Vector* IMGEntry::getVector(){
	if(value == NULL) return NULL;
	if(value->type == WZ_VECTOR) return (Vector*)value;
	return NULL;
}

Canvas* IMGEntry::getCanvas(){
	if(value == NULL) return NULL;
	if(value->type == WZ_CANVAS) return (Canvas*)value;
	return NULL;
}

short IMGEntry::getShort(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return 0;
	return e->getShort();
}
int IMGEntry::getInt(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return 0;
	return e->getInt();
}
double IMGEntry::getDouble(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return 0;
	return e->getDouble();
}
float IMGEntry::getFloat(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return 0;
	return e->getFloat();
}
std::string IMGEntry::getString(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return "";
	return e->getString();
}
Vector* IMGEntry::getVector(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return NULL;
	return e->getVector();
}

Canvas* IMGEntry::getCanvas(std::string name){
	IMGEntry* e = getChild(name);
	if(e == NULL) return NULL;
	return e->getCanvas();
}