//C++ BRSTM reader
//Copyright (C) 2020 Extrasklep
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "brstm.h"
#include "brstm_encode.h"

// Header definitions

Brstm* brstmp;
Brstm* brstmpe;
std::ifstream brstmfile;

unsigned char brstm_read(Brstm* brstmi,const unsigned char* fileData,signed int debugLevel,uint8_t decodeAudio);
void brstm_getbuffer(Brstm * brstmi,const unsigned char* fileData,unsigned long sampleOffset,unsigned int bufferSamples);
unsigned char brstm_fstream_safe_getbuffer(Brstm * brstmi,std::ifstream& stream,unsigned long sampleOffset,unsigned int bufferSamples);
unsigned char brstm_fstream_read(Brstm * brstmi,std::ifstream& stream,signed int debugLevel);
void brstm_close(Brstm * brstmi);
unsigned char brstm_encode(Brstm* brstmi, signed int debugLevel, uint8_t encodeADPCM);
//Getters for outer world access

extern "C" void initStruct(){
    brstmp = new Brstm;
    for (unsigned int c = 0; c < 16; c++){
        brstmp->ADPCM_buffer[c] = nullptr;
        brstmp->ADPCM_data[c] = nullptr;
        brstmp->ADPCM_hsamples_1[c] = nullptr;
        brstmp->ADPCM_hsamples_2[c] = nullptr;
        brstmp->PCM_blockbuffer[c] = nullptr;
        brstmp->PCM_buffer[c] = nullptr;
        brstmp->PCM_samples[c] = nullptr;
    }
}

extern "C" unsigned long  gHEAD1_sample_rate(){
    return brstmp->sample_rate;
};
extern "C" unsigned long gHEAD1_loop_start(){
    return brstmp->loop_start;
}
extern "C" unsigned char readABrstm (const unsigned char* fileData, unsigned char debugLevel, bool decodeADPCM){
    return brstm_read(brstmp, fileData, debugLevel, decodeADPCM);
}
extern "C" unsigned char readFstreamBrstm(){
    return brstm_fstream_read(brstmp, brstmfile, 1);
}
extern "C" int16_t** gPCM_samples(){
    return brstmp->PCM_samples;
}
extern "C" unsigned int gnum_tracks() {
    return brstmp->num_tracks;
}
extern "C" unsigned int  gHEAD3_num_channels(int trackNumber){
    return brstmp->track_num_channels[trackNumber];
}
extern "C" unsigned int gLChId(int trackNumber){
    return brstmp->track_lchannel_id[trackNumber];
}
extern "C" unsigned int gRChId(int trackNumber){
    return brstmp->track_rchannel_id[trackNumber];
}
extern "C" unsigned long gHEAD1_blocks_samples(){
    return brstmp->blocks_samples;
}



extern "C" int16_t**  getBufferBlock(unsigned long sampleOffset){
    unsigned int readLength;
    if (sampleOffset/brstmp->blocks_samples < (brstmp->total_blocks)) readLength = brstmp->blocks_samples;
    else readLength = brstmp->final_block_size;
    unsigned char code = brstm_fstream_safe_getbuffer(brstmp, brstmfile, sampleOffset, readLength);
    return brstmp->PCM_buffer;
}

extern "C" int16_t** getbuffer(unsigned long offset, uint32_t frames) {
    unsigned char code = brstm_fstream_safe_getbuffer(brstmp, brstmfile, offset, frames);
    return brstmp->PCM_buffer;
}

extern "C" void closeBrstm(){
    brstm_close(brstmp);
    delete brstmp;
    brstmfile.close();
}
extern "C" unsigned long gHEAD1_total_samples(){
    return brstmp->total_samples;
}
extern "C" unsigned int gHEAD1_loop(){
    return brstmp->loop_flag;
}

extern "C" int createIFSTREAMObject(char* filename){
     brstmfile.open(filename);
     return brstmfile.is_open();
}
extern "C" unsigned long gHEAD1_total_blocks(){
    return brstmp->total_blocks;
}

extern "C" unsigned long gHEAD1_final_block_samples(){
    return brstmp->final_block_samples;
}

extern "C" unsigned int gFileType(){
    return brstmp->file_format;
}

extern "C" unsigned int gFileCodec() {
    return brstmp->codec;
}

extern "C" void setBrstmEncodeSettings(unsigned int fileType, unsigned int fileCodec, unsigned int num_channels, unsigned int num_tracks, unsigned int loopFlag, unsigned long loop_start, unsigned long sample_rate, unsigned long total_samples){
    brstmpe = new Brstm;
    for (unsigned int c = 0; c < 16; c++){
        brstmpe->ADPCM_buffer[c] = nullptr;
        brstmpe->ADPCM_data[c] = nullptr;
        brstmpe->ADPCM_hsamples_1[c] = nullptr;
        brstmpe->ADPCM_hsamples_2[c] = nullptr;
        brstmpe->PCM_blockbuffer[c] = nullptr;
        brstmpe->PCM_buffer[c] = nullptr;
        brstmpe->PCM_samples[c] = nullptr;
    }
    brstmpe->file_format   = fileType;
    brstmpe->codec         = fileCodec;
    brstmpe->loop_flag     = loopFlag;
    brstmpe->num_channels  = num_channels;
    brstmpe->sample_rate   = sample_rate;
    brstmpe->loop_start    = loop_start;
    brstmpe->total_samples = total_samples;
    brstmpe->num_tracks      = num_tracks;
    brstmpe->track_desc_type = 0;
    brstmpe->track_num_channels[0] = num_channels;
    brstmpe->track_lchannel_id[0] = 0;
    brstmpe->track_lchannel_id[1] = 1;
}

extern "C" void writeSamplesToChannel(int channel_id, int16_t* samples, unsigned long size){
    brstmpe->PCM_samples[channel_id] = new int16_t[size];
    memcpy(brstmpe->PCM_samples[channel_id], samples, size);
}

extern "C" unsigned char runEncoder(unsigned int encodeADPCM = 1){
    return brstm_encode(brstmpe, 0, encodeADPCM);
}

extern "C" unsigned char* getEncFile() {
    return brstmpe->encoded_file;
}

extern "C" unsigned long gEFileSize() {
    return brstmpe->encoded_file_size;
}

extern "C" void closeEbrstm() {
    brstm_close(brstmpe);
    delete brstmpe;
}