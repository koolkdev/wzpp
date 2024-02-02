#include <fstream>
#include <assert.h>
#include <algorithm>
#include "WZObjects.h"
#include "WZFile.h"
#include "../zlib/zlib.h"
#include "../libpng/png.h"

extern unsigned char keys[_UI16_MAX];

unsigned char *block = new unsigned char[_UI16_MAX];
/*
void WZObject::deleteObject(WZObject* obj){
	switch(obj->type){
		case WZ_EMPTY: delete (Empty*)obj; break;
		case WZ_SHORT: delete (Short*)obj; break;
		case WZ_INTEGER: delete (Integer*)obj; break;
		case WZ_FLOAT: delete (Float*)obj; break;
		case WZ_DOUBLE: delete (Double*)obj; break;
		case WZ_STRING: delete (String*)obj; break;
		case WZ_OBJECT: delete (Object*)obj; break;
		case WZ_PROPERTY: delete (Property*)obj; break;
		case WZ_CONVEX: delete (Convex*)obj; break;
		case WZ_VECTOR: delete (Vector*)obj; break;
		case WZ_CANVAS: delete (Canvas*)obj; break;
		case WZ_UOL: delete (UOL*)obj; break;
		case WZ_SOUND: delete (Sound*)obj; break;
	}
}
*/
gdImagePtr Canvas::getImage(File* file) {

	//TODO: use just pixels(the biggest)

	file->fileSeek(offset + 1);

	size_t end = offset + size--;

	size_t newSize = 0;
	unsigned char *bytes = new unsigned char[size];

	unsigned short zlib_header = file->readShort();

	file->fileSeekNext(-2);

	if(zlib_header == 0x9C78){
		newSize = size;
		file->readBytes((char*)bytes, size);
	}
	else {
		while(file->filePos() < end){
			size_t blockSize = file->readInt();
			file->readBytes((char*)block, blockSize);
			for(size_t i=0; i<blockSize; i++){
				bytes[newSize++] = block[i] ^ keys[i];
			}			
		}
	}

	assert(file->filePos() == end);

	size_t uSize = 0;

	switch(format){
		case FORMAT_4444:
		case FORMAT_565: uSize = width * height * 2; break;
		case FORMAT_8888: uSize = width * height * 4; break;
		case FORMAT_BIN: uSize = width * height / 128; //break;
		default: throw "Invalid image format.";
	}
	
	unsigned char* uncompressed = new unsigned char[uSize];

	//create zlib stream
	z_stream strm;
	strm.zalloc		= Z_NULL;
    strm.zfree		= Z_NULL;
    strm.opaque		= Z_NULL;
	strm.avail_in	= newSize;
	strm.next_in	= bytes;
	strm.avail_out	= uSize;
	strm.next_out	= uncompressed;

	//init inflate stream
	if(inflateInit(&strm) != Z_OK)
		throw "Can't uncompress image";

	//attempt to inflate zlib stream
	if(inflate(&strm, Z_NO_FLUSH) != Z_OK)
		throw "Can't uncompress image";

	//tidy up zlib
    inflateEnd(&strm);

	/*unsigned char* t = new unsigned char[newSize];
	unsigned long tsize = newSize;
	compress(t, &tsize, uncompressed, uSize);
	for(int i=0; i<newSize; i++){
		if(t[i] != bytes[i]){
			printf("%x %x ", t[i], bytes[i]);
		}
	}*/

	unsigned char* pixels = new unsigned char[width * height * 4];

	if(format == FORMAT_4444)
	{
		for(size_t i = 0; i < uSize; i++)
		{
			unsigned char low	= uncompressed[i] & 0x0F;
			unsigned char high	= uncompressed[i] & 0xF0;

			pixels[(i << 1)] = (low << 4) | low;
			pixels[(i << 1) + 1] = high | (high >> 4);
		}
	}
	else if(format == FORMAT_8888)
	{
		memcpy(pixels, uncompressed, uSize);
	}
	else if(format == FORMAT_565)
	{
		for(size_t i = 0; i < uSize; i+=2)
		{
			unsigned char bBits = (uncompressed[i] & 0x1F) << 3;
			unsigned char gBits = ((uncompressed[i + 1] & 0x07) << 5) | ((uncompressed[i] & 0xE0) >> 3);
			unsigned char rBits = uncompressed[i + 1] & 0xF8;

			pixels[(i << 1)]		= bBits | (bBits >> 5);
			pixels[(i << 1) + 1]	= gBits | (gBits >> 6);
			pixels[(i << 1) + 2]	= rBits | (rBits >> 5);
			pixels[(i << 1) + 3]	= 0xFF;
		}
	}
	else if(format == FORMAT_BIN)
	{
		unsigned char byte = 0;
		int pixelIndex = 0;
		for(size_t i = 0; i < uSize; i++)
		{
			for(int j = 0; j < 8; j++)
			{
				byte = ((uncompressed[i] & (0x01 << (7 - j))) >> (7 - j)) * 255;
				for(int k = 0; k < 16; k++)
				{
					pixelIndex = (i << 9) + (j << 6) + (k << 2);
					pixels[pixelIndex]		= byte;
					pixels[pixelIndex + 1]	= byte;
					pixels[pixelIndex + 2]	= byte;
					pixels[pixelIndex + 3]	= 0xFF;
				}
			}
		}
	}

	gdImagePtr image = gdImageCreateTrueColor(width, height);
	gdImageAlphaBlending(image, false);
	gdImageFilledRectangle(image, 0, 0, width, height, gdImageColorAllocateAlpha(image, 0, 0, 0, 127));
	gdImageAlphaBlending(image, true);


	for(size_t i = 0; i < width; i++)
		for(size_t j = 0; j < height; j++)
			gdImageSetPixel(image, i, j, gdImageColorAllocateAlpha(image, pixels[j*width*4 + i*4 + 2], pixels[j*width*4 + i*4 + 1], pixels[j*width*4 + i*4], 127 - (pixels[j*width*4 + i*4 + 3] >> 1)));
		

	size++;

	delete [] bytes;
	delete [] uncompressed;
	delete [] pixels;

	gdImageAlphaBlending(image, false);
	gdImageSaveAlpha(image, true);

	return image;
}
/*
void Canvas::save(std::string filename, WZFile* file) {
	gdImagePtr image = getImage(file);

	FILE* pngFile;
	
	fopen_s(&pngFile, filename.c_str(), "wb");

	gdImagePng(image, pngFile);

	fclose(pngFile);

	gdImageDestroy(image);
}*/
size_t maxRows = 600;
size_t uncompressedSize = 800 * 600 * 2;
size_t writeSize = 800 * 600 * 4;
size_t compressSize = _UI16_MAX;

png_bytep* rows = new png_bytep[maxRows]; 
unsigned char *bytes = new unsigned char[_UI16_MAX];
unsigned char *compressed = new unsigned char[compressSize];
unsigned char *uncompressed = new unsigned char[uncompressedSize];
unsigned char *writeBytes = new unsigned char[writeSize];

void Canvas::save(std::string filename, File* file) {
	int index = filename.find(":");
	if(index != -1)
		filename.replace(index, 5, "colon");

	file->fileSeek(offset + 1);

	size--;

	if(size > compressSize){
		compressSize = size;
		delete [] compressed;
		compressed = new unsigned char[compressSize];
	}

	size_t end = offset + size + 1;
	size_t newSize = size;

	unsigned short zlib_header = file->readShort();

	file->fileSeekNext(-2);

	if(zlib_header == 0x9C78){
		file->readBytes((char*)compressed, size);
	}
	else {
		newSize = 0;
		while(file->filePos() < end){
			size_t blockSize = file->readInt();
			file->readBytes((char*)bytes, blockSize);
			for(size_t i=0; i<blockSize; i++){
				compressed[newSize++] = bytes[i] ^ keys[i];
			}	
		}			
	}

	assert(file->filePos() == end);

	size_t uSize = 0;

	switch(format){
		case FORMAT_4444:
		case FORMAT_565: uSize = width * height * 2; break;
		case FORMAT_8888: uSize = width * height * 4; break;
		case FORMAT_BIN: uSize = width * height / 128; break;
		default: throw "Invalid image format.";
	}
	
	if(uSize > uncompressedSize){
		uncompressedSize = uSize;
		delete [] uncompressed;
		uncompressed = new unsigned char[uncompressedSize];
	}
	if(width * height * 4 > writeSize){
		writeSize = width * height * 4;
		delete [] writeBytes;
		writeBytes = new unsigned char[writeSize];
	}
	if(height > maxRows){
		maxRows = height;
		delete [] rows;
		rows = new png_bytep[maxRows];
	}

	//create zlib stream
	z_stream strm;
	strm.zalloc		= Z_NULL;
    strm.zfree		= Z_NULL;
    strm.opaque		= Z_NULL;
	strm.avail_in	= newSize;
	strm.next_in	= compressed;
	strm.avail_out	= uSize;
	strm.next_out	= uncompressed;

	//init inflate stream
	if(inflateInit(&strm) != Z_OK)
		throw "Can't uncompress image";

	//attempt to inflate zlib stream
	if(inflate(&strm, Z_NO_FLUSH) != Z_OK)
		throw "Can't uncompress image";

	int bit_depth = 8;

	//tidy up zlib
    inflateEnd(&strm);
	if(format == FORMAT_4444)
	{
		for(size_t i = 0; i < uSize; i++)
		{
			unsigned char low	= uncompressed[i] & 0x0F;
			unsigned char high	= uncompressed[i] & 0xF0;

			writeBytes[(i << 1)] = (low << 4) | low;
			writeBytes[(i << 1) + 1] = high | (high >> 4);
		}
	}
	else if(format == FORMAT_8888)
	{
		memcpy(writeBytes, uncompressed, uSize);
	}
	else if(format == FORMAT_565)
	{
		for(size_t i = 0; i < uSize; i+=2)
		{
			unsigned char bBits = (uncompressed[i] & 0x1F) << 3;
			unsigned char gBits = ((uncompressed[i + 1] & 0x07) << 5) | ((uncompressed[i] & 0xE0) >> 3);
			unsigned char rBits = uncompressed[i + 1] & 0xF8;

			writeBytes[(i << 1)]		= bBits | (bBits >> 5);
			writeBytes[(i << 1) + 1]	= gBits | (gBits >> 6);
			writeBytes[(i << 1) + 2]	= rBits | (rBits >> 5);
			writeBytes[(i << 1) + 3]	= 0xFF;
		}
	}
	else if(format == FORMAT_BIN)
	{
		unsigned char byte = 0;
		int pixelIndex = 0;
		for(size_t i = 0; i < uSize; i++)
		{
			for(int j = 0; j < 8; j++)
			{
				byte = ((uncompressed[i] & (0x01 << (7 - j))) >> (7 - j)) * 255;
				for(int k = 0; k < 16; k++)
				{
					pixelIndex = (i << 9) + (j << 6) + (k << 2);
					writeBytes[pixelIndex]		= byte;
					writeBytes[pixelIndex + 1]	= byte;
					writeBytes[pixelIndex + 2]	= byte;
					writeBytes[pixelIndex + 3]	= 0xFF;
				}
			}
		}
	}/*
	if(format == FORMAT_4444){
		for(size_t i = 0; i < height; i++)
			rows[i] = &uncompressed[i * width * 2];
	}
	else{
		for(size_t i = 0; i < height; i++)
			rows[i] = &uncompressed[i * width * 4];
	}*/
	for(size_t i = 0; i < height; i++)
		rows[i] = &writeBytes[i * width * 4];

	//create PNG file
	FILE* pngFile;
	
	fopen_s(&pngFile, filename.c_str(), "wb");

	if(!pngFile)
		throw "Creating PNG error";

	//create png write and info structs
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		throw "Creating PNG error";

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		throw "Creating PNG error";

	//png_set_compression_mem_level(png_ptr, 1);

	//initialize IO
	if(setjmp(png_jmpbuf(png_ptr)))
		throw "Creating PNG error";
	png_init_io(png_ptr, pngFile);

	//write PNG header
	if(setjmp(png_jmpbuf(png_ptr)))
		throw "Creating PNG error";
	png_set_bgr(png_ptr);

	/*png_color_8_struct sig_bit;
	
	switch(format) {
		case FORMAT_4444:
			png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			sig_bit.red = 4;
			sig_bit.green = 4;
			sig_bit.blue = 4;
			sig_bit.alpha = 4;
			//png_ptr->usr_bit_depth = 4;
			png_set_sBIT(png_ptr, info_ptr, &sig_bit);
			break;
		case FORMAT_8888:
			png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			sig_bit.red = 8;
			sig_bit.green = 8;
			sig_bit.blue = 8;
			sig_bit.alpha = 8;
			//png_ptr->usr_bit_depth = 8;
			png_set_sBIT(png_ptr, info_ptr, &sig_bit);
			break;
		case FORMAT_565:
			png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			sig_bit.red = 5;
			sig_bit.green = 6;
			sig_bit.blue = 5;
			png_set_sBIT(png_ptr, info_ptr, &sig_bit);
			break;
		case FORMAT_BIN:
		/*	png_set_IHDR(PngStruct, PngInfo, Width, Height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
			sig_bit.red = 8;
			sig_bit.green = 8;
			sig_bit.blue = 8;
			sig_bit.alpha = 8;
			png_set_sBIT(PngStruct, PngInfo, &SigBits);
			break;
	}*/

	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	/*png_write_info(png_ptr, info_ptr);


	png_set_sBIT(png_ptr, info_ptr, &bits);*/
	png_write_info(png_ptr, info_ptr);

	//png_set_rows(PngStruct, PngInfo, RowPtrs);
	//png_set_shift( png_ptr, &sig_bit );
	//write image data
	if(setjmp(png_jmpbuf(png_ptr)))
		throw "Creating PNG error";
	png_write_image(png_ptr, rows);

	//write PNG end and close file
	if (setjmp(png_jmpbuf(png_ptr)))
		throw "Creating PNG error";
	png_write_end(png_ptr, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);


/*	png_structp PngStruct = NULL;
	png_infop PngInfo = NULL;
	PngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!PngStruct)
		return false;

	PngInfo = png_create_info_struct(PngStruct);
	if(!PngInfo)
	{
		png_destroy_write_struct(&PngStruct, NULL);
		return false;
	}


	png_color_8_struct SigBits;

	switch(Format)
	{
	case ImgA8:
		png_set_IHDR(PngStruct, PngInfo, Width, Height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		SigBits.gray = 8;
		png_set_sBIT(PngStruct, PngInfo, &SigBits);
		break;
	case ImgA16:
		png_set_IHDR(PngStruct, PngInfo, Width, Height, 16, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		SigBits.gray = 16;
		png_set_sBIT(PngStruct, PngInfo, &SigBits);
		break;
	case ImgR8G8B8:
		png_set_IHDR(PngStruct, PngInfo, Width, Height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		SigBits.red = 8;
		SigBits.green = 8;
		SigBits.blue = 8;
		png_set_sBIT(PngStruct, PngInfo, &SigBits);
		break;
	case ImgA8R8G8B8:
		png_set_IHDR(PngStruct, PngInfo, Width, Height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		SigBits.red = 8;
		SigBits.green = 8;
		SigBits.blue = 8;
		SigBits.alpha = 8;
		png_set_sBIT(PngStruct, PngInfo, &SigBits);
		break;
	default:
		png_destroy_write_struct(&PngStruct, &PngInfo);
		return false;
	}

	png_byte ** RowPtrs = new png_byte *[Height];

	int Pitch = Width * GetBytesPerPixel();
	for(int y = 0; y < Height; ++y)
		RowPtrs[y] = &m_ImageData[y * Pitch];

	png_set_rows(PngStruct, PngInfo, RowPtrs);

	png_write_png(PngStruct, PngInfo, g_PngTransforms, NULL); // g_PngTransforms = PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ALPHA

	png_destroy_write_struct(&PngStruct, &PngInfo);
	*/
	fclose(pngFile);

	size++;
}


void Sound::save(std::string filename, File* file) {
	std::ofstream mp3;

	file->fileSeek(offset);

	char *bytes = new char[size];
	file->readBytes(bytes, size);

	mp3.open((char*)filename.c_str(), std::ios::out | std::ios::binary);
	mp3.write(bytes, size);
	mp3.close();
	
	delete [] bytes;
}