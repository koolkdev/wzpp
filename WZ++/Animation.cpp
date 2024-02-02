#include "Animation.h"
#include "WZFile.h"
#include "WZObjects.h"
#include "IMGFile.h"
#include "IMGEntry.h"
#include <windows.h>
#include <algorithm>

#include "..\libpng\png.h"

void gdImageMergeAlpha(gdImagePtr first, gdImagePtr second, int x, int y){
	gdImageAlphaBlending(second, false);
	gdImageSaveAlpha(second, true);
	gdImageCopy(first, second, x, y, 0, 0, gdImageSX(second), gdImageSY(second));
}
void Animation::findFolder(size_t baseOffset){
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
		offset = file->readOffset();
		
		createDirectory(name);
		enterDirectory(name);

		size_t base = file->filePos();

		switch(type){
			case 0x02:
			case 0x04: if(name == folder){ createAnimations(offset); } break;
			case 0x03: findFolder(offset); break;
		}

		leaveDirectory();
	}
	file->fileSeek(cur);
}

void Animation::createAnimations(size_t baseOffset){
	size_t cur = file->filePos();
	file->fileSeek(baseOffset + file->getFileStart());
/*
	IMGFile img(file);

	img.load();
	
	IMGEntry* e = new IMGEntry(img.getProperty());

	stdext::hash_map <std::string, IMGEntry*>* entries = e->getEntries();
	
	for(stdext::hash_map <std::string, IMGEntry*>::iterator iter = entries->begin(); iter != entries->end(); iter++){
		if(iter->first != "info"){
			createAnimation(iter->second);
		}
	}
*/
	file->fileSeek(cur);
}

struct Frame {
	int id;
	gdImagePtr image;
	int delay;
	int x;
	int y;
};

bool operator<(const Frame& a, const Frame& b){
	return a.id < b.id;
}

IMGEntry* Animation::getRealImage(IMGEntry* ent){
	if(ent->getValue() == NULL) return NULL;
	if(ent->getValue()->type == WZ_CANVAS)
		return ent;
	else if(ent->getValue()->type = WZ_UOL){
		if(((UOL*)(ent->getValue()))->value != "")
			return getRealImage((IMGEntry*)(ent->getParent()->getChild(((UOL*)ent->getValue())->value))); // add check?
	}
	return NULL;
}

void Animation::createAnimation(IMGEntry* animation){
	stdext::hash_map <std::string, IMGEntry*>* entries = animation->getEntries();

	bool zigzag = animation->getChild("zigzag") != NULL;

	std::vector <Frame> frames;
	
	for(stdext::hash_map <std::string, IMGEntry*>::iterator iter = entries->begin(); iter != entries->end(); iter++){
		if(iter->first.c_str()[0] < '0' || iter->first.c_str()[0] > '9'){
			createAnimation(iter->second);
			continue;
		}
		IMGEntry* frame = getRealImage(iter->second);
		if(frame == NULL){
			createAnimation(iter->second);
			continue;
		}
		if(frame->getChild("origin") != NULL){
			Frame f = {atoi((char*)(iter->first.c_str())),\
				frame->getCanvas()->getImage(file),\
				frame->getInt("delay"),\
				frame->getVector("origin")->x,\
				frame->getVector("origin")->y};
			frames.push_back(f); 
		}
		else{
			createAnimation(iter->second);
			continue;
		}
	}
	if(frames.size() == 0) return;

	std::string name = animation->getName();

	while(animation->getParent() != NULL && animation->getParent()->getName() != ""){
		animation = animation->getParent();
		name = animation->getName() + "." + name;
	}

	std::sort(frames.begin(), frames.end());
	int maxX = 0, maxY = 0, minX = 0, minY = 0;
	for(size_t i=0; i<frames.size(); i++){
		int imageX = gdImageSX(frames[i].image), imageY = gdImageSY(frames[i].image);

		if(frames[i].x > maxX) maxX = frames[i].x;
		if(frames[i].y > maxY) maxY = frames[i].y;
		if(imageX - frames[i].x > minX) minX = imageX - frames[i].x;
		if(imageY - frames[i].y > minY) minY = imageY - frames[i].y;
	}

	int imgWidth = maxX + minX;
	int imgHeight = maxY + minY;
	int xDiff = maxX;
	int yDiff = maxY;
	
#ifdef APNG
	// Fix size of first frame
	fixFrameSize(frames[0], imgWidth, imgHeight, xDiff, yDiff);
#else
	for(size_t i=0; i<frames.size(); i++){
		fixFrameSize(frames[i], imgWidth, imgHeight, xDiff, yDiff); // fix size of frames
	}
#endif

	size_t oSize = frames.size();

	if(zigzag){
		for(int i = (int)oSize - 1; i >= 0; i--){
			frames.push_back(frames[i]);
		}
	}

#ifdef APNG
	writeFrames(name, frames, imgWidth, imgHeight, xDiff, yDiff);
#else
	writeFrames(name, frames);
#endif

	for(size_t i=0; i<oSize; i++){
		gdImageDestroy(frames[i].image);
	}
}

#ifdef APNG
void Animation::writeFrames(std::string name, std::vector <Frame>& frames, int imgWidth, int imgHeight, int xDiff, int yDiff) {
#else
void Animation::writeFrames(std::string name, std::vector <Frame>& frames) {
#endif
	FILE* file;
	
#ifdef APNG
	fopen_s(&file, (name + ".png").c_str(), "wb");
#else
	fopen_s(&file, (name + ".gif").c_str(), "wb");
#endif

	if(!file)
		throw "Creating file error";

#ifdef APNG
	//create png write and info structs
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		throw "Creating PNG error";

	png_infop info_ptr = png_create_info_struct(png_ptr);
 	if(!info_ptr)
		throw "Creating PNG error";

	//initialize IO
	if(setjmp(png_jmpbuf(png_ptr)))
		throw "Creating PNG error";
	png_init_io(png_ptr, file);

	//write PNG header
	if(setjmp(png_jmpbuf(png_ptr)))
		throw "Creating PNG error";
	png_set_bgr(png_ptr);

	// text
	png_text pngtext[2] = {0};
	info_ptr->text = pngtext;
	info_ptr->num_text = 2;
	pngtext[0].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[0].key = "Author";
	pngtext[0].text = "koolk - www.koolk.net";
	pngtext[1].compression = PNG_TEXT_COMPRESSION_NONE;
	pngtext[1].key = "For more extractions";
	pngtext[1].text = "www.southperry.net";

	png_set_IHDR(png_ptr, info_ptr, imgWidth, imgHeight,
			8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// compression level
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	png_set_acTL(png_ptr, info_ptr, frames.size(), 0);

	png_write_info(png_ptr, info_ptr);
#endif

	for(size_t i=0; i<frames.size(); i++){
#ifdef APNG
		writeFrame(png_ptr, info_ptr, frames[i], xDiff, yDiff);
#else
		writeFrame(file, frames[i], (i == 0) ? NULL : frames[i-1].image);
#endif
	}

#ifdef APNG
	png_write_end(png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
#else
	gdImageGifAnimEnd(file);
#endif

	fclose(file);
}

#ifdef APNG
void Animation::writeFrame(void* png_ptr_p, void* info_ptr_p, Frame& frame, int xDiff, int yDiff){
#else
void Animation::writeFrame(FILE* file, Frame& frame, gdImagePtr last){
#endif
#ifdef APNG
	png_structp png_ptr = (png_structp)png_ptr_p;
	png_infop info_ptr = (png_infop)info_ptr_p;
	int height = gdImageSY(frame.image);
	int width = gdImageSX(frame.image);
	png_bytepp rows = new png_bytep[height];
	png_bytepp p = rows;
	png_bytep c;
	for(int i=0; i<height; i++){
		*p = new png_byte[width*4];
		c = *p++;
		for(int j=0; j<width; j++){
			int pix = frame.image->tpixels[i][j];
			*c++ = gdTrueColorGetBlue(pix);
			*c++ = gdTrueColorGetGreen(pix);
			*c++ = gdTrueColorGetRed(pix);
			*c++ = 255 - ((gdTrueColorGetAlpha(pix) << 1) + (gdTrueColorGetAlpha(pix) >> 6));
		}
	}

	png_write_frame_head(png_ptr, info_ptr, rows, gdImageSX(frame.image), gdImageSY(frame.image), xDiff - frame.x, yDiff - frame.y, frame.delay, 1000, PNG_DISPOSE_OP_PREVIOUS, PNG_BLEND_OP_OVER);
	
	png_write_image(png_ptr, rows);
	png_write_frame_tail(png_ptr, info_ptr);
#else
	if(last == NULL){
		gdImageGifAnimBegin(frame.image, file, 1, 0);	
	}
	gdImageGifAnimAdd(frame.image, file, 1, 0, 0, frame.delay/10, 0, last);
#endif
}

void Animation::fixFrameSize(Frame& frame, int imgWidth, int imgHeight, int xDiff, int yDiff) {

	// Transparety
	gdImagePtr image = gdImageCreateTrueColor(imgWidth, imgHeight);

	gdImageAlphaBlending(image, false);
#ifdef APNG
	int trans = gdImageColorAllocateAlpha(image, 0, 0, 0, 127);
#else
#ifdef GIF_BLACK
	int trans = gdImageColorAllocate(image, 0, 0, 0);
#else
	int trans = gdImageColorAllocate(image, 255, 255, 255);
#endif
#endif
	gdImageFilledRectangle(image, 0, 0, imgWidth, imgHeight, trans);
	gdImageAlphaBlending(image, true);

	gdImageMergeAlpha(image, frame.image, xDiff - frame.x, yDiff - frame.y);

	// Transparety
	gdImageAlphaBlending(image, false);
	gdImageSaveAlpha(image, true);

	gdImageDestroy(frame.image);
	frame.image = image;
	frame.x = xDiff;
	frame.y = yDiff;
}

void Animation::createDirectory(std::string& name) {
	std::wstring fileName(name.length(),L' ');
	std::copy(name.begin(), name.end(), fileName.begin());
	::CreateDirectory(fileName.c_str(), NULL);
}

void Animation::enterDirectory(std::string& name) {
	std::wstring fileName(name.length(),L' ');
	std::copy(name.begin(), name.end(), fileName.begin());
	::SetCurrentDirectory(fileName.c_str());
}
void Animation::leaveDirectory() {
	::SetCurrentDirectory(L"..");
}