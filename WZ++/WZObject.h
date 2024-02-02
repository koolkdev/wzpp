#ifndef WZOBJECT_H
#define WZOBJECT_H

#include <string>
#include <vector>

class WZFile;

class Empty;

enum WZObjectType {
	WZ_EMPTY = 0x1, // 0x00
	WZ_SHORT = 0x2, // 0x02
	WZ_INTEGER = 0x4, // 0x03
	WZ_FLOAT = 0x8, // 0x04
	WZ_DOUBLE = 0x10, // 0x05
	WZ_STRING = 0x20, // 0x08
	WZ_OBJECT = 0x40, // 0x09
	WZ_PROPERTY = 0x80, // Property
	WZ_CONVEX = 0x100, // Shape2D#Convex2D
	WZ_VECTOR = 0x200, // Shape2D#Vector2D
	WZ_CANVAS = 0x400, // Canvas
	WZ_UOL = 0x800, // UOL
	WZ_SOUND = 0x1000, // Sound_DX8
	WZ_ENTRY = 0x2000 // for Entrys
};

#define WZ_SIMPLE_OBJECT (WZ_EMPTY | WZ_SHORT | WZ_INTEGER | WZ_FLOAT | WZ_DOUBLE | WZ_STRING)
#define WZ_OBJECTS (WZ_PROPERTY | WZ_CONVEX | WZ_CANVAS)
#define WZ_NUMBERIC_VALUE (WZ_SHORT | WZ_INTEGER | WZ_FLOAT | WZ_DOUBLE)

enum ImageFormat {
	FORMAT_4444	= 1,
	FORMAT_8888	= 2,
	FORMAT_565	= 513,
	FORMAT_BIN	= 517
};

class WZObject {
public:
	WZObjectType type;
	
	virtual ~WZObject(){}
};

#endif