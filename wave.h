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

// Delete WAV from memory
void freeWAV(WAV_FILE* wav);
// Initialize WAV RIFF header
void initRiffChunk(RIFF_CHUNK* chunk, uint32_t SubChunk2Size);
// Initialize WAV FMT header
void initFmtChunk(FMT_CHUNK* chunk, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample);
// Initialize WAV DATA section
void initDataChunk(DATA_CHUNK* chunk, uint32_t data_size);
// Initialize WAV File
void initWavFile(WAV_FILE* wav, uint32_t data_size, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample);
// Write data to wav (8-bit)
int writeToWav8(WAV_FILE* wav, uint32_t offset, int8_t data);
// Write data to wav (16-bit)
int writeToWav16(WAV_FILE* wav, uint32_t offset, int16_t data);
// Write data to wav (32-bit)
int writeToWav32(WAV_FILE* wav, uint32_t offset, int32_t data);
// Write sample to wav
int writeSample(WAV_FILE* wav, uint32_t sampleCount, double sample);
// Write sample encoded with steganographic data
int writeSampleEncoded(WAV_FILE* wav, uint32_t sampleCount, double sample, int bitValue);
// Write WAV to file
int writeToFile_WAV(FILE* outFile, WAV_FILE* wav);

// Encode steganographic data in WAV
int encodeToFile_WAV(const char* text, WAV_FILE* wav);
int encode_File_ToFile_WAV(FILE* input_file, WAV_FILE* wav);

// Read WAV from file
WAV_FILE* readFromFile_WAV(const char* path);
int decodeFromFile_WAV(const char* path);
int decode_toFile_FromFile_WAV(const char* path, const char* output_path);
#endif //STEG_WAVE_H
// hidden secret...