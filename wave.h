//
// Created by wogob on 9/15/2024.
//
#ifndef STEG_WAVE_H
#define STEG_WAVE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mathutilities.h"

typedef struct RiffHeader {
    uint32_t ChunkID; // big end.
    uint32_t ChunkSize;
    uint32_t Format;
} RIFF_CHUNK;

typedef struct FmtHeader {
    uint32_t Subchunk1ID; // big end.
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
} FMT_CHUNK;

typedef struct DataChunk {
    uint32_t Subchunk2ID; // big end.
    uint32_t Subchunk2Size;
    uint8_t *byteArray; // pointer to start of data
    // data size is equal to subchunk2size
} DATA_CHUNK;

typedef struct WaveFile {
    RIFF_CHUNK RIFF;
    FMT_CHUNK  FMT;
    DATA_CHUNK DATA;
} WAV_FILE;


// in stereo WAVs, left channel and right channel alternate every other sample
// 16-bit sample: (24 17) < left (1e f3) < right
// 8-bit sample: (24) < left (1e) < right
// so just interpret the data differently based on FMT_CHUNK

void freeWAV(WAV_FILE * wav) {
    free(wav->DATA.byteArray);
}

void initRiffChunk(RIFF_CHUNK* chunk, uint32_t SubChunk2Size)
{
    chunk->ChunkID = 0x46464952; // "RIFF" in big-endian form
    chunk->ChunkSize = SubChunk2Size + 36;
    chunk->Format = 0x45564157; // "WAVE" in big-endian form
}

// Standard sample rates are 8000, 44100, 48000.
// Standard Bits/Sample are 8, 16
void initFmtChunk(FMT_CHUNK* chunk, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample)
{
    chunk->Subchunk1ID = 0x20746D66; // "fmt " in big-endian form
    chunk->Subchunk1Size = 16; // For PCM
    chunk->AudioFormat = 1; // We're using PCM
    chunk->NumChannels = numChannels; // 1: Mono, 2: Stereo
    chunk->SampleRate = sampleRate; // 8000, 44100, 48000, etc.
    chunk->ByteRate = sampleRate * numChannels * (bitsPerSample / 8);
    chunk->BlockAlign = numChannels * (bitsPerSample / 8); // num of bytes for one sample including all channels (2 * 16, etc.)
    chunk->BitsPerSample = bitsPerSample; // 8, 16 are standard
}
// data_size = NumSamples * NumChannels * BitsPerSample/8
void initDataChunk(DATA_CHUNK* chunk, uint32_t data_size)
{
    chunk->Subchunk2ID = 0x61746164; // "data" in big-endian form
    chunk->Subchunk2Size = data_size;
    chunk->byteArray = (int8_t*)malloc(data_size);
}

// initialize wave file data
// data_size = NumSamples * NumChannels * BitsPerSample/8
// Standard sample rates are 8000, 44100, 48000.
// Standard Bits/Sample are 8, 16
void initWavFile(WAV_FILE* wav, uint32_t data_size, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample)
{
    initRiffChunk(&wav->RIFF, data_size);
    initFmtChunk(&wav->FMT, numChannels, sampleRate, bitsPerSample);
    initDataChunk(&wav->DATA, data_size);
}


// write a byte to a wave file's data section, at an int offset.
int writeToWav8(WAV_FILE* wav, uint32_t offset, int8_t data)
{
    int success = 1;
    if(offset >= wav->DATA.Subchunk2Size)
    {
        // writing out of bounds
        printf("ERROR: Writing out of data bounds! Intended offset: %u / Data size: %u\n", offset, wav->DATA.Subchunk2Size);
        success = 0;
    } else if(wav->DATA.byteArray == NULL) {
        printf("ERROR: WAV data is NULL!\n");
        success = 0;
    } else {
        int8_t *data_ptr = (*wav).DATA.byteArray;
        data_ptr[offset] = data; // write data to byte
    }
    return success;
}



int writeToWav16(WAV_FILE* wav, uint32_t offset, int16_t data)
{
    int success = 1;
    if(offset >= wav->DATA.Subchunk2Size)
    {
        // writing out of bounds
        printf("ERROR: Writing out of data bounds! Intended offset: %u / Data size: %u\n", offset, wav->DATA.Subchunk2Size);
        success = 0;
    } else if(wav->DATA.byteArray == NULL) {
        printf("ERROR: WAV data is NULL!\n");
        success = 0;
    } else {
        // split int16 into two pieces, put into two bytes
        int8_t first = 0, second = 0;
        first = data & (0xFF);
        second = (data >> 8) & (0xFF);
        writeToWav8(wav, offset, first);
        writeToWav8(wav, offset + 1, second);
    }
    return success;
}

int writeToWav32(WAV_FILE* wav, uint32_t offset, int32_t data)
{
    int success = 1;
    if(offset >= wav->DATA.Subchunk2Size)
    {
        // writing out of bounds
        printf("ERROR: Writing out of data bounds! Intended offset: %u / Data size: %u\n", offset, wav->DATA.Subchunk2Size);
        success = 0;
    } else if(wav->DATA.byteArray == NULL) {
        printf("ERROR: WAV data is NULL!\n");
        success = 0;
    } else {
        // split int16 into two pieces, put into two bytes
        int8_t first = 0, second = 0;
        first = data & (0xFFFF);
        second = (data >> 16) & (0xFFFF);

        writeToWav16(wav, offset, first);
        writeToWav16(wav, offset + 2, second);
        // remember to increment by 2 if using this.
    }
    return success;
}

int writeSample(WAV_FILE* wav, uint32_t sampleCount, double sample) {
    uint32_t trueOffset = sampleCount * (wav->FMT.BitsPerSample / 8);

    switch(wav->FMT.BitsPerSample) {
        case 8:
            // fit sample within range 0 - 255
            uint8_t raster8 = (uint8_t)(clamp(sample, -128.0, 128.0) + 128.0);
            writeToWav8(wav, trueOffset, raster8);
            break;
        case 16:
            int16_t raster16 = (int16_t)(clamp(sample, -32767.0, 32767.0));
            writeToWav16(wav, trueOffset, raster16);
            break;
        case 32:
            int32_t raster32 = (int32_t)(clamp(sample, -2147483647, 2147483647));
            writeToWav32(wav, trueOffset, raster32);
            break;
        default:
            break;
    }
    return 0;
}

int writeSampleEncoded(WAV_FILE* wav, uint32_t sampleCount, double sample, int bitValue) {
    uint32_t trueOffset = sampleCount * (wav->FMT.BitsPerSample / 8);

    switch(wav->FMT.BitsPerSample) {
        case 8:
            // fit sample within range 0 - 255
            uint8_t raster8 = (uint8_t)(clamp(sample, -128.0, 128.0) + 128.0);
            raster8 = raster8 & 0xFE | bitValue;
            writeToWav8(wav, trueOffset, raster8);
            break;
        case 16:
            int16_t raster16 = (int16_t)(clamp(sample, -32767.0, 32767.0));
            raster16 = raster16 & 0xFE | bitValue;
            writeToWav16(wav, trueOffset, raster16);
            break;
        case 32:
            int32_t raster32 = (int32_t)(clamp(sample, -2147483647, 2147483647));
            raster32 = raster32 & 0xFE | bitValue;
            writeToWav32(wav, trueOffset, raster32);
            break;
        default:
            break;
    }
    return 0;
}

int writeToFile(FILE* outFile, WAV_FILE* wav)
{
    printf("Writing WAV to file...\n");
    if(outFile == NULL)
    {
        printf("ERROR: outFile is NULL!\n");
        return 0;
    }
    fwrite(&(wav->RIFF), sizeof(RIFF_CHUNK), 1, outFile); // write RIFF header
    fwrite(&(wav->FMT), sizeof(FMT_CHUNK), 1, outFile); // write FMT header
    fwrite(&(wav->DATA).Subchunk2ID, sizeof(uint32_t), 1, outFile); // write data ID
    fwrite(&(wav->DATA).Subchunk2Size, sizeof(uint32_t), 1, outFile); // write data header
    fwrite(wav->DATA.byteArray, (size_t)wav->DATA.Subchunk2Size, 1, outFile); // write all bytes of byte array
    return 1;
}

WAV_FILE* readFromFile(const char* path) {
    FILE* inFile;
    fopen_s(&inFile, path, "r+b");
    RIFF_CHUNK riff = { 0 };
    if(inFile == NULL) {
        printf("Error: Failed to open %s", path);
        fclose(inFile);
        return NULL;
    }
    // read riff chunk
    fread(&riff, sizeof(RIFF_CHUNK), 1, inFile);
    if(riff.ChunkID != 0x46464952 || riff.Format != 0x45564157) {
        printf("Invalid ChunkID or Format!\n");
        fclose(inFile);
        return NULL;
    }
    FMT_CHUNK fmt = { 0 };
    fread(&fmt, sizeof(FMT_CHUNK), 1, inFile);
    if(fmt.Subchunk1ID != 0x20746D66 || fmt.Subchunk1Size != 16) {
        printf("Error: Invalid FMT chunk or size! (files should be in PCM format.)\n");
        fclose(inFile);
        return NULL;
    }
    DATA_CHUNK data = { 0 };
    // get size of subchunk 2
    fread(&data.Subchunk2ID, sizeof(uint32_t), 1, inFile);
    if(data.Subchunk2ID != 0x61746164) {
        printf("Bad DATA chunk header!\n");
        fclose(inFile);
        return NULL;
    }
    fread(&data.Subchunk2Size, sizeof(uint32_t), 1, inFile);
    data.byteArray = (int8_t*)malloc(data.Subchunk2Size);
    // read waveform data from file
    fread(data.byteArray, data.Subchunk2Size, 1, inFile);
    if(data.byteArray == NULL) {
        printf("Error: No waveform data could be read!\n");
    }

    WAV_FILE* wave = (WAV_FILE*)malloc(sizeof(WAV_FILE));
    wave->RIFF = riff;
    wave->FMT = fmt;
    wave->DATA = data;

    return wave;
}
#endif //STEG_WAVE_H
// hidden secret...