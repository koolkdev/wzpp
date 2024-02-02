#ifndef ANIMATION_H
#define ANIMATION_H

#define APNG
#ifndef APNG
	//#define GIF_BLACK
#endif

#include "../gd/gd.h"
#include <stdio.h>
#include <string>
#include <vector>
class WZFile;
class IMGEntry;
struct Frame;

class Animation {
private:
	WZFile* file;
	std::string folder;
public:
	Animation(WZFile* file, std::string folder) : file(file), folder(folder) {}
	void findFolder(size_t baseOffset);
	void createAnimations(size_t baseOffset);
	void createAnimation(IMGEntry* animation);
#ifdef APNG
	void writeFrames(std::string name, std::vector <Frame>& frames, int imgWidth, int imgHeight, int xDiff, int yDiff);
	void writeFrame(void* png_ptr, void* info_ptr, Frame& frame, int xDiff, int yDiff);
#else
	void writeFrames(std::string name, std::vector <Frame>& frames);
	void writeFrame(FILE* file, Frame& frame, gdImagePtr last);
#endif
	IMGEntry* getRealImage(IMGEntry* ent);
	void fixFrameSize(Frame& frame, int imgWidth, int imgHeight, int xDiff, int yDiff);

	void createDirectory(std::string& name);
	void enterDirectory(std::string& name);
	void leaveDirectory();
};

#endif