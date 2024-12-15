#include "wave.h"
// in stereo WAVs, left channel and right channel alternate every other sample
// 16-bit sample: (24 17) < left (1e f3) < right
// 8-bit sample: (24) < left (1e) < right
// so just interpret the data differently based on FMT_CHUNK

void freeWAV(WAV_FILE* wav) {
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
    if (offset >= wav->DATA.Subchunk2Size)
    {
        // writing out of bounds
        printf("ERROR: Writing out of data bounds! Intended offset: %u / Data size: %u\n", offset, wav->DATA.Subchunk2Size);
        success = 0;
    }
    else if (wav->DATA.byteArray == NULL) {
        printf("ERROR: WAV data is NULL!\n");
        success = 0;
    }
    else {
        int8_t* data_ptr = (*wav).DATA.byteArray;
        data_ptr[offset] = data; // write data to byte
    }
    return success;
}



int writeToWav16(WAV_FILE* wav, uint32_t offset, int16_t data)
{
    int success = 1;
    if (offset >= wav->DATA.Subchunk2Size)
    {
        // writing out of bounds
        printf("ERROR: Writing out of data bounds! Intended offset: %u / Data size: %u\n", offset, wav->DATA.Subchunk2Size);
        success = 0;
    }
    else if (wav->DATA.byteArray == NULL) {
        printf("ERROR: WAV data is NULL!\n");
        success = 0;
    }
    else {
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
    if (offset >= wav->DATA.Subchunk2Size)
    {
        // writing out of bounds
        printf("ERROR: Writing out of data bounds! Intended offset: %u / Data size: %u\n", offset, wav->DATA.Subchunk2Size);
        success = 0;
    }
    else if (wav->DATA.byteArray == NULL) {
        printf("ERROR: WAV data is NULL!\n");
        success = 0;
    }
    else {
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
    uint8_t raster8;
    int16_t raster16;
    int32_t raster32;
    switch (wav->FMT.BitsPerSample) {
    case 8:
        // fit sample within range 0 - 255
        raster8 = (uint8_t)(clamp(sample, -128.0, 128.0) + 128.0);
        writeToWav8(wav, trueOffset, raster8);
        break;
    case 16:
        raster16 = (int16_t)(clamp(sample, -32767.0, 32767.0));
        writeToWav16(wav, trueOffset, raster16);
        break;
    case 32:
        raster32 = (int32_t)(clamp(sample, -2147483647, 2147483647));
        writeToWav32(wav, trueOffset, raster32);
        break;
    default:
        break;
    }
    return 0;
}

int writeSampleEncoded(WAV_FILE* wav, uint32_t sampleCount, double sample, int bitValue) {
    uint32_t trueOffset = sampleCount * (wav->FMT.BitsPerSample / 8);
    uint8_t raster8;
    int16_t raster16;
    int32_t raster32;
    switch (wav->FMT.BitsPerSample) {
    case 8:
        // fit sample within range 0 - 255
        raster8 = (uint8_t)(clamp(sample, -128.0, 128.0) + 128.0);
        raster8 = raster8 & 0xFE | bitValue;
        writeToWav8(wav, trueOffset, raster8);
        break;
    case 16:
        raster16 = (int16_t)(clamp(sample, -32767.0, 32767.0));
        raster16 = raster16 & 0xFE | bitValue;
        writeToWav16(wav, trueOffset, raster16);
        break;
    case 32:
        raster32 = (int32_t)(clamp(sample, -2147483647, 2147483647));
        raster32 = raster32 & 0xFE | bitValue;
        writeToWav32(wav, trueOffset, raster32);
        break;
    default:
        break;
    }
    return 0;
}

int writeToFile_WAV(FILE* outFile, WAV_FILE* wav)
{
    printf("Writing WAV to file...\n");
    if (outFile == NULL)
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


int encode_File_ToFile_WAV(FILE* input_file, WAV_FILE* wav)
{
    char curChar = '\0';
    uint8_t currentByte = 0;
    int charBitIndex = 0;
    int value = 0;
    int size_read = 0;
    int progress = 0;
    while (!feof(input_file)) {
        // read a character into the file
        size_read = fread(&curChar, 1, 1, input_file);
        printf("Encoding character: %c\n", curChar);

        for (int i = 0; i < 8; i++) {
            // get current sample
            currentByte = wav->DATA.byteArray[progress + i];
            
            // get i'th bit of string
            value = (curChar >> i) & 1;
            // encode into WAV
            //printf("Encoding bit: %d\n", value);

            writeSampleEncoded(wav, progress + i, currentByte, value);
        }
        progress += 8;
    }
    
    return 0;
}

int encodeToFile_WAV(const char* text, WAV_FILE* wav)
{
    if (text == NULL) return 0;
    int stringLengthBits = (strlen(text) + 1) * 8; // string length (including null)

    char curChar = '\0';
    uint8_t currentByte = 0;
    int charBitIndex = 0;
    int value = 0;
    // keep track of which bit we're on, and encode that into the sample
    for (int i = 0; i < stringLengthBits; i++) {
        // get current sample
        currentByte = wav->DATA.byteArray[i];
        // get i'th bit of string
        value = (text[i / 8] >> (i % 8)) & 1;
        // encode into WAV
        writeSampleEncoded(wav, i, currentByte, value);
    }
    return 0;
}

WAV_FILE* readFromFile_WAV(const char* path) {
    FILE* inFile;
    fopen_s(&inFile, path, "r+b");
    RIFF_CHUNK riff = { 0 };
    if (inFile == NULL) {
        printf("Error: Failed to open %s\n", path);
        return NULL;
    }
    // read riff chunk
    fread(&riff, sizeof(RIFF_CHUNK), 1, inFile);
    if (riff.ChunkID != 0x46464952 || riff.Format != 0x45564157) {
        printf("Invalid ChunkID or Format!\n");
        fclose(inFile);
        return NULL;
    }
    FMT_CHUNK fmt = { 0 };
    fread(&fmt, sizeof(FMT_CHUNK), 1, inFile);
    if (fmt.Subchunk1ID != 0x20746D66 || fmt.Subchunk1Size != 16) {
        printf("Error: Invalid FMT chunk or size! (files should be in PCM format.)\n");
        fclose(inFile);
        return NULL;
    }
    if (fmt.BitsPerSample != 8 && fmt.BitsPerSample != 16 && fmt.BitsPerSample != 32) {
        printf("Error: WAV files that aren't 8, 16, or 32-bit aren't supported.\n");
    }
    DATA_CHUNK data = { 0 };
    // get size of subchunk 2
    fread(&data.Subchunk2ID, sizeof(uint32_t), 1, inFile);
    if (data.Subchunk2ID != 0x61746164) {
        printf("Bad DATA chunk header!\n");
        fclose(inFile);
        return NULL;
    }
    fread(&data.Subchunk2Size, sizeof(uint32_t), 1, inFile);
    data.byteArray = (int8_t*)malloc(data.Subchunk2Size);
    // read waveform data from file
    if (data.byteArray == NULL) {
        printf("Could not allocate byte array memory!\n");
        return NULL;
    }
    fread(data.byteArray, data.Subchunk2Size, 1, inFile);
    if (data.byteArray == NULL) {
        printf("Error: No waveform data could be read!\n");
    }

    WAV_FILE* wave = (WAV_FILE*)malloc(sizeof(WAV_FILE));
    if (wave != NULL) {
        wave->RIFF = riff;
        wave->FMT = fmt;
        wave->DATA = data;
    }
    else {
        printf("Could not allocate memory for WAV file!\n");
        return NULL;
    }
    

    return wave;
}

int decodeFromFile_WAV(const char* path) {
    WAV_FILE* wavData = readFromFile_WAV(path);
    if (wavData == NULL) {
        printf("Could not read WAV file!\n");
        return -1;
    }
    uint32_t numBytes = wavData->DATA.Subchunk2Size;
    int bitsPerSample = wavData->FMT.BitsPerSample;
    char curChar = '\0';
    int charBitIndex = 0;
    uint8_t currentByte = 0;
    // read through each byte
    // if byte is the end of a sample, capture its value, and set that bit of the char
    // print the char
    int bytesPerSample = bitsPerSample / 8;
    int value = 0;
    for (int i = 0; i < numBytes; i += bytesPerSample) {
        // wav file is little endian, so read the first byte, then skip n bytes
        // where n is the bytes per sample, so bitspersample / 8
        currentByte = wavData->DATA.byteArray[i];
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
    printf("\nString printed!\n");
    return 0;
}

int decode_toFile_FromFile_WAV(const char* path, const char* output_path)
{

    FILE* output_file = fopen(output_path, "wb+");


    WAV_FILE* wavData = readFromFile_WAV(path);
    if (wavData == NULL) {
        printf("Could not read WAV file!\n");
        return -1;
    }
    uint32_t numBytes = wavData->DATA.Subchunk2Size;
    int bitsPerSample = wavData->FMT.BitsPerSample;
    char curChar = '\0';
    int charBitIndex = 0;
    uint8_t currentByte = 0;
    // read through each byte
    // if byte is the end of a sample, capture its value, and set that bit of the char
    // print the char
    int bytesPerSample = bitsPerSample / 8;
    int value = 0;
    for (int i = 0; i < numBytes; i += bytesPerSample) {
        // wav file is little endian, so read the first byte, then skip n bytes
        // where n is the bytes per sample, so bitspersample / 8
        currentByte = wavData->DATA.byteArray[i];
        value = currentByte & 1; // get LSB of byte
        curChar |= value << charBitIndex;
        charBitIndex++;
        if (charBitIndex == 8) {
            charBitIndex = 0;
            if (curChar == '\0') {
                //break;
            }
            else {
                //printf("%c", curChar);
                printf("Writing byte: %c\n", curChar);
                fwrite(&curChar, 1, 1, output_file);
            }
            curChar = '\0';
        }
    }
    fclose(output_file);
    printf("\nDecoded data written to %s!\n", output_path);
    return 0;
}
