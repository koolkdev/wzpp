#ifndef WZDIRECTORY_H
#define WZDIRECTORY_H

#include <string>
#include <hash_map>

class IMGFile;

class WZDirectory {
private:
	std::string name;

	stdext::hash_map<std::string, WZDirectory*> dirs;
	stdext::hash_map<std::string, IMGFile*> imgs;
public:
	WZDirectory(std::string name) : name(name) {}
	WZDirectory(){}

	std::string& getName(){
		return name;
	}

	void add(WZDirectory* dir);
	void add(IMGFile* file);

	stdext::hash_map<std::string, WZDirectory*>* getDirectories(){
		return &dirs;
	}
	stdext::hash_map<std::string, IMGFile*>* getIMGs(){
		return &imgs;
	}

	WZDirectory* getDirectory(std::string name){
		if(dirs.find(name) == dirs.end()) return NULL;
		return dirs[name];
	}
	IMGFile* getIMG(std::string name){	
		if(imgs.find(name) == imgs.end()) return NULL;
		return imgs[name];
	}
};

#endif