#include "bmp.h"

int initializeBMP(BMP_FILE* bmp, uint32_t width, uint32_t height, uint16_t bpp) {
	initBmpInfoHeader(bmp, width, height, bpp);
	initBmpFileHeader(bmp);
	int imageSizeBytes = (width * height * (bpp / 8)) + 54 + 84;
	// round up to nearest multiple of 4
	imageSizeBytes += (imageSizeBytes % 4);
	// allocate pixel data
	bmp->data = (uint8_t*) malloc(imageSizeBytes * sizeof(uint8_t));
	return 0;
}
int initBmpInfoHeader(BMP_FILE* bmp, uint32_t width, uint32_t height, uint16_t bpp) {
	BMP_INFO_HEADER* header = &(bmp->info_header);
	header->infoHeaderSize = 40;
	header->width = width;
	header->height = height;
	header->planes = 1;
	header->bitsPerPixel = bpp;
	// haven't implemented decompression
	header->compressionType = 0;
	header->imageSize = 0; // not compressed
	header->Xppm = 72;
	header->Yppm = 72;
	header->colorsUsed = (1 << bpp) - 1; // max number byte can hold
	header->importantColors = 0; // all colors are important

	return 1;
}

void freeBMP(BMP_FILE* bmp) {
	free(bmp->data);
	free(bmp);
}

int initBmpFileHeader(BMP_FILE* bmp) {
	if (bmp->info_header.width == 0 || bmp->info_header.height == 0) {
		return 0;
	}
	BMP_FILE_HEADER* header = &(bmp->file_header);
	header->signature = 0x4D42;
	// width * height * bpp
	header->fileSize = 	bmp->info_header.width * bmp->info_header.height * bmp->info_header.bitsPerPixel;
	header->reserved1 = 0x0000;
	header->reserved2 = 0x0000;
	header->dataOffset = 54;
	
	return 1;
}


int writeToBMP(int byte, BMP_FILE* bmp, uint32_t value)
{
	int byteArraySize = bmp->info_header.width * bmp->info_header.height * (bmp->info_header.bitsPerPixel / 8);

	if (byte > byteArraySize) {
		printf("Error: Tried writing outside file bounds!\n");
	}
	bmp->data[byte] = value;
	return 1;
}

int encodeToFile_BMP(BMP_FILE* bmp, const char* text) {
	if (text == NULL) return 0;
	int stringLengthBits = (strlen(text) + 1) * 8; // string length (including null)

	char curChar = '\0';
	uint8_t currentByte = 0;
	uint32_t maxBits = bmp->info_header.width * bmp->info_header.height * bmp->info_header.bitsPerPixel;
	if (stringLengthBits >= maxBits) {
		printf("ERROR: Encode data too large!\n");
		printf("string bits: %d | max bits: %d\n", stringLengthBits, maxBits);
		return 0;
	}
	int charBitIndex = 0;
	int value = 0;
	// keep track of which bit we're on, and encode that into the sample
	for (int i = 0; i < stringLengthBits; i++) {
		// get current sample
		currentByte = bmp->data[i];
		// get i'th bit of string
		value = (text[i / 8] >> (i % 8)) & 1;
		currentByte = currentByte & 0xFE | value;
		// encode into BMP
		writeToBMP(i, bmp, currentByte);
	}
	printf("Encoding complete.\n");
	return 1;
}

int encode_File_ToFile_BMP(BMP_FILE* bmp, FILE* infile)
{
	char curChar = '\0';
	uint8_t currentByte = 0;
	int charBitIndex = 0;
	int value = 0;
	int size_read = 0;
	int progress = 0;
	while (!feof(infile)) {
		// read a character into the file
		size_read = fread(&curChar, 1, 1, infile);

		printf("Writing character: %c\n", curChar);
		for (int i = 0; i < 8; i++) {
			currentByte = bmp->data[progress + i];
			// get i'th bit of string
			value = (curChar >> i) & 1;
			currentByte = currentByte & 0xFE | value;
			// encode into BMP
			printf("Writing %d to index %d\n", value, progress + i);
			writeToBMP(progress + i, bmp, currentByte);
		}
		progress += 8;
	}

	return 0;
}

int decode_ToFile_FromFile_BMP(const char* path, const char* output_path) {
	BMP_FILE* bmp = malloc(sizeof(BMP_FILE));
	FILE* outfile = fopen(output_path, "w+b");
	readBMPFromFile(path, bmp);
	
	char curChar = '\0';
	int charBitIndex = 0;
	uint8_t currentByte = 0;
	int value = 0;

	if (bmp == NULL) {
		printf("Could not read BMP file!\n");
		return -1;
	}
	uint32_t numBytes = bmp->info_header.height * bmp->info_header.width * (bmp->info_header.bitsPerPixel / 8);
	for (int i = 0; i < numBytes; i++) {
		currentByte = bmp->data[i];
		value = currentByte & 1; // get LSB of byte
		curChar |= value << charBitIndex;
		charBitIndex++;
		if (charBitIndex == 8) {
			charBitIndex = 0;
			if (curChar == '\0') {
				break;
			}
			else {
				//printf("%c", curChar);
				fwrite(&curChar, 1, 1, outfile);
			}
			curChar = '\0';
		}
	}
	return 0;
}


int decodeFromFile_BMP(const char* path) {
	BMP_FILE* bmp = malloc(sizeof(BMP_FILE));

	readBMPFromFile(path, bmp);

	char curChar = '\0';
	int charBitIndex = 0;
	uint8_t currentByte = 0;
	int value = 0;

	if (bmp == NULL) {
		printf("Could not read BMP file!\n");
		return -1;
	}
	uint32_t numBytes = bmp->info_header.height * bmp->info_header.width * (bmp->info_header.bitsPerPixel / 8);
	for (int i = 0; i < numBytes; i++) {
		currentByte = bmp->data[i];
		value = currentByte & 1; // get LSB of byte
		curChar |= value << charBitIndex;
		charBitIndex++;
		if (charBitIndex == 8) {
			charBitIndex = 0;
			if (curChar == '\0') {
				break;
			}
			else {
				printf("%c", curChar);
			}
			curChar = '\0';
		}
	}
	return 0;
}

int writeBmpToFile(const char* path, BMP_FILE* bmp) {
	FILE* outFile;
	fopen_s(&outFile, path, "w+b");

	if (outFile == NULL) {
		printf("Failed to open %s!\n", path);
		return 0;
	}
	int byteArraySize = bmp->info_header.width * bmp->info_header.height * (bmp->info_header.bitsPerPixel / 8);
	// write data to BMP file
	fwrite(&(bmp->file_header), sizeof(BMP_FILE_HEADER), 1, outFile);
	fwrite(&(bmp->info_header), sizeof(BMP_INFO_HEADER), 1, outFile);
	fwrite(bmp->data, byteArraySize, 1, outFile);

	fclose(outFile);

	return 1;
}
int readBMPFromFile(const char* path, BMP_FILE* output) {
	FILE* inFile;
	fopen_s(&inFile, path, "r+b");

	if (inFile == NULL) {
		printf("Failed to open %s!\n", path);
		return 0;
	}

	// read file header
	fread(&(output->file_header), sizeof(BMP_FILE_HEADER), 1, inFile);
	// read info header
	fread(&(output->info_header), sizeof(BMP_INFO_HEADER), 1, inFile);
	if (output->info_header.compressionType != 0) {
		printf("ERROR: Compressed BMP files aren't supported.\n");
		return 0;
	}
	// read past data offset to start of image data, if there's anything in between
	fseek(inFile, output->file_header.dataOffset, SEEK_SET);
	// allocate space for byte array
		// read to byte array (size: width * height * bits per pixel)
	
	int byteArraySize = output->info_header.width * output->info_header.height * (output->info_header.bitsPerPixel / 8);

	output->data = (uint8_t*)malloc(byteArraySize);
	if (output->data != NULL) {
		fread(output->data, byteArraySize, 1, inFile);
	}
	else {
		printf("Could not allocate byte array!\n");
	}
	fclose(inFile);

	return 1;
}