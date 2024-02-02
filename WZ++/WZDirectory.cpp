#include "WZDirectory.h"
#include "IMGFile.h"

void WZDirectory::add(WZDirectory* dir){
	dirs[dir->getName()] = dir;

}
void WZDirectory::add(IMGFile* file){
	imgs[file->getName()] = file;
}