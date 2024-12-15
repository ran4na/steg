#include <stdio.h>
#include <math.h>
#include "wave.h"
#include <stdio.h>
#include "getopt.h"
#include <time.h>
#include "mathutilities.h"
#include "bmp.h"

enum FileTypes {
    TYPE_WAV,
    TYPE_BMP
};

#define PI 3.14159265358979323846
#define MAX_FILENAME_LENGTH 256

const uint32_t numSeconds = 10;
const uint16_t numChannels = 1;
const uint32_t sampleRate = 44100;
const uint32_t bitsPerSample = 16;

/*
 * TODO:
 * - add support for custom files to load and edit a WAV (i think?)
 * - BMP or PNG images
 * - embed images in other images
 * - maybe try diff algorithms
 */
void printUsage() {
    printf("Usage: ./steg.exe [-h] -t FILETYPE [-d] [-e TEXT] -f FILENAME\n");
    printf("\n\t-h\t\tShow usage\n");
    printf("\t-t FILETYPE\tFile type (wav, bmp)\n");
    printf("\t-d\t\tDecode mode\n");
    printf("\t-e TEXT\t\tEncode TEXT to file\n");
    printf("\t-f FILENAME\tinput/output filename\n");
}

int main(int argc, char* argv[]) {

    char* inpath = NULL;
    char* outpath = NULL;
    int opt;
    int mode = 0; // 0: encode, 1: decode
    int filetype = -1;
    // get clargs
    while(optind < argc) {
        if ((opt = getopt(argc, argv, "ht:d:e:f:")) != -1);
        switch(opt) {
            case 'h':
                printUsage();
                return 0;
                break;
            case 't':
            // file type
                if (strcmp(optarg, "bmp") == 0) {
                    filetype = TYPE_BMP;
                }
                else if (strcmp(optarg, "wav") == 0) {
                    filetype = TYPE_WAV;
                }
                break;
            case 'd':
                // decode mode
                mode = 1;
                if (optarg != NULL) inpath = optarg;
                printf("Output file path: %s\n", inpath);
                break;
            case 'e':
                // encode mode, get text.
                mode = 0;
                if(optarg != NULL) inpath = optarg;
                printf("Input file path: %s\n", inpath);
                break;
            case 'f':
                // Get file
                outpath = optarg;
                break;
            case ':':
                printf("Error: option not provided!\n");
                printUsage();
                return -1;
            case '?':
                printf("Error: invalid argument provided!\n");
                printUsage();
                return -1;
            default:
                break;
        }
    }
    uint32_t numBytes;
    if (outpath == NULL) {
        printf("No path provided.\n");
        printUsage();
        return -1;
    }

    if (filetype == -1) {
        printf("Invalid file type.\n");
        printUsage();
        return -1;
    }
    // Decode
    if(mode == 1) {
        if (filetype == TYPE_BMP) {
            //decodeFromFile_BMP(outpath);
            decode_ToFile_FromFile_BMP(outpath, inpath);
        }
        else if (filetype == TYPE_WAV) {
            //decodeFromFile_WAV(outpath);
            decode_toFile_FromFile_WAV(outpath, inpath);
        }
    } else if(inpath != NULL){
    // Encode
        FILE* input_file = fopen(inpath, "rb+");

        if (filetype == TYPE_WAV) {
            WAV_FILE* wavData = readFromFile_WAV(outpath);
            printf("|| Encoding...\n");
            
            //encodeToFile_WAV(text, wavData);
            encode_File_ToFile_WAV(input_file, wavData);
            printf("|| Saving...\n");
            FILE* output;
            char buffer[MAX_FILENAME_LENGTH];
            sprintf_s(buffer, MAX_FILENAME_LENGTH, "encoded_%s", outpath);
            output = fopen(buffer, "w+b");
            writeToFile_WAV(output, wavData);

            fclose(output);
            freeWAV(wavData);
        }
        else if (filetype == TYPE_BMP) {


            BMP_FILE* bmp = (BMP_FILE*) malloc(sizeof(BMP_FILE));
            //decodeFromFile_BMP("testbitmap_encoded.bmp");
            readBMPFromFile(outpath, bmp);

            //encodeToFile_BMP(bmp, text);
            encode_File_ToFile_BMP(bmp, input_file);
            char buffer[MAX_FILENAME_LENGTH];
            sprintf_s(buffer, MAX_FILENAME_LENGTH, "encoded_%s", outpath);
            printf("|| Writing file to %s\n", buffer);
            writeBmpToFile(buffer, bmp);
            freeBMP(bmp);
        }
        
    }
    

    return 0;
}
