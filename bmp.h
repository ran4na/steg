#ifndef STEG_BMP_H
#define STEG_BMP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mathutilities.h"

// 14 bytes
#pragma pack(1)
typedef struct BitMapFileHeader {
	uint16_t signature; // 'BM', 2 bytes
	uint32_t fileSize; // File size in bytes
	uint16_t reserved1; // unused field
	uint16_t reserved2; // unused field
	uint32_t dataOffset; // offset from beginning of file to BMP data
						// if infoheader is 40 bytes (+ 14), then its 54.
						// can be increased by color table size
} BMP_FILE_HEADER;
#pragma pack()
// 40 bytes
#pragma pack(1)
typedef struct BitMapInfoHeader {
	uint32_t infoHeaderSize;
	uint32_t width; // width of BMP in pixels
	uint32_t height; // height of BMP in pixels
	uint16_t planes; // number of planes (1)
	uint16_t bitsPerPixel;
					/*
						1: monochrome pallette
						4: 16 colors
						8: 256 colors
						16: 16-bit RGB, 65536 colors 
						24: 24-bit RGB 16 million colors // default
						32: 24-bit RGB + 8-bit A channel
					*/
	uint32_t compressionType;
					/*
						0: BI_RGB (no compression) // default
						1: BI_RLE8 (8-bit RLE encoding)
						2: BI_RLE4 (4-bit RLE encoding)
					*/
	uint32_t imageSize; // COMPRESSED size of image (=0 if compression = 0)
	uint32_t Xppm; // Horizontal pixels per meter
	uint32_t Yppm; // Vertical pixels per meter
	uint32_t colorsUsed; // number of colors used.
						// see bitsPerPixel (8-bit = 256 colors or 100h)
	uint32_t importantColors; // number of important colors (0 = all)
	// Color Table (4 * numCOlors) bytes if bpp < 8
} BMP_INFO_HEADER;
#pragma pack()
typedef struct BitmapFile {
	BMP_FILE_HEADER file_header;
	BMP_INFO_HEADER info_header;
	uint8_t* data; // pointer to BMP data
} BMP_FILE;



int initializeBMP(BMP_FILE* bmp, uint32_t width, uint32_t height, uint16_t bpp);
int initBmpInfoHeader(BMP_FILE* bmp, uint32_t width, uint32_t height, uint16_t bpp);
int initBmpFileHeader(BMP_FILE* bmp);
int writeToBMP(int byte, BMP_FILE* bmp, uint32_t value);

int encodeToFile_BMP(BMP_FILE* bmp, const char* text);
int encode_File_ToFile_BMP(BMP_FILE* bmp, FILE* infile);

int decode_ToFile_FromFile_BMP(const char* path, const char* output_path);
int decodeFromFile_BMP(const char* path);
void freeBMP(BMP_FILE* bmp);
int writeBmpToFile(const char* path, BMP_FILE* bmp);
int readBMPFromFile(const char* path, BMP_FILE* output);
#endif