#include "WZFile.h"
#include "Compiler.h"
#include "IMGFile.h"
#include "Extractor.h"
#include <iostream>
#include <time.h>

int main() {
	try {
		File::initialize();
		size_t start = clock();

		File file("0100100.img");
		file.open();
		/*WZFile file("String.wz");

		file.open();
		Extractor e(&file);
		//IMGFile img(&file);
		//img.load();
		//e.saveIMG(&img);
		file.extract();*/
		IMGFile f(&file, 0);
		f.load();
		Extractor e(&file);
		e.createDirectory(std::string("Mob"));
		e.enterDirectory(std::string("Mob"));
		e.saveIMG(&f);
		file.close();

		//Compiler compile("Mob.wz", 60);
		//compile.compile();

		std::cout << "Time: " << clock() - start;
	}
	catch(const char* err){
		std::cerr << "Exception catch: " << err << std::endl;
	}
	catch(const std::string &e){
		std::cerr << "Exception catch: " << e << std::endl;
	}
	catch( const std::exception &e ) {
		std::cerr << "Exception catch: " << e.what() << std::endl;
	}
}