#include "Compiler.h"
#include "WZDirectory.h"
#include "WZObjects.h"
#include "IMGFile.h"
#include <windows.h>
#include <direct.h>
#include <assert.h>
#include "../zlib/zlib.h"

#define CHUNK 16384

extern unsigned char keys[_UI16_MAX];

void Compiler::compile(){
	std::ofstream tfile(wzname.c_str(), std::ios::out|std::ios::binary);
	file = &tfile;
	enterDirectory("data");
	enterDirectory(wzname);

	// write header
	writeString("PKG1", 4);
	
	writeLong(0); // skip size
	writeInt(0); // skip base offset
	
	std::string copyright = "Package file v1.0 Copyright 2002 Wizet, ZMS";
	writeString(copyright, copyright.length()); // write copyrights
	writeByte(0); // end string

	baseOffset = file->tellp();

	writeIntAt(12, baseOffset); // write baseOffset

	base = new WZDirectory();

	// load directories and compile imgs
	loadDirectory(base);

	tstrings.clear();
	std::ofstream f("struct.dir", std::ios::out|std::ios::binary);
	file = &f;

	writeByte(encodeVersion()); // encoded version
	writeByte(0);

	// write directories info
	writeDirectoryInfo(base);

	// write the imgs
	writeIMGs(base);

	file = &tfile;

	f.close();
	//get size of the wz file
	size_t size = getIMGSize("struct.dir");

	char* bytes = new char[size];
	std::ifstream ff("struct.dir", std::ios::in|std::ios::binary);
	ff.read(bytes, size);
	ff.close();

	file->write(bytes, size);
	delete [] bytes;
	writeLongAt(4, size);

	leaveDirectory();
	leaveDirectory();
	
	file->close();
}
size_t Compiler::getIMGSize(std::string name){
	size_t size;
	std::ifstream f(name.c_str(), std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	size = f.tellg();
	f.close();
	return size;
}
size_t Compiler::getIMGsSize(){
	size_t size = 0;

	WIN32_FIND_DATAA FindFileData;
   
	HANDLE hFind = FindFirstFileA("*", &FindFileData);
	do{
		std::string name = std::string((char*)FindFileData.cFileName);
		if(*name.c_str() == '.' || name.find(".dir") != -1) continue;
		enterDirectory(name);
		if(name.find(".img") != -1){ // img
			size += getIMGSize(name);
		}
		else{
			size += getIMGsSize();
		}
		leaveDirectory();

	}
	while (FindNextFileA(hFind, &FindFileData));
	return size;
}
void Compiler::writeDirectoryInfo(WZDirectory* base){
	if(dirsoff.find(base) != dirsoff.end()){
		writeOffsetAt(dirsoff[base], file->tellp());
	}
	
	stdext::hash_map<std::string, WZDirectory*>* dirs = base->getDirectories();
	stdext::hash_map<std::string, IMGFile*>* imgs = base->getIMGs();

	writePackedInt(dirs->size() + imgs->size());

	for(stdext::hash_map<std::string, WZDirectory*>::iterator iter = dirs->begin(); iter != dirs->end(); iter++){
		enterDirectory(iter->first);
		writeString(iter->first, 3, 1, true);
		writePackedInt(getIMGsSize()); // calc size of the imgs in this directory
		writePackedInt(0); // checksum
		dirsoff[iter->second] = file->tellp();
		writeInt(0); // keep place for offset
		leaveDirectory();
	}
	for(stdext::hash_map<std::string, IMGFile*>::iterator iter = imgs->begin(); iter != imgs->end(); iter++){
		enterDirectory(iter->first);
		writeString(iter->first, 4, 2, true);
		writePackedInt(getIMGSize(iter->first)); // size of this img
		writePackedInt(0); // checksum
		imgsoff[iter->second] = file->tellp();
		writeInt(0); // keep place for offset
		leaveDirectory();
	}
	for(stdext::hash_map<std::string, WZDirectory*>::iterator iter = dirs->begin(); iter != dirs->end(); iter++){
		enterDirectory(iter->first);
		writeDirectoryInfo(iter->second);
		leaveDirectory();
	}
}
void Compiler::writeIMGs(WZDirectory* base){
	stdext::hash_map<std::string, WZDirectory*>* dirs = base->getDirectories();
	stdext::hash_map<std::string, IMGFile*>* imgs = base->getIMGs();

	for(stdext::hash_map<std::string, IMGFile*>::iterator iter = imgs->begin(); iter != imgs->end(); iter++){
		enterDirectory(iter->first);
		if(imgsoff.find(iter->second) != imgsoff.end()){
			writeOffsetAt(imgsoff[iter->second], file->tellp());
		}
		size_t size = getIMGSize(iter->first);
		char* bytes = new char[size];
		std::ifstream f(iter->first.c_str(), std::ios_base::in | std::ios_base::binary);
		f.read(bytes, size);
		f.close();
		file->write(bytes, size);
		leaveDirectory();
		delete [] bytes;
	}

	for(stdext::hash_map<std::string, WZDirectory*>::iterator iter = dirs->begin(); iter != dirs->end(); iter++){
		enterDirectory(iter->first);
		writeIMGs(iter->second);
		leaveDirectory();
	}
}
void Compiler::loadDirectory(WZDirectory* base){
	WIN32_FIND_DATAA FindFileData;
   
	HANDLE hFind = FindFirstFileA("*", &FindFileData);
	do{
		std::string name = std::string((char*)FindFileData.cFileName);
		if(*name.c_str() == '.' || name.find(".dir") != -1) continue;
		enterDirectory(name);
		if(name.find(".img") != -1){ // img
			IMGFile* img = new IMGFile(name);
			base->add(img);
			std::string nname = name;
			std::wstring fileName(nname.length(),L' ');
			std::copy(name.begin(), name.end(), fileName.begin()); // TODO: check that is a folder
			if(GetFileAttributes(fileName.c_str()) == INVALID_FILE_ATTRIBUTES){
			std::ofstream file(nname.c_str(), std::ios_base::out | std::ios_base::binary);
			this->file = &file;
			compileIMG();
			file.close();
			}
		}
		else{
			WZDirectory* dir = new WZDirectory(name);
			base->add(dir);
			loadDirectory(dir);
		}
		leaveDirectory();

	}
	while (FindNextFileA(hFind, &FindFileData));
}

std::string Compiler::getFolderName(){
	char name[200];
	_getcwd(name, 200);
	std::string rname = std::string(name);
	int p = rname.find_last_of("\\");
	if(p != -1)
		rname = rname.substr(p+1);
	return rname;
}

template <typename T>
T Compiler::getValue_IMG(std::string name){
	std::ifstream f((name + ".txt").c_str(), std::ios_base::in);
	T value;
	f >> value;
	f.close();
	return value;
}

template <>
std::string Compiler::getValue_IMG<std::string>(){
	std::ifstream f("value.txt", std::ios_base::in);
	std::string value;
	std::getline(f, value);
	f.close();
	return value;
}

template <typename T>
T Compiler::getValue_IMG(){
	std::ifstream f("value.txt", std::ios_base::in);
	T value;
	f >> value;
	f.close();
	return value;
}

std::string Compiler::getTypeFile(){
	std::ifstream f("type.txt", std::ios_base::in);
	std::string type;
	f >> type;
	f.close();
	return type;
}
WZObject* Compiler::getType_IMG(){
	std::string type = getTypeFile();
	std::string name = getFolderName();
	if(type == "WZ_EMPTY"){
		return new Empty(name);
	}
	else if(type == "WZ_SHORT"){
		return new Short(name, getValue_IMG<short>());
	}
	else if(type == "WZ_INTEGER"){
		return new Integer(name, getValue_IMG<int>());
	}
	else if(type == "WZ_FLOAT"){
		return new Float(name, getValue_IMG<float>());
	}
	else if(type == "WZ_DOUBLE"){
		return new Double(name, getValue_IMG<double>());
	}
	else if(type == "WZ_STRING"){
		return new String(name, getValue_IMG<std::string>());
	}
	else{ // object
		return new Object(name, getObject_IMG());
	}
}

WZObject* Compiler::getObject_IMG(){
	std::string type = getTypeFile();
	if(type == "WZ_PROPERTY"){
		Property* p = new Property();
		WIN32_FIND_DATAA FindFileData;
	   
		HANDLE hFind = FindFirstFileA("*", &FindFileData);
		do{
			std::string name = std::string((char*)FindFileData.cFileName);
			if(name.find(".") != -1) continue;
			enterDirectory(name);
			p->getObjects()->push_back(getType_IMG());
			leaveDirectory();
		}
		while (FindNextFileA(hFind, &FindFileData));
		return p;
	}
	else if(type == "WZ_VECTOR"){
		return new Vector(getValue_IMG<int>("x"), getValue_IMG<int>("y"));
	}
	else if(type == "WZ_CONVEX"){
		Convex* c = new Convex();
		WIN32_FIND_DATAA FindFileData;
	   
		HANDLE hFind = FindFirstFileA("*", &FindFileData);
		do{
			std::string name = std::string((char*)FindFileData.cFileName);
			if(name.find(".") != -1) continue;
			enterDirectory(name);
			c->getObjects()->push_back(getObject_IMG());
			leaveDirectory();
		}
		while (FindNextFileA(hFind, &FindFileData));
		return c;
	}
	else if(type == "WZ_CANVAS"){
		Canvas* c = new Canvas();
		WIN32_FIND_DATAA FindFileData;
	   
		HANDLE hFind = FindFirstFileA("*", &FindFileData);
		do{
			std::string name = std::string((char*)FindFileData.cFileName);
			if(name.find(".") != -1) continue;
			enterDirectory(name);
			c->getObjects()->push_back(getType_IMG());
			leaveDirectory();
		}
		while (FindNextFileA(hFind, &FindFileData));
		c->format = (ImageFormat)getValue_IMG<int>("format");
		char name[200];
		_getcwd(name, 200);
		c->file = std::string(name) + "\\image.png"; // save the location of the file
		return c;
	}
	else if(type == "WZ_SOUND"){
		Sound* s = new Sound();
		char name[200];
		_getcwd(name, 200);
		s->file = std::string(name) + "\\sound.mp3";
		s->bytesFile = std::string(name) + "\\bytes.dat";
		return s;
	}
	else if(type == "WZ_UOL"){
		return new UOL(getValue_IMG<std::string>());
	}
	return NULL; // impossible
}

void Compiler::compileIMG(){
	tstrings.clear();
	Property* p = (Property*)getObject_IMG();
	write(p);
	delete p;
}

char Compiler::encodeVersion(){
	char ver[5] = {0};

	char* cv = ver;

	vSum = 0;

	_itoa_s(version, cv, 5, 10);

	while(*cv){
		vSum <<= 5;
		vSum += (unsigned char)(*cv++ +1);
	}
	
	char s1 = (vSum >> 24) & 0xFF;
	char s2 = (vSum >> 16) & 0xFF;
	char s3 = (vSum >> 8) & 0xFF;
	char s4 = (vSum >> 0) & 0xFF;

	return (~(s1^s2^s3^s4));
}

void Compiler::writeIntAt(size_t at, int value){
	size_t pos = file->tellp();	
	file->seekp(at, std::ios_base::beg);

	writeInt(value);	

	file->seekp(pos, std::ios_base::beg);
}
void Compiler::writeLongAt(size_t at, __int64 value){
	size_t pos = file->tellp();	
	file->seekp(at, std::ios_base::beg);

	writeLong(value);	

	file->seekp(pos, std::ios_base::beg);
}
void Compiler::writeInt(int value){
	writeValue(value);
}
void Compiler::writeShort(short value){
	writeValue(value);
}
void Compiler::writeByte(char value){
	writeValue(value);
}
void Compiler::writeDouble(double value){
	writeValue(value);
}
void Compiler::writeFloat(float value){
	writeValue(value);
}
void Compiler::writeLong(__int64 value){
	writeValue(value);
}
void Compiler::writeOffsetAt(size_t at, size_t offset){
	size_t pos = file->tellp();	
	file->seekp(at, std::ios_base::beg);

	size_t noffset = (~(file->tellp()))*vSum - 0x581C3F6D;
	noffset = _lrotl(noffset, noffset & 0x1F);
	writeInt((noffset)^(offset - baseOffset));

	file->seekp(pos, std::ios_base::beg);
}
void Compiler::writePackedInt(int value){
	if(value <= _I8_MAX)
		writeByte(value);
	else{
		writeByte(_I8_MIN);
		writeInt(value);
	}
}
void Compiler::writePackedFloat(float value){
	if(value == 0) writeByte(0);
	else{
		writeByte(_I8_MIN);
		writeFloat(value);
	}
}

template <typename T>
void Compiler::writeType(T* obj, char type){
	writeString(obj->name, 0, 1);
	writeByte(type);
}
template <typename T>
void Compiler::writeObjects(T* obj){
	writePackedInt(obj->getObjects()->size());

	for(size_t i=0; i<obj->getObjects()->size(); i++){
		write((*obj->getObjects())[i]);
	}
}
void Compiler::write(WZObject* obj){
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
void Compiler::write(Empty* obj){
	writeType(obj, 0);
}
void Compiler::write(Short* obj){
	writeType(obj, 2);
	writeShort(obj->value);
}
void Compiler::write(Integer* obj){
	writeType(obj, 3);
	writePackedInt(obj->value);
}
void Compiler::write(Float* obj){
	writeType(obj, 4);
	writePackedFloat(obj->value);
}
void Compiler::write(Double* obj){
	writeType(obj, 5);
	writeDouble(obj->value);
}
void Compiler::write(String* obj){
	writeType(obj, 8);
	writeString(obj->value, 0, 1);
}
void Compiler::write(Object* obj){
	writeType(obj, 9);
	size_t offset = file->tellp();
	writeInt(0);
	write(obj->value);
	writeIntAt(offset, (size_t)file->tellp() - offset - 4); // size of the block
}
void Compiler::write(Property* obj){
	writeString("Property", 0x73, 0x1B);
	writeShort(0); // NULL
	writeObjects(obj);
}
void Compiler::write(Convex* obj){
	writeString("Shape2D#Convex2D", 0x73, 0x1B);
	writeObjects(obj);
}
void Compiler::write(Vector* obj){
	writeString("Shape2D#Vector2D", 0x73, 0x1B);
	writePackedInt(obj->x);
	writePackedInt(obj->y);
}
void Compiler::write(Canvas* obj){
	writeString("Canvas", 0x73, 0x1B);
	writeByte(0); // NULL
	writeByte(obj->getObjects()->size() > 0);
	if(obj->getObjects()->size() > 0){
		writeShort(0); // NULL
		writeObjects(obj);
	}

	// get if alpha(for modifided images)
	bool alpha = GetFileAttributes(L"alpha.txt") != INVALID_FILE_ATTRIBUTES;
	

	//load image
	FILE* f;
	fopen_s(&f, obj->file.c_str(), "rb");
	gdImagePtr image = gdImageCreateFromPng(f);
	fclose(f);

	// write the height and the width of the image
	int width = gdImageSX(image);
	int height = gdImageSY(image);
	
	writePackedInt(width);
	writePackedInt(height);

	ImageFormat format = obj->format;

	// FORMAT_565 and FORMAT_BIN will require more code, so I will just use 8888 and 4444	

	switch(format){
		case FORMAT_565: format = FORMAT_8888; break;
		case FORMAT_BIN: format = FORMAT_4444; break;
	}

	// write format
	switch(format){
		case FORMAT_4444: writePackedInt(1); break;
		case FORMAT_8888: writePackedInt(2); break;
	}
	writePackedInt(0);

	writeInt(0); // NULL
	// save the offset
	size_t offset = file->tellp();
	// keep place for the size
	writeInt(0);

	// get the pixels of the image
	unsigned char* pixels = new unsigned char[height*width*4];
	for(int i=0; i<height; i++){
		for(int j=0; j<width; j++){
			int pix = image->tpixels[i][j];
			pixels[i*width*4 + j*4] = gdTrueColorGetBlue(pix);
			pixels[i*width*4 + j*4 + 1] = gdTrueColorGetGreen(pix);
			pixels[i*width*4 + j*4 + 2] = gdTrueColorGetRed(pix);
			pixels[i*width*4 + j*4 + 3] = 255 - ((gdTrueColorGetAlpha(pix) << 1) + (gdTrueColorGetAlpha(pix) >> 6));
		}
	}

	size_t size = 0;
	switch(format){
		case FORMAT_4444: size = width*height*2; break;
		case FORMAT_8888: size = width*height*4; break;
	}

	// compress to 4444
	if(format == FORMAT_4444){
		unsigned char* tpix = new unsigned char[height*width*2];
		for(int i=0; i<height*width*4; i++){
			unsigned char pix = pixels[i];
			unsigned char npix1 = ((pix >> 4) << 4) + (pix >> 4), npix2 = (((pix >> 4) + 1) << 4) + ((pix >> 4) + 1);
			if(abs(npix1-pix) < abs(npix2-pix)) pix = npix1;
			else pix = npix2;
			pix >>= 4;
			if(i%2) tpix[i >> 1] += pix << 4;
			else tpix[i >> 1] = pix;
		}	
		delete [] pixels;
		pixels = tpix;
	}

	if(alpha){
		for(int i=0; i<height; i++){
			for(int j=0; j<width; j++){
				if(pixels[i*width*4 + j*4 + 3] == 0){
					pixels[i*width*4 + j*4] = 0;
					pixels[i*width*4 + j*4 + 1] = 0;
					pixels[i*width*4 + j*4 + 2] = 0;
				}
			}
		}	
	}

	size_t bsize = (size_t)(size*1.01 + 12);
	size_t size_left = size;
	unsigned char* bytes = new unsigned char[bsize];

	//compress bytes
	bsize = 0;
	int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate deflate state //
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
	ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
        throw "Compress error";

    // compress until end of file //
    do {
		strm.avail_in = min(CHUNK, size_left);
		memcpy_s(in, strm.avail_in, pixels + size - size_left, strm.avail_in); 
		size_left -= strm.avail_in;

        strm.next_in = in;

        // run deflate() on input until output buffer not full, finish
        //  compression if all of source has been read in *
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, Z_SYNC_FLUSH);    // no bad return value //
            have = CHUNK - strm.avail_out;
			memcpy_s(bytes+bsize, have, out, have);
			bsize += have;
			assert(bsize <= size*1.01 + 12);
        } while (strm.avail_out == 0);

        // done when last data in file processed 
    } while (size_left);

    // clean up and return 
    (void)deflateEnd(&strm);

	// write compressed data
	writeByte(0);

	file->write((char*)bytes, bsize);

	// write size
	writeIntAt(offset, bsize+1);

	// free memory 
	delete [] pixels;
	delete [] bytes;

	gdImageDestroy(image);


}
void Compiler::write(UOL* obj){
	writeString("UOL", 0x73, 0x1B);
	writeByte(0); // NULL
	writeString(obj->value, 0, 1);
}
unsigned char soundBytes[] = {0x83, 0xEB, 0x36, 0xE4, 0x4F, 0x52, 0xCE, 0x11, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70, 0x8B, 0xEB, 0x36, 0xE4, 0x4F, 0x52, 0xCE, 0x11, 0x9F, 0x53, 0x00, 0x20, 0xAF, 0x0B, 0xA7, 0x70, 0x00, 0x01, 0x81, 0x9F, 0x58, 0x05, 0x56, 0xC3, 0xCE, 0x11, 0xBF, 0x01, 0x00, 0xAA, 0x00, 0x55, 0x59, 0x5A};

void Compiler::write(Sound* obj){
	writeString("Sound_DX8", 0x73, 0x1B);
	writeByte(0); // NULL
	size_t size = getIMGSize(obj->file);
	size_t bytesSize = getIMGSize(obj->bytesFile);
	std::ifstream f(obj->file.c_str(), std::ios_base::in | std::ios_base::binary);
	char* sound = new char[size];
	char* bytes = new char[bytesSize];
	f.read(sound, size);
	f.close();
	std::ifstream bf(obj->bytesFile.c_str(), std::ios_base::in | std::ios_base::binary);
	bf.read(bytes, bytesSize);
	bf.close();
	writePackedInt(size);
	writePackedInt(0); // checksum TODO
	writeByte(2); // format?
	file->write((char*)soundBytes, sizeof(soundBytes));
	writeByte(bytesSize);
	file->write(bytes, bytesSize);
	file->write(sound, size);
	delete [] sound;
	delete [] bytes;
}
	

void Compiler::writeString(std::string str, int len){
	file->write(str.c_str(), len);
}
void Compiler::writeString(std::string str){
	size_t len = str.length();
	// todo unicode
	if(len <= _I8_MAX){
		writePackedInt(-(int)len);
	}
	else{
		writePackedInt(len);
	}

	char* bytes = (char*)str.c_str();
	
	unsigned char key = 0xAA;

	for(size_t i=0; i<len; i++)
		*bytes++ ^= keys[i] ^ key++;
	
	bytes -= len;

	file->write(bytes, len);
}
void Compiler::writeString(std::string str, int n, int e, bool wflag){
	if(tstrings.find(str) != tstrings.end()){
		writeByte(e);
		writeInt(tstrings[str]);
	}
	else{
		if(wflag) tstrings[str] = file->tellp();
		writeByte(n);
		if(!wflag) tstrings[str] = file->tellp();
		writeString(str);
	}
}

void Compiler::enterDirectory(std::string name) {
	std::wstring fileName(name.length(),L' ');
	std::copy(name.begin(), name.end(), fileName.begin()); // TODO: check that is a folder
	::SetCurrentDirectory(fileName.c_str());
}
void Compiler::leaveDirectory() {
	::SetCurrentDirectory(L"..");
}