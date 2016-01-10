/**
 * \brief Icon loader library.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */

#include <math.h> 
#include "iconset.hpp"

#define MAGIC_PNG_1 (0x474E5089)
#define MAGIC_PNG_2 (0x0A1A0A0D)
#define MAGIC_BMP   (0x00000028)

#if defined (ICON_RESSOURCE)
#pragma pack( push )
#pragma pack( 2 )
#endif
	
struct IconDir
{
	const unsigned short idReserved;   // Reserved (must be 0)
	const unsigned short idType;       // Resource Type (1 for icons)
	const unsigned short idCount;      // How many images?
};
#if defined (ICON_RESSOURCE)
#pragma pack( pop )
#endif

struct BmpEntry
{
    int   width;
    int   height;
    short planes;
    short bitCount;
    int   compression;
    int   imageSize;
    int   xPixelsPerM;
    int   yPixelsPerM;
    int   colorsUsed;
    int   colorsImportant;
};

class BmpPalette
{
public:
	unsigned char * r;
	unsigned char * g;
	unsigned char * b;

	BmpPalette(int colorCount)
	{
		r = (unsigned char *) malloc(colorCount);
		g = (unsigned char *) malloc(colorCount);
		b = (unsigned char *) malloc(colorCount);
	}

	~BmpPalette()
	{
		if (r != NULL) free(r);
		if (g != NULL) free(g);
		if (b != NULL) free(b);
	}
};

Image::Image(int myWidth, int myHeight)
{
	height = myHeight;
	width = myWidth;
	data = (unsigned char *) calloc (0x000000FF, width * height * 4 );
}

Image::~Image()
{
	if (data != NULL) { free(data); }
}

int Image::getHeight() { return height; }
int Image::getWidth()  { return width;  }

unsigned char * Image::getRawData() { return data; }

Image::RGBA * Image::getRGBA() { return (RGBA*) data; }

Icon::Icon(const char * iconFilename)
{
	IconDir     * header = (IconDir*) malloc(sizeof(IconDir));
	entries     = NULL;
	entriesSize = 0;

	iconFile = fopen (iconFilename,"rb");
	if (iconFile == NULL)  { throw (1); }
	
	if (fread(header, sizeof(IconDir), 1, iconFile) < 1)
	{ 
		fclose (iconFile);
		iconFile =  NULL;
		throw (2); 
	}

	if (header->idReserved != 0) 
	{ 
		fclose (iconFile);
		iconFile = NULL;
		throw (2); 
	}

	if (header->idType != 1) 
	{ 
		fclose (iconFile);
		iconFile = NULL;
		throw (2); 
	}

	entriesSize = header->idCount;

	free(header);
	entries = (IconDirEntry *) malloc (sizeof(IconDirEntry) * entriesSize);
	if (fread(entries, sizeof(IconDirEntry), entriesSize, iconFile) < entriesSize)
	{ 
		fclose (iconFile);
		iconFile = NULL;
		throw (3); 
	}

}

Icon::~Icon()
{
	if (iconFile != NULL) { fclose (iconFile); }
	if (entries != NULL)  { free (entries); }
}
	
unsigned short Icon::getSize() { return entriesSize; }

Icon::IconDirEntry * Icon::getEntryInfo(int id)
{ 
	if (id >= entriesSize) return NULL;
	return &entries[id]; 
}

Image * Icon::getIcon(int id)
{
	if (iconFile == NULL) return NULL;
	if (id >= entriesSize) return NULL;
		
	fseek(iconFile, entries[id].dwImageOffset, SEEK_SET);
	int magic;
	if (fread(&magic, sizeof(int), 1, iconFile) != 1) { return NULL; }

	switch(magic)
	{
		case MAGIC_PNG_1: //PNG Icon Windows 7
/*			if (fread(&magic, sizeof(int), 1, iconFile) < 1) { return NULL; }
			if (magic != MAGIC_PNG_2) { return NULL; }
		
			// read PNG file not implemented
*/			return NULL;
			
		case MAGIC_BMP:
		{
			BmpEntry bmpEntry;
			if (fread(&bmpEntry, sizeof(BmpEntry), 1, iconFile) != 1) { return NULL;}
			bmpEntry.height /= 2;
			if (bmpEntry.compression != 0) { return NULL; }

			BmpPalette * colorPalette;
			char * dataLine; 
            int dataPerLine;
            int bytesPerLine;

            if (bmpEntry.bitCount <= 8)
            {
                dataPerLine = bmpEntry.width;

                int bitsPerLine = dataPerLine * bmpEntry.bitCount;
                if (bitsPerLine % 32 != 0) { bitsPerLine = (bitsPerLine / 32 + 1) * 32; }
                bytesPerLine = (int) (bitsPerLine / 8);

                // Palette de couleurs
				int numColors = pow (2.0, bmpEntry.bitCount);
				colorPalette = new BmpPalette(numColors);
				char rgb [3];

                for (int boucleColor = 0; boucleColor < numColors; boucleColor++)
                {
					if (fread(&rgb, 1, 3, iconFile) != 3)
					{ 
						free(colorPalette);
						return NULL; 
					}
                    colorPalette->b[boucleColor] = rgb[0];
                    colorPalette->g[boucleColor] = rgb[1];
                    colorPalette->r[boucleColor] = rgb[2];
                    fseek (iconFile, 1, SEEK_CUR); // Champ 'Reserved'
                }
            }
            else
            {
                colorPalette = NULL;
                dataPerLine  = bmpEntry.width  * (bmpEntry.bitCount / 8);
                bytesPerLine = dataPerLine;
				dataLine     = NULL;
            }

            if (bytesPerLine % 4 != 0) { bytesPerLine = (bytesPerLine / 4 + 1) * 4;  }
			dataLine = (char*)malloc(bytesPerLine); 

			if (dataLine == NULL)
			{ 
				if (colorPalette != NULL) { free(colorPalette); }
				return NULL;
			}

			Image * img = new Image(entries[id].bWidth, entries[id].bHeight);
			Image::RGBA * imgData = img->getRGBA();

			if (bmpEntry.bitCount <= 8)
			{
				int colorIndex;

				for (int boucleY = bmpEntry.height - 1; boucleY >= 0; boucleY--) 
                {
					if (fread(dataLine, bytesPerLine, 1, iconFile) != 1)
					{ 
						free(dataLine);
						free(colorPalette);
						delete img; 
						return NULL;
					}
					
					for (int boucleX = 0; boucleX < bmpEntry.width; boucleX++)
                    {
						int dataIndex = (boucleY * bmpEntry.height) + boucleX;

						switch (bmpEntry.bitCount)
						{
							 case 1: // Palette de 2 Couleurs
								  colorIndex = (dataLine[( boucleX / 8)] >> (7 - (boucleX % 8))) & 1;
								  break;

							 case 4: // Palette de 16 Couleurs
								 colorIndex = (dataLine[(boucleX / 2)] >> (4 * (1 - (boucleX % 2)))) & 0x0F;
								 break;

							 case 8:
								 colorIndex = dataLine[boucleX];
								 
						}
						dataIndex++;
						imgData[dataIndex].r = colorPalette->r[colorIndex];
						imgData[dataIndex].g = colorPalette->g[colorIndex];
						imgData[dataIndex].b = colorPalette->b[colorIndex];
					}
				}
	
				free(colorPalette);
			}
			else 
            {
				int padBytesPerLine = bytesPerLine - dataPerLine;

                for (int boucleY = bmpEntry.height - 1; boucleY >= 0; boucleY--)
                {
					if (fread(dataLine, bytesPerLine, 1, iconFile) != 1)
					{ 
						free(dataLine);
						delete img; 
						return NULL;
					}

					switch (bmpEntry.bitCount)
					{
						case 24: // RGB
							for (int boucleX = 0; boucleX < bmpEntry.width; boucleX++) 
							{
								int dataIndex = (boucleY * bmpEntry.height) + boucleX;
								imgData[dataIndex].b = dataLine[(boucleX*3)];
								imgData[dataIndex].g = dataLine[(boucleX*3)+1];
								imgData[dataIndex].r = dataLine[(boucleX*3)+2];
								dataIndex++;
							}   
							break;

						case 32: // RGBA
							for (int boucleX = 0; boucleX < bmpEntry.width; boucleX++) 
							{
								int dataIndex = (boucleY * bmpEntry.height) + boucleX;
								imgData[dataIndex].b = dataLine[(boucleX*4)];
								imgData[dataIndex].g = dataLine[(boucleX*4)+1];
								imgData[dataIndex].r = dataLine[(boucleX*4)+2];
								imgData[dataIndex].a = dataLine[(boucleX*4)+3];
								dataIndex++;
							}  

						default:
							continue;
					}

					fseek (iconFile, padBytesPerLine, SEEK_CUR);
				}
			}
			
			free(dataLine);

            // Lecture du masque
			
            if (bmpEntry.bitCount != 32)
            {
                dataPerLine = bmpEntry.width;
                int bitsPerLine = dataPerLine;
                if (bitsPerLine % 32 != 0) { bitsPerLine = (bitsPerLine / 32 + 1) * 32; }
                bytesPerLine = (int) (bitsPerLine / 8);
                dataLine = (char *) malloc (bytesPerLine);
				int dataIndex = 0;

				if (dataLine == NULL)
				{
					free(img);
					return NULL;
				}

                for (int boucleY = bmpEntry.height - 1; boucleY >= 0; boucleY--)
                {
                    if (fread(dataLine, bytesPerLine, 1, iconFile) != 1)
					{
						free(dataLine);
						delete img;
						return NULL;
					}

                    for (int boucleX = 0; boucleX < bmpEntry.width; boucleX++)
                    {
                        int colorIndex = (dataLine[( boucleX / 8)] >> (7 - (boucleX % 8))) & 1;
						
                        if (colorIndex == 0)
						{ imgData[dataIndex].a = 0xFF; }
                        else
                        { imgData[dataIndex].a = 0x00; }
						
						dataIndex++;
                    }
                }
				
				free(dataLine);
            }
			return img;
		}
			
		default:
			return NULL;
	}
}	
