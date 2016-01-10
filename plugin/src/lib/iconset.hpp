/**
 * \brief Icon loader library.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#pragma once
#ifndef __JREKCED_ICONSET_HPP__
#define __JREKCED_ICONSET_HPP__

#include <stdio.h>
#include <stdlib.h>

/** \brief Image RGBA class. */
class Image
{
private:
	int width;
	int height;
	unsigned char * data;
	
public:
	/** \brief Point color structure. */
	typedef struct RGBA
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	};

	/**
	 * \param width Image width.
	 * \param height Image height. 
	 */
	Image(int width, int height);

	~Image();
	
	/** 
	 * \brief Get image height.
	 * \return Image height. 
	 */
	int getHeight();

	/** 
	 * \brief Get image width.
	 * \return Image width. 
	 */
	int getWidth();

	/**
	 * \brief Get pointer to raw image.
	 * \return Pointer to raw image.
	 */
	unsigned char * getRawData();

	/**
	 * \brief Get pointer to RGBA image.
	 * \return Pointer to RGBA image.
	 */
	RGBA * getRGBA();
};

/** Icon loader. */
class Icon
{
public:
	#pragma pack(push, 1)
	typedef struct IconDirEntry /**< Icon info entry. */
	{
		const unsigned char  bWidth;          /**< Width, in pixels, of the image */
		const unsigned char  bHeight;         /**< Height, in pixels, of the image */
		const unsigned char  bColorCount;     /**< Number of colors in image (0 if >=8bpp) */
		const unsigned char  bReserved;       /**< Reserved (must be 0) */
		const unsigned short wPlanes;         /**< Color Planes */
		const unsigned short wBitCount;       /**< Bits per pixel */
		const unsigned long  dwBytesInRes;    /**< How many bytes in this resource ? */
		const unsigned long  dwImageOffset;   /**< Where in the file is this image ? */
	};
	#pragma pack(pop)

private:
	FILE *		   iconFile;
	IconDirEntry * entries;
	unsigned short entriesSize;

public:
	/**
	 * \param iconFilename Icon file path to load.
	 */
	Icon(const char * iconFilename);
	
	~Icon();
	
	/**
	 * \brief Get number of icon into ICO file.
	 * \return Number of icon into ICO file.
	 */
	unsigned short getSize();
	
	/**
	 * \brief Get one icon info entry.
	 * \param id Icon's id.
	 * \return Icon's info entry.
	 */
	IconDirEntry * getEntryInfo(int id);

	/**
	 * \brief Get icon's image.
	 * \param id Icon's id.
	 * \return Image icon in RGBA format.
	 */
	Image * getIcon(int id);
};

#endif
