#include <stdio.h>
#include <math.h>
#include "wave.h"
#include <stdio.h>
#include "getopt.h"
#include <time.h>
#include "mathutilities.h"

#define PI 3.14159265358979323846
const uint32_t numSeconds = 10;
const uint16_t numChannels = 1;
const uint32_t sampleRate = 44100;
const uint32_t bitsPerSample = 16;

int main(int argc, char* argv[]) {

    char* text = NULL;
    char* path = NULL;
    int opt;
    int mode = 0; // 0: encode, 1: decode
    // get clargs

    while((opt = getopt(argc, argv, "d:e:")) != -1) {
        switch(opt) {
            case 'd':
                // decode mode
                mode = 1;
                text = NULL;
                if(optarg != NULL) path = optarg;
                break;
            case 'e':
                mode = 0;
                if(optarg != NULL) text = optarg;
                // encode mode, get text.
                break;
            case ':':
                printf("Error: option not provided!\n");
                return 0;
            case '?':
                printf("Error: invalid argument provided!\n");
                return 0;
            default:
                break;
        }
    }
    WAV_FILE* wavData;
    uint32_t numBytes;
    if(mode == 1) {
        printf("Decoding...\n");
        wavData = readFromFile(path);
        // read out LSB from each sample
        numBytes = wavData->DATA.Subchunk2Size;
        int bitsPerSample = wavData->FMT.BitsPerSample;
        char curChar = '\0';
        int charBitIndex = 0;
        uint8_t currentByte = 0;
        // read through each byte
        // if byte is the end of a sample, capture its value, and set that bit of the char
        // print the char
        int bytesPerSample = bitsPerSample / 8;
        int value = 0;
        for(int i = 0; i < numBytes; i += bytesPerSample) {
            // wav file is little endian, so read the first byte, then skip n bytes
            // where n is the bytes per sample, so bitspersample / 8
            currentByte = wavData->DATA.byteArray[i];
            value = currentByte & 1; // get LSB of byte
            curChar |= value << charBitIndex;
            charBitIndex++;
            if(charBitIndex == 8) {
                charBitIndex = 0;
                if(curChar == '\0') {
                    break;
                } else {
                    printf("%c", curChar);
                }
                curChar = '\0';
            }
        }
        printf("\nString printed!\n");

    } else if(text != NULL){

        wavData = (WAV_FILE*)malloc(sizeof(WAV_FILE));
        numBytes = numSeconds * (numChannels * sampleRate * (bitsPerSample/8));
        initWavFile(wavData, numBytes, numChannels, sampleRate, bitsPerSample);

        printf("Encoding...\n");
        double sample = 64.0;
        double wave1 = 0, wave2 = 0, wave3 = 0;
        double freq = 1.0;
        double amplitude = INT16_MAX / 4.0;
        //writeToWav16(&testFile, numBytes - 1, (int16_t)1);
        double pitch1 = 261.626;
        double pitch2 = 120.665;
        double pitch3 = 329.628;
        uint32_t numSamples = numSeconds * numChannels * sampleRate;
        // encode text into each sample
        // string should be split across samples, so get length of string in bits
        int stringLength = strlen(text) * 8;
        int value;
        for(int i = 0; i < numSamples; i++)
        {

            double t = (double)(i * (bitsPerSample / 8)) / (double)sampleRate;
            double angle = (t) * (2.0 * PI);
            wave1 = sin(angle * (pitch1));
            wave2 = sin(angle * (pitch2));
            wave3 = sin(angle *  (pitch3));
            pitch1 = 36 * sin(angle * pitch3 / 500);
            pitch2 = 12 * cos(angle * 0.1);
            pitch3 = 64 * sin(angle * pitch2 / 500);
            sample = amplitude * (wave1 + wave2 + wave3);

            if(i < stringLength) {
                // get i'th bit of array
                value = (text[i/8] >> (i % 8)) & 1;
                // encode that bit into the sample
                writeSampleEncoded(wavData, i, sample, value);
            } else if(i >= stringLength && i < stringLength + 8) {
                // encode null terminator too
                writeSampleEncoded(wavData, i, sample, 0);
            }else {
                // write rest of file
                writeSample(wavData, i, sample);
            }
        }
        printf("Saving...\n");
        FILE* output;
        output = fopen("test.wav", "w+b");
        writeToFile(output, wavData);

        fclose(output);
        freeWAV(wavData);
    }


    return 0;
}
