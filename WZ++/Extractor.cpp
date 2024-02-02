#include "Extractor.h"
#include "WZFile.h"
#include "IMGFile.h"
#include "WZObjects.h"
#include <assert.h>
#include <windows.h>
#include <algorithm>
#include <hash_map>

//#define REPACK_EXTRACTION

void Extractor::extractFolder(size_t baseOffset) {
	size_t cur = file->filePos();
	file->fileSeek(baseOffset + file->getFileStart());

	size_t dirs = file->readValue();

	for(size_t i = 0; i<dirs; i++) {

		std::string name;
		size_t size, checksum, offset;
		unsigned char type = file->readByte();

		switch(type){
			case 0x02: name = file->readStringAt(true); break;
			case 0x03:
			case 0x04: name = file->readString(); break;
			default: throw "Can't parse directories, Invalid directory type.";
		}

		size = file->readValue();
		checksum = file->readValue();
		offset = ((WZFile*)file)->readOffset();
		
		createDirectory(name);
		enterDirectory(name);

		size_t base = file->filePos();

		std::cout << "extract " << name << " " << size << std::endl;

		switch(type){
			case 0x02:
			case 0x04: extractIMG(offset); break;
			case 0x03: extractFolder(offset); break;
		}

		leaveDirectory();
	}
	

	file->fileSeek(cur);
}

void Extractor::saveIMG(std::string name, size_t baseOffset, size_t size) {
	size_t cur = file->filePos();

	
	std::ofstream f(name.c_str(), std::ios_base::out | std::ios_base::binary);

	char* bytes = new char[size];

	file->readBytes(bytes, size);
	f.write(bytes, size);

	f.close();

	delete [] bytes;

	file->fileSeek(cur);
}

void Extractor::extractIMG(size_t baseOffset) {
	size_t cur = file->filePos();

	IMGFile img(file, baseOffset + file->getFileStart());

	img.load();
	
#ifdef REPACK_EXTRACTION
	write(img.getProperty());
#else
	saveIMG(&img);
#endif

	file->fileSeek(cur);
}

void Extractor::saveIMG(IMGFile* img){
	std::ofstream file;
	file.open("index.html");
	
	write(file, std::string(""), img->getProperty());

	file.close();
}

void Extractor::write(std::ofstream& file, std::string& baseStr, WZObject* obj){
	switch(obj->type){
		case WZ_EMPTY: write(file, baseStr, (Empty*)obj); break;
		case WZ_SHORT: write(file, baseStr, (Short*)obj); break;
		case WZ_INTEGER: write(file, baseStr, (Integer*)obj); break;
		case WZ_FLOAT: write(file, baseStr, (Float*)obj); break;
		case WZ_DOUBLE: write(file, baseStr, (Double*)obj); break;
		case WZ_STRING: write(file, baseStr, (String*)obj); break;
		case WZ_OBJECT: write(file, baseStr, (Object*)obj); break;
		case WZ_PROPERTY: write(file, baseStr, (Property*)obj); break;
		case WZ_CONVEX: write(file, baseStr, (Convex*)obj); break;
		case WZ_VECTOR: write(file, baseStr, (Vector*)obj); break;
		case WZ_CANVAS: write(file, baseStr, (Canvas*)obj); break;
		case WZ_UOL: write(file, baseStr, (UOL*)obj); break;
		case WZ_SOUND: write(file, baseStr, (Sound*)obj); break;
	}
}

template <typename T>
void Extractor::writeValue(std::ofstream& file, std::string& baseStr, T* obj){
	file << baseStr << obj->name << " " << obj->value << NEW_LINE;
}

template <typename T>
void Extractor::writeObjects(std::ofstream& file, std::string& baseStr, T* obj){
	for(size_t i=0; i<obj->getObjects()->size(); i++){
		write(file, baseStr, (*obj->getObjects())[i]);
	}
	if(obj->getObjects()->size() == 0)
		file << baseStr << " NULL" << NEW_LINE;
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Empty* obj){
	file << baseStr << obj->name << " NULL" << NEW_LINE;
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Short* obj){
	writeValue(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Integer* obj){
	writeValue(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Float* obj){
	writeValue(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Double* obj){
	writeValue(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, String* obj){
	writeValue(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Object* obj){
	write(file, baseStr + obj->name + PATH_SPLIT, obj->value);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Property* obj){
	writeObjects(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Convex* obj){
	writeObjects(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Vector* obj){
	file << baseStr << "x " << obj->x << NEW_LINE;
	file << baseStr << "y " << obj->y << NEW_LINE;

}

void Extractor::write(std::ofstream& file, std::string& baseStr, Canvas* obj){
	std::string name = baseStr;
	std::replace(name.begin(), name.end(), '/', '.' );
	obj->save(name + "png", this->file);
	file << baseStr << "image <img src='" << name << "png' />" << NEW_LINE;
	writeObjects(file, baseStr, obj);
}

void Extractor::write(std::ofstream& file, std::string& baseStr, UOL* obj){
	file << baseStr << "image* " << obj->value << NEW_LINE;
}

void Extractor::write(std::ofstream& file, std::string& baseStr, Sound* obj){
	std::string name = baseStr;
	std::replace(name.begin(), name.end(), '/', '.' );
	obj->save(name + "mp3", this->file);
	file << baseStr << "sound <a href='" << name << "mp3'>" << name << "mp3</a>" << NEW_LINE;
}


void Extractor::write(WZObject* obj){
	switch(obj->type){
		case WZ_EMPTY: write((Empty*)obj); break;
		case WZ_SHORT: write((Short*)obj); break;
		case WZ_INTEGER: write((Integer*)obj); break;
		case WZ_FLOAT: write((Float*)obj); break;
		case WZ_DOUBLE: write((Double*)obj); break;
		case WZ_STRING: write((String*)obj); break;
		case WZ_OBJECT: write((Object*)obj); break;
		case WZ_PROPERTY: write((Property*)obj); break;
		case WZ_CONVEX: write((Convex*)obj); break;
		case WZ_VECTOR: write((Vector*)obj); break;
		case WZ_CANVAS: write((Canvas*)obj); break;
		case WZ_UOL: write((UOL*)obj); break;
		case WZ_SOUND: write((Sound*)obj); break;
	}
}

template <typename T>
void Extractor::writeValue(T* obj){
	createDirectory(obj->name);
	enterDirectory(obj->name);

	writeType(obj);
	std::ofstream f;
	f.open("value.txt", std::ios::in | std::ios::trunc);

	f << obj->value;

	f.close();

	leaveDirectory();
}

template <typename T>
void Extractor::writeObjects(T* obj){
	writeType(obj);
	for(size_t i=0; i<obj->getObjects()->size(); i++){
		write((*obj->getObjects())[i]);
	}
}

void Extractor::write(Empty* obj){
	createDirectory(obj->name);
	enterDirectory(obj->name);

	writeType(obj);

	leaveDirectory();
}

void Extractor::write(Short* obj){
	writeValue(obj);
}

void Extractor::write(Integer* obj){
	writeValue(obj);
}

void Extractor::write(Float* obj){
	writeValue(obj);
}

void Extractor::write(Double* obj){
	writeValue(obj);
}

void Extractor::write(String* obj){
	writeValue(obj);
}

void Extractor::write(Object* obj){
	createDirectory(obj->name);
	enterDirectory(obj->name);

	write(obj->value);

	leaveDirectory();
}

void Extractor::write(Property* obj){
	writeObjects(obj);
}

void Extractor::write(Convex* obj){
	writeObjects(obj);
}

void Extractor::write(Vector* obj){
	writeType(obj);
	std::ofstream f;
	f.open("x.txt", std::ios::in | std::ios::trunc);
	f << obj->x;
	f.close();
	f.open("y.txt", std::ios::in | std::ios::trunc);
	f << obj->y;
	f.close();
}

void Extractor::write(Canvas* obj){
	obj->save("image.png", this->file);
	writeObjects(obj);
	std::ofstream f;
	f.open("format.txt", std::ios::in | std::ios::trunc);
	f << obj->format;
	f.close();
}

void Extractor::write(UOL* obj){
	writeType(obj);
	std::ofstream f;
	f.open("value.txt", std::ios::in | std::ios::trunc);
	f << obj->value;
	f.close();
}

void Extractor::write(Sound* obj){
	writeType(obj);
	obj->save("sound.mp3", this->file);
	// save wierd bytes:
	std::ofstream f;

	file->fileSeek(obj->bytesOffset);

	char *bytes = new char[obj->bytesSize];
	file->readBytes(bytes, obj->bytesSize);

	f.open("bytes.dat", std::ios::out | std::ios::binary);
	f.write(bytes, obj->bytesSize);
	f.close();
}

void Extractor::writeType(WZObject* obj){
	std::string type;
	switch(obj->type){
		case WZ_EMPTY: type = "WZ_EMPTY"; break;
		case WZ_SHORT: type = "WZ_SHORT"; break;
		case WZ_INTEGER: type = "WZ_INTEGER";; break;
		case WZ_FLOAT: type = "WZ_FLOAT"; break;
		case WZ_DOUBLE: type = "WZ_DOUBLE"; break;
		case WZ_STRING: type = "WZ_STRING"; break;
		case WZ_OBJECT: type = "WZ_OBJECT"; break; // should not be
		case WZ_PROPERTY: type = "WZ_PROPERTY"; break;
		case WZ_CONVEX: type = "WZ_CONVEX"; break;
		case WZ_VECTOR: type = "WZ_VECTOR"; break;
		case WZ_CANVAS: type = "WZ_CANVAS"; break;
		case WZ_UOL: type = "WZ_UOL"; break;
		case WZ_SOUND: type = "WZ_SOUND"; break;
	}

	std::ofstream f;
	f.open("type.txt", std::ios::in | std::ios::trunc);

	f << type;

	f.close();
}

void Extractor::createDirectory(std::string& name) {
	std::wstring fileName(name.length(),L' ');
	std::copy(name.begin(), name.end(), fileName.begin());
	::CreateDirectory(fileName.c_str(), NULL);
}

void Extractor::enterDirectory(std::string& name) {
	std::wstring fileName(name.length(),L' ');
	std::copy(name.begin(), name.end(), fileName.begin());
	::SetCurrentDirectory(fileName.c_str());
}
void Extractor::leaveDirectory() {
	::SetCurrentDirectory(L"..");
}