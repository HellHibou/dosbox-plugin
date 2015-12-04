#pragma once

#include <stdio.h>
#include <stdlib.h>

class Image
{
private:
	int width;
	int height;
	unsigned char * data;
	
public:
	typedef struct RGBA
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};

	Image(int width, int height);
	~Image();
	
	int getHeight();
	int getWidth();
	unsigned char * getRawData();
	RGBA * getRGBA();
};


class Icon
{
public:
	#pragma pack(push, 1)
	typedef struct IconDirEntry
	{
		const unsigned char  bWidth;          // Width, in pixels, of the image
		const unsigned char  bHeight;         // Height, in pixels, of the image
		const unsigned char  bColorCount;     // Number of colors in image (0 if >=8bpp)
		const unsigned char  bReserved;       // Reserved ( must be 0)
		const unsigned short wPlanes;         // Color Planes
		const unsigned short wBitCount;       // Bits per pixel
		const unsigned long  dwBytesInRes;    // How many bytes in this resource?
		const unsigned long  dwImageOffset;   // Where in the file is this image?
	};
	#pragma pack(pop)

private:
	FILE *		   iconFile;
	IconDirEntry * entries;
	unsigned short entriesSize;

public:
	Icon(const char * iconFilename);
	~Icon();
	unsigned short getSize();
	
	IconDirEntry * getEntryInfo(int id);
	Image * getIcon(int id);
};
