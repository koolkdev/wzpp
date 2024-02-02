#include "IMGFile.h"
#include "WZFile.h"
#include "WZObjects.h"

IMGFile::~IMGFile(){
	if(obj != NULL)
		delete obj;
}

void IMGFile::load(){
	file->fileSeek(baseOffset);

	obj = (Property*)getObject();
}

std::string IMGFile::readString(){
	switch(file->readByte()){
		case 0x00:
		case 0x73: return file->readString();
		case 0x01:
		case 0x1B: return file->readStringAt(baseOffset);
		default: throw "Invalid string type.";
	}
}

WZObject* IMGFile::getType(){
	std::string name = readString();
	switch(file->readByte()){
		case 0x00: return new Empty(name); break;
		case 0x02: return new Short(name, file->readShort());
		case 0x03: return new Integer(name, file->readValue());
		case 0x04: return new Float(name, file->readPacketFloat());
		case 0x05: return new Double(name, file->readDouble());
		case 0x08: return new String(name, readString());
		case 0x09: file->readInt(); return new Object(name, getObject()); // size
		default: throw "Invaild simple object type.";		
	}
}

WZObject* IMGFile::getObject(){
	std::string objectType = readString();
	if(objectType == "Property"){
		Property* pr = new Property();
		file->readShort(); // NULL

		size_t objects = file->readValue();

		for(size_t i=0; i<objects; i++) {
			pr->getObjects()->push_back(getType());
		}

		return pr;
	}
	else if(objectType == "Shape2D#Convex2D"){
		Convex* con = new Convex();

		size_t objects = file->readValue();

		for(size_t i=0; i<objects; i++) {
			con->getObjects()->push_back(getObject());
		}

		return con;
	}
	else if(objectType == "Shape2D#Vector2D"){
		int x = file->readValue();
		int y = file->readValue();

		return new Vector(x, y);
	}
	else if(objectType == "Canvas"){
		Canvas* can = new Canvas();
		file->readByte(); // NULL

		if(file->readByte()){
			file->readShort(); // NULL
			size_t objects = file->readValue();

			for(size_t i=0; i<objects; i++) {
				can->getObjects()->push_back(getType());
			}
		}

		can->width = file->readValue();
		can->height = file->readValue();
		int format = file->readValue(); // format1
		can->format = (ImageFormat)(file->readValue() + format); // fomrat2
		file->readInt(); // NULL
		can->size = file->readInt();
		can->offset = file->filePos();
		file->fileSeekNext(can->size);

		return can;
	}
	else if(objectType == "Sound_DX8"){
		Sound* sound = new Sound();
		file->readByte(); // NULL
		sound->size = file->readValue();
		file->readValue(); // checksum
		file->fileSeekNext(51);
		sound->bytesSize = file->readByte();
		sound->bytesOffset = file->filePos();
		sound->offset = file->filePos() + sound->bytesSize;
		file->fileSeekNext(sound->size + sound->bytesSize);
		return sound;
	}
	else if(objectType == "UOL"){
		file->readByte(); // NULL
		return new UOL(readString());
	}

	throw "Invalid Object type: " + objectType;
}