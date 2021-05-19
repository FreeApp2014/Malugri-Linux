//C++ BRSTM and other format reading tools
//Copyright (C) 2021 IC
//https://github.com/ic-scm/openrevolution

#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include <errno.h>

//Bool endian: 0 = little endian, 1 = big endian

const char* BRSTM_version_str = "v2.7.0-dev";
const char* brstm_getVersionString() {return BRSTM_version_str;}

//Format information
const unsigned int BRSTM_formats_count = 10;
//File header magic words for each file format
const char* BRSTM_formats_str[BRSTM_formats_count] = {"RIFF", "RSTM", "CSTM", "FSTM", "BWAV", "OSTM", "RWAV", "CWAV", "FWAV", "IDSP"};
//Offset to the audio offset information in each format (32 bit integer)
//(doesn't have to be accurate, just enough to fit the entire file header before it)
//Can be set to 0, a default offset (0x2000) will be used in that case.
const unsigned int BRSTM_formats_audio_off_off[BRSTM_formats_count] = {0x00, 0x70, 0x30, 0x30, 0x40, 0x10, 0x00, 0x24, 0x24, 0x28};
//Offset to the codec information and their sizes in each format
const unsigned int BRSTM_formats_codec_off  [BRSTM_formats_count] = {0x14, 0x60, 0x60, 0x60, 0x10, 0x00, 0x00, 0x48, 0x48, 0x04};
const unsigned int BRSTM_formats_codec_bytes[BRSTM_formats_count] = {1, 1, 1, 1, 2, 1, 1, 1, 1, 1};
//Flags for which formats have a standard byte order mark, getBaseInformation will use BRSTM_formats_default_endian if set to 0
const bool BRSTM_formats_has_bom[BRSTM_formats_count] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 0};
//Default byte order for formats (used in encoder)
const bool BRSTM_formats_default_endian[BRSTM_formats_count] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 1};
//Short human readable strings (equal to file extension)
const char* BRSTM_formats_short_usr_str[BRSTM_formats_count] = {"WAV", "BRSTM", "BCSTM", "BFSTM", "BWAV", "ORSTM", "BRWAV", "BCWAV", "BFWAV", "IDSP"};
//Long human readable strings
const char* BRSTM_formats_long_usr_str [BRSTM_formats_count] = {
"Waveform Audio",
"Binary Revolution Stream",
"Binary CTR Stream",
"Binary Cafe Stream",
"Nintendo BWAV",
"Open Revolution Stream",
"Binary Revolution WAV",
"Binary CTR WAV",
"Binary Cafe WAV",
"Interleaved DSP"
};

//Codec information
const unsigned int BRSTM_codecs_count = 4;
//Human readable strings
const char* BRSTM_codecs_usr_str[BRSTM_codecs_count] = {
"8-bit PCM",
"16-bit PCM",
"4-bit DSPADPCM",
"IMA ADPCM"
};


//The BRSTM struct
struct Brstm {
    //Byte order mark
    bool BOM = 0;
    //File type, 1 = BRSTM, see above for full list
    unsigned int  file_format   = 0;
    //Audio codec, 0 = PCM8, 1 = PCM16, 2 = DSPADPCM, see above for full list
    unsigned int  codec         = 0;
    bool          loop_flag     = 0;
    unsigned int  num_channels  = 0;
    unsigned long sample_rate   = 0;
    unsigned long loop_start    = 0;
    unsigned long total_samples = 0;
    unsigned long audio_offset  = 0;
    unsigned long total_blocks  = 0;
    unsigned long blocks_size   = 0;
    unsigned long blocks_samples  = 0;
    unsigned long final_block_size  = 0;
    unsigned long final_block_samples = 0;
    unsigned long final_block_size_p  = 0;
    
    //Track information
    unsigned int  num_tracks      = 0;
    unsigned int  track_desc_type = 0;
    unsigned int  track_num_channels[8] = {0,0,0,0,0,0,0,0};
    //TODO: Add support for more channels per track.
    unsigned int  track_lchannel_id [8] = {0,0,0,0,0,0,0,0};
    unsigned int  track_rchannel_id [8] = {0,0,0,0,0,0,0,0};
    unsigned int  track_volume      [8] = {0,0,0,0,0,0,0,0};
    unsigned int  track_panning     [8] = {0,0,0,0,0,0,0,0};
    
    int16_t* PCM_samples[16];
    int16_t* PCM_buffer [16];
    
    unsigned char* ADPCM_data   [16];
    unsigned char* ADPCM_buffer [16]; //Not used yet
    int16_t  ADPCM_coefs    [16][16];
    int16_t* ADPCM_hsamples_1   [16];
    int16_t* ADPCM_hsamples_2   [16];
    
    //Encoder
    unsigned char* encoded_file = nullptr;
    unsigned long  encoded_file_size = 0;
    
    //Things you probably shouldn't touch
    //Block cache
    int16_t* PCM_blockbuffer[16];
    unsigned long PCM_blockbuffer_currentBlock = -1;
    bool getbuffer_useBuffer = true;
    //Audio stream format,
    //0 for normal block data in BRSTM and similar files
    //1 for WAV which has 1 sample per block
    //so the block size here can be made bigger and block reads
    //won't be made one by one for every sample
    unsigned int audio_stream_format = 0;
    
    //BRSTM file fstream object used to internally pass the file stream in brstm_fstream functions.
    std::ifstream* file_ifstream;
    
    //Warning and error flags
    
    //Invalid file data warnings
    //Invalid track information, guessed track information
    bool warn_invalid_track_info = 0;
    //Invalid sample rate, used default sample rate
    bool warn_invalid_sample_rate = 0;
    //Invalid loop, removed loop point and flag
    bool warn_invalid_loop = 0;
    
    //Unsupported feature warnings
    //Unsupported file chunk
    bool warn_unsupported_chunk = 0;
    //Unsupported channels (more than 2 in a single track)
    bool warn_unsupported_channels = 0;
    
    //Other warnings
    //File doesn't include track information
    bool warn_guessed_track_info = 0;
    //File format does not support realtime decoding
    bool warn_realtime_decoding = 0;
};

#include "utils.h"
//This function is used by brstm_decode_block
unsigned char* brstm_getblock(Brstm* brstmi, const unsigned char* fileData, bool dataType, unsigned long start, unsigned long length);
//Internal fstream error handler for fstream functions.
unsigned char brstm_fstream_read_header_fstream_error_handler(std::ifstream& stream, signed int debugLevel);
#include "audio_decoder.h"
#include "d_formats/all.h"

//Get short file format string
const char* brstm_getShortFormatString(unsigned int format) {
    if(format >= BRSTM_formats_count) return "Unknown format";
    else return BRSTM_formats_short_usr_str[format];
}
//Get long file format string
const char* brstm_getLongFormatString(unsigned int format) {
    if(format >= BRSTM_formats_count) return "Unknown format";
    else return BRSTM_formats_long_usr_str[format];
}
//Get codec string
const char* brstm_getCodecString(unsigned int codec) {
    if(codec >= BRSTM_codecs_count) return "Unknown codec";
    else return BRSTM_codecs_usr_str[codec];
}

const char* brstm_getShortFormatString(Brstm* brstmi) { return brstm_getShortFormatString(brstmi->file_format); }
const char* brstm_getLongFormatString(Brstm* brstmi) { return brstm_getLongFormatString(brstmi->file_format); }
const char* brstm_getCodecString(Brstm* brstmi) { return brstm_getCodecString(brstmi->codec); }

//Get error string from code
const char* brstm_getErrorString(unsigned char code) {
    const char* invalidfile = "Invalid file";
    switch(code) {
        case 0: return "Success";
        
        //Warning codes
        //Common library warnings
        //(warn_unsupported_chunk, warn_unsupported_channels, warn_guessed_track_info, warn_realtime_decoding)
        case 1: return "Warning";
        //Invalid file data warnings, can also include the above warnings
        //(warn_invalid_track_info, warn_invalid_sample_rate, warn_invalid_loop)
        case 2: return "Invalid file data warning";
            
        //Errors
        case 255: return invalidfile;
        case 250: return invalidfile;
        case 249: return "Too many channels";
        case 248: return "Too many tracks";
        case 244: return "Invalid track information";
        case 240: return invalidfile;
        case 230: return invalidfile;
        case 222: return "Cannot write raw ADPCM data because the codec is not ADPCM";
        case 220: return "Unsupported audio codec";
        case 210: return "Unsupported file format or invalid file";
        case 206: return "Unsupported sample rate";
        case 205: return "Invalid encoder input data";
        case 185: return "File read error";
        case 184: return "Corrupted file (Unexpected end of file)";
        case 182: return "Corrupted file or not enough data was given";
        case 181: return "Invalid file handle";
        case 180: return "Not enough data was given";
    }
    return "Unknown error";
}

//Used by brstm_fstream_read, return standard codec number from the number in the file
unsigned int brstm_getStandardCodecNum(Brstm* brstmi,unsigned int num) {
    switch(brstmi->file_format) {
        case 0: {
            //WAV (PCM16 only)
            return 1;
        }
        case 1: {
            //BRSTM
            if(num < 3) {
                return num;
            }
            return -1;
        }
        case 2: case 3: case 7: case 8: {
            //BCSTM, BFSTM, BCWAV, BFWAV
            if(num < 4) {
                return num;
            }
            return -1;
        }
        case 4: {
            //BWAV
            if(num == 0) {
                return 1;
            } else if(num == 1) {
                return 2;
            }
            return -1;
        }
        case 5: {
            //ORSTM (doesn't exist)
            return -1;
        }
        case 9: {
            //IDSP (only DSPADPCM)
            return 2;
        }
    }
    return -1;
}

/* 
 * Read the BRSTM file headers and optionally decode the audio data.
 * 
 * brstmi: Your BRSTM struct
 * fileData: BRSTM file
 * debugLevel:
 *    -1 = Never output anything
 *     0 = Only output errors/warnings
 *     1 = Output information about the BRSTM
 *     2 = Output all information about the BRSTM (offsets, sizes, ADPCM information etc.)
 * decodeAudio:
 *     0 = Don't decode the audio data (Don't read the DATA chunk)
 *     1 = Decode audio data into PCM_samples
 *     2 = Write the raw ADPCM data into ADPCM_data
 * 
 * Returns error code (>127) or warning code (<128):
 *        0 = No error
 *      255 = Invalid BRSTM file (Doesn't begin with RSTM)
 *      250 = Invalid HEAD chunk (Doesn't begin with HEAD)
 *      249 = Too many channels
 *      248 = Too many tracks
 *      244 = Invalid track information
 *      240 = Invalid ADPC chunk (Doesn't begin with ADPC)
 *      230 = Invalid DATA chunk (Doesn't begin with DATA)
 *      222 = Cannot write raw ADPCM data because the codec is not ADPCM
 *      220 = Unsupported or unknown audio codec
 *      210 = Unsupported file format
 *      Errors other than listed here could be returned.
 *      See the brstm_getErrorString function for the full list of error codes.
 */
__attribute__((warn_unused_result))
unsigned char brstm_read(Brstm* brstmi,const unsigned char* fileData,signed int debugLevel,uint8_t decodeAudio) {
    //Initialize struct
    for(unsigned int c=0;c<16;c++) {
        brstmi->PCM_samples      [c] = nullptr;
        brstmi->PCM_buffer       [c] = nullptr;
        brstmi->ADPCM_data       [c] = nullptr;
        brstmi->ADPCM_buffer     [c] = nullptr;
        brstmi->ADPCM_hsamples_1 [c] = nullptr;
        brstmi->ADPCM_hsamples_2 [c] = nullptr;
        brstmi->PCM_blockbuffer  [c] = nullptr;
    }
    
    //Clear warnings
    brstmi->warn_invalid_track_info = 0;
    brstmi->warn_invalid_sample_rate = 0;
    brstmi->warn_invalid_loop = 0;
    brstmi->warn_unsupported_chunk = 0;
    brstmi->warn_unsupported_channels = 0;
    brstmi->warn_guessed_track_info = 0;
    brstmi->warn_realtime_decoding = 0;
    
    bool &BOM = brstmi->BOM;
    unsigned char readres = 0;
    
    //Find filetype
    brstmi->file_format = BRSTM_formats_count;
    for(unsigned int t=0;t<BRSTM_formats_count;t++) {
        if(strcmp(BRSTM_formats_str[t],brstm_getSliceAsString(fileData,0,strlen(BRSTM_formats_str[t]))) == 0) {
            brstmi->file_format = t;
            break;
        }
    }
    if(brstmi->file_format >= BRSTM_formats_count) {
        if(debugLevel>=0) {std::cout << "Invalid or unsupported file format.\n";}
        return 210;
    }
    
    if(debugLevel>0) std::cout << "File format: " << brstm_getShortFormatString(brstmi) << '\n';
    
    if(brstmi->file_format == 0) {
        //WAV
        readres = brstm_formats_read_wav  (brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 1) {
        //BRSTM
        readres = brstm_formats_read_brstm(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 2) {
        //BCSTM
        readres = brstm_formats_read_bcstm(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 3) {
        //BFSTM
        readres = brstm_formats_read_bfstm(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 4) {
        //BWAV
        readres = brstm_formats_read_bwav (brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 5) {
        //ORSTM
        readres = brstm_formats_read_orstm(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 6) {
        //BRWAV
        readres = brstm_formats_read_brwav(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 7) {
        //BCWAV
        readres = brstm_formats_read_bcwav(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 8) {
        //BFWAV
        readres = brstm_formats_read_bfwav(brstmi,fileData,debugLevel,decodeAudio);
    } else if(brstmi->file_format == 9) {
        //IDSP
        readres = brstm_formats_read_idsp (brstmi,fileData,debugLevel,decodeAudio);
    } else {
        if(debugLevel>=0) {std::cout << "Invalid or unsupported file format.\n";}
        return 210;
    }
    
    //Return now if a read error occurred
    if(readres>127) return readres;
    
    
    //Invalid data checks
    if(brstmi->sample_rate == 0) {
        if(debugLevel >= 0) std::cout << "Warning: This file has an invalid sample rate, defaulting to 44100Hz.\n";
        brstmi->sample_rate = 44100;
        brstmi->warn_invalid_sample_rate = 1;
    }
    
    if(brstmi->total_samples == 0) {
        if(debugLevel >= 0) std::cout << "Invalid sample count.\n";
        return 255;
    }
    
    if(brstmi->num_channels == 0) {
        if(debugLevel >= 0) std::cout << "Invalid channel count.\n";
        return 255;
    }
    
    if(brstmi->total_blocks == 0 ||
        (brstmi->total_blocks != 1 && (brstmi->blocks_size == 0 || brstmi->blocks_samples == 0)) ||
        (brstmi->total_blocks == 1 && (brstmi->final_block_size == 0 || brstmi->final_block_samples == 0 || brstmi->final_block_size_p < brstmi->final_block_size))
    ) {
        if(debugLevel >= 0) std::cout << "Invalid block information.\n";
        return 255;
    }
    
    //Check if track information is valid (and correct it if possible)
    uint8_t trackinfo_fail = 0; // 1 = Correction warnings, 2 = Errors
    if(brstmi->track_desc_type > 1) {
        trackinfo_fail = 2;
        //Set num tracks to 0 so the track information checking loop won't run.
        brstmi->num_tracks = 0;
    }
    if(brstmi->num_tracks == 0 || brstmi->num_tracks > 8) {
        trackinfo_fail = 2;
        brstmi->num_tracks = 0;
    }
    
    for(unsigned int t=0; t<brstmi->num_tracks; t++) {
        //Bad channel count
        if(brstmi->track_num_channels[t] < 1 || brstmi->track_num_channels[t] > 2) {
            trackinfo_fail = 2;
            break;
        }
        //Bad left channel ID
        if(brstmi->track_lchannel_id[t] >= brstmi->num_channels) {
            trackinfo_fail = 2;
            break;
        }
        //Bad right channel ID
        if(brstmi->track_num_channels[t] == 2 && brstmi->track_rchannel_id[t] >= brstmi->num_channels) {
            trackinfo_fail = 2;
            break;
        }
        
        switch(brstmi->track_desc_type) {
            case 0: {
                if(brstmi->track_volume[t] != 0 || brstmi->track_panning[t] != 0) {
                    trackinfo_fail = 1;
                    brstmi->track_volume[t] = 0;
                    brstmi->track_panning[t] = 0;
                }
                break;
            }
            case 1: {
                if(brstmi->track_volume[t] > 0x7F) {
                    trackinfo_fail = 1;
                    brstmi->track_volume[t] = 0x7F;
                }
                if(brstmi->track_panning[t] > 0x7F) {
                    trackinfo_fail = 1;
                    brstmi->track_panning[t] = 0x40;
                }
                break;
            }
        }
    }
    
    if(trackinfo_fail == 1) {
        if(debugLevel >= 0) std::cout << "Warning: This file has invalid track information and it was corrected.\n";
        brstmi->warn_invalid_track_info = 1;
    }
    else if(trackinfo_fail == 2) {
        if(debugLevel >= 0) std::cout << "Warning: This file has invalid track information, track information is guessed.\n";
        brstmi->warn_invalid_track_info = 1;
        
        //Erase current track information
        brstmi->num_tracks = 0;
        brstmi->track_desc_type = 0;
        for(unsigned char i=0; i<8; i++) {
            brstmi->track_num_channels[i] = 0;
            brstmi->track_lchannel_id [i] = 0;
            brstmi->track_rchannel_id [i] = 0;
            brstmi->track_volume      [i] = 0;
            brstmi->track_panning     [i] = 0;
        }
        
        //Guess standard track information
        brstmi->num_tracks = (brstmi->num_channels > 1 && brstmi->num_channels%2 == 0) ? brstmi->num_channels/2 : brstmi->num_channels;
        if(brstmi->num_tracks > 8) {
            if(debugLevel>=0) {std::cout << "Too many tracks, max supported is 8.\n";}
            return 248;
        }
        
        unsigned char track_num_channels = brstmi->num_tracks*2 == brstmi->num_channels ? 2 : 1;
        brstmi->track_desc_type = 0;
        
        for(unsigned char c=0; c<brstmi->num_channels; c++) {
            brstmi->track_num_channels[c/track_num_channels] = track_num_channels;
            if(track_num_channels == 1 || c%2 == 0) brstmi->track_lchannel_id[c/track_num_channels] = c;
            if(track_num_channels == 2 && c%2 == 1) brstmi->track_rchannel_id[c/track_num_channels] = c;
            else if(track_num_channels == 1) brstmi->track_rchannel_id[c/track_num_channels] = 0;
            //Erase any previously written volume and panning because this is type 0
            //brstmi->track_volume [c/track_num_channels] = 0;
            //brstmi->track_panning[c/track_num_channels] = 0;
        }
    }
    
    
    //Log details
    //BRSTM reader has it's own logger
    if(brstmi->file_format != 1) {
        if(debugLevel>0) {std::cout
            << "Byte order: " << (BOM == 1 ? "Big endian" : "Little endian")
            << "\nCodec: " << brstm_getCodecString(brstmi)
            << "\nLoop: " << brstmi->loop_flag
            << "\nChannels: " << brstmi->num_channels
            << "\nSample rate: " << brstmi->sample_rate
            << "\nLoop start: " << brstmi->loop_start
            << "\nTotal samples: " << brstmi->total_samples
            << "\nOffset to audio data: " << brstmi->audio_offset
            << "\nTotal blocks: " << brstmi->total_blocks
            << "\nBlock size: " << brstmi->blocks_size
            << "\nSamples per block: " << brstmi->blocks_samples
            << "\nFinal block size: " << brstmi->final_block_size
            << "\nFinal block samples: " << brstmi->final_block_samples
            << "\nFinal block size with padding: " << brstmi->final_block_size_p
            << "\n\n";
        }
        if(debugLevel>0) {std::cout
            << "Tracks: " << brstmi->num_tracks
            << "\nTrack type: " << brstmi->track_desc_type
            << "\n";
        }
        if(debugLevel>1) {
            for(unsigned char i=0;i<brstmi->num_tracks;i++) {
                std::cout
                << "\nTrack " << i+1
                << "\nVolume: " << brstmi->track_volume[i]
                << "\nPanning: " << brstmi->track_panning[i]
                << "\nChannels: " << brstmi->track_num_channels[i]
                << "\nLeft channel ID:  " << brstmi->track_lchannel_id[i]
                << "\nRight channel ID: " << brstmi->track_rchannel_id[i] << "\n\n";
            }
        }
        if(debugLevel>0) {std::cout 
            << "Channels: " << brstmi->num_channels
            << "\n";
        }
        if(debugLevel>1) {
            for(unsigned char i=0;i<brstmi->num_channels;i++) {
                std::cout
                << "\nChannel " << i+1
                << "\nADPCM coefficients: ";
                for(unsigned char x=0;x<16;x++) {
                    std::cout << brstmi->ADPCM_coefs[i][x] << ' ';
                }
                std::cout << "\n\n";
            }
        }
    }
    
    //Fail on unsupported codecs
    if(brstmi->codec > 2) {
        if(debugLevel >= 0) std::cout << brstm_getCodecString(brstmi) << " codec is not supported.\n";
        return 220;
    }
    
    //Check if loop point is valid
    if(brstmi->loop_start >= brstmi->total_samples) {
        brstmi->loop_start = 0;
        brstmi->loop_flag = 0;
        if(debugLevel >= 0) std::cout << "Warning: This file has an invalid loop point and it was removed.\n";
        brstmi->warn_invalid_loop = 1;
    }
    
    //Set return code
    readres = 0;
    if(brstmi->warn_unsupported_chunk || brstmi->warn_unsupported_channels || brstmi->warn_guessed_track_info || brstmi->warn_realtime_decoding) readres = 1;
    if(brstmi->warn_invalid_track_info || brstmi->warn_invalid_sample_rate || brstmi->warn_invalid_loop) readres = 2;
    
    return readres;
}

/* 
 * Get a buffer of audio data
 * 
 * brstmi: Your BRSTM struct
 * fileData: BRSTM file
 * sampleOffset: Offset to the first sample in the buffer
 * bufferSamples: Amount of samples in the buffer (don't make this more than the amount of samples per block!)
 * 
 */
void brstm_getbuffer(Brstm* brstmi, const unsigned char* fileData, unsigned long sampleOffset, unsigned int bufferSamples);

/*
 * Get a buffer of audio data (fstream mode)
 * 
 * brstmi: Your BRSTM struct
 * stream: std::ifstream with an open BRSTM file
 * sampleOffset: Offset to the first sample in the buffer
 * bufferSamples: Amount of samples in the buffer (don't make this more than the amount of samples per block!)
 * debugLevel (optional): Debug level for error output, like brstm_read
 * 
 * Can return an error code on file stream errors (>127).
 * 
 */
__attribute__((warn_unused_result))
unsigned char brstm_fstream_safe_getbuffer(Brstm* brstmi ,std::ifstream& stream, unsigned long sampleOffset, unsigned int bufferSamples, signed int debugLevel);
__attribute__((warn_unused_result))
unsigned char brstm_fstream_safe_getbuffer(Brstm* brstmi ,std::ifstream& stream, unsigned long sampleOffset, unsigned int bufferSamples);


/*
 * Main function for both memory modes
 * dataType will be 1 for disk streaming mode (fileData will be null) so brstm_getblock
 * will know to do disk streaming stuff instead of just getting a slice of fileData
 */
void brstm_getbuffer_main(Brstm* brstmi, const unsigned char* fileData, bool dataType, unsigned long sampleOffset, unsigned int bufferSamples) {
    //safety
    if(sampleOffset>brstmi->total_samples) {
        for(unsigned int c=0;c<brstmi->num_channels;c++) {
            delete[] brstmi->PCM_buffer[c];
            brstmi->PCM_buffer[c] = new int16_t[bufferSamples];
            for(unsigned int i=0;i<bufferSamples;i++) {
                brstmi->PCM_buffer[c][i] = 0;
            }
        }
        return;
    }
    //decode a new block if we don't have the current block in the blockbuffer cache
    if(brstmi->PCM_blockbuffer_currentBlock != sampleOffset/brstmi->blocks_samples) {
        //calculate block number
        unsigned long b=sampleOffset/brstmi->blocks_samples;
        //Run decoder
        brstm_decode_block(brstmi,b,fileData,dataType);
    }
    //create the requested buffer
    if(brstmi->getbuffer_useBuffer) {
        bool blockEndReached = false;
        unsigned int blockEndReachedAt = 0;
        for(unsigned int c=0;c<brstmi->num_channels;c++) {
            //Create new array of samples for the current channel
            delete[] brstmi->PCM_buffer[c];
            brstmi->PCM_buffer[c] = new int16_t[bufferSamples];
            //offset in current block
            unsigned long dataIndex = sampleOffset-brstmi->blocks_samples*(unsigned int)(sampleOffset/brstmi->blocks_samples);
            for(unsigned int p=0;p<bufferSamples;p++) {
                if(dataIndex+p >= brstmi->blocks_samples) {
                    blockEndReached=true;
                    blockEndReachedAt=p;
                    break;
                }
                brstmi->PCM_buffer[c][p] = brstmi->PCM_blockbuffer[c][dataIndex+p];
            }
        }
        while(blockEndReached) {
            blockEndReached = false;
            //safety, return if we are outside the last block
            if((sampleOffset+blockEndReachedAt)/brstmi->blocks_samples >= brstmi->total_blocks) return;
            //don't make a new buffer in PCM_buffer
            brstmi->getbuffer_useBuffer = false;
            brstm_getbuffer_main(brstmi,fileData,dataType,sampleOffset+blockEndReachedAt,0);
            brstmi->getbuffer_useBuffer = true;
            unsigned int newstart = blockEndReachedAt;
            for(unsigned int c=0;c<brstmi->num_channels;c++) {
                unsigned int dataIndex=0;
                for(unsigned int p=newstart;p<bufferSamples;p++) {
                    brstmi->PCM_buffer[c][p] = brstmi->PCM_blockbuffer[c][dataIndex++];
                    if(dataIndex >= brstmi->blocks_samples) {
                        blockEndReached = true;
                        blockEndReachedAt = ++p;
                        break;
                    }
                }
            }
        }
    }
}

void brstm_getbuffer(Brstm* brstmi, const unsigned char* fileData, unsigned long sampleOffset, unsigned int bufferSamples) {
    brstm_getbuffer_main(brstmi, fileData, 0, sampleOffset, bufferSamples);
}


__attribute__((warn_unused_result))
unsigned char brstm_fstream_safe_getbuffer(Brstm* brstmi ,std::ifstream& stream, unsigned long sampleOffset, unsigned int bufferSamples, signed int debugLevel) {
    if(!stream.is_open() || !stream.good()) return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    brstmi->file_ifstream = &stream;
    
    brstm_getbuffer_main(brstmi, nullptr, 1, sampleOffset, bufferSamples);
    
    if(!stream.good()) return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    
    return 0;
}

//No debugLevel argument
__attribute__((warn_unused_result))
unsigned char brstm_fstream_safe_getbuffer(Brstm* brstmi ,std::ifstream& stream, unsigned long sampleOffset, unsigned int bufferSamples) {
    return brstm_fstream_safe_getbuffer(brstmi, stream, sampleOffset, bufferSamples, 0);
}

//Old function without proper error handling, deprecated
__attribute__((deprecated("Use brstm_fstream_safe_getbuffer instead")))
void brstm_fstream_getbuffer(Brstm* brstmi, std::ifstream& stream, unsigned long sampleOffset, unsigned int bufferSamples) {
    if(!stream.is_open()) {perror("brstm_fstream_getbuffer: No file open in stream"); exit(255);}
    brstmi->file_ifstream = &stream;
    brstm_getbuffer_main(brstmi, nullptr, 1, sampleOffset, bufferSamples);
}


//This function is used by brstm_getbuffer/brstm_decode_block
unsigned char* brstm_getblock(Brstm* brstmi, const unsigned char* fileData, bool dataType, unsigned long start, unsigned long length) {
    if(dataType == 0) {
        return brstm_getSlice((const unsigned char*)fileData,start,length);
    } else {
        //disk streaming
        delete[] brstm_slice;
        brstm_slice = new unsigned char[length];
        
        brstmi->file_ifstream->seekg(start);
        
        brstmi->file_ifstream->read((char*)brstm_slice,length);
        return brstm_slice;
    }
}


//File error handler for below function.
unsigned char brstm_fstream_read_header_fstream_error_handler(std::ifstream& stream, signed int debugLevel) {
    if(!stream.is_open()) {
        if(debugLevel>=0) std::cout << "brstm_fstream_read: no file open in stream.\n";
        return 181;
    }
    if(stream.eof()) {
        if(debugLevel>=0) std::cout << "brstm_fstream_read: Unexpected end of file.\n";
        return 184;
    }
    if(stream.fail()) {
        if(debugLevel>=0) {
            std::cout << "brstm_fstream_read: " << (errno != 0 ? "File read error: " : "File handle error.") << (errno != 0 ? strerror(errno) : "") << "\n";
        }
        return 185;
    }
    
    if(debugLevel>=0) std::cout << "brstm_fstream_read: Unknown file handle error.\n";
    return 181;
}

//readAllData is 0 when called from brstm_fstream_getBaseInformation, 1 when called from brstm_fstream_read
__attribute__((warn_unused_result))
unsigned char brstm_fstream_read_header(Brstm * brstmi, std::ifstream& stream, signed int debugLevel, bool readAllData) {
    if(!stream.is_open() || !stream.good()) return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    
    bool &BOM = brstmi->BOM;
    unsigned char res = 0;
    unsigned char* brstm_header;
    brstmi->file_format = BRSTM_formats_count;
    
    //find file format so we can read the header size from the correct offset
    for(unsigned int t=0;t<BRSTM_formats_count;t++) {
        //read magic word from ifstream
        unsigned int emagiclen = strlen(BRSTM_formats_str[t]);
        char* magicword = new char[emagiclen+1];
        stream.seekg(0);
        stream.read(magicword,emagiclen);
        magicword[emagiclen] = '\0';
        //compare
        if(strcmp(BRSTM_formats_str[t],magicword) == 0) {
            brstmi->file_format = t;
            break;
        }
        delete[] magicword;
    }
    if(!stream.good())
        return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    if(brstmi->file_format >= BRSTM_formats_count) {
        if(debugLevel>=0) {std::cout << "Invalid or unsupported file format.\n";}
        return 210;
    }
    
    //Read Byte Order Mark
    if(BRSTM_formats_has_bom[brstmi->file_format]) {
        unsigned char bomword[2];
        stream.seekg(0x04);
        stream.read((char*)bomword,2);
        if(brstm_getSliceAsInt16Sample(bomword,0,1) == -257) {
            BOM = 1; //Big endian
        } else {
            BOM = 0; //Little endian
        }
    } else {
        BOM = BRSTM_formats_default_endian[brstmi->file_format];
    }
    
    if(!stream.good()) return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    
    //get offset to audio data so we know how much data to read to get the full header
    if(BRSTM_formats_audio_off_off[brstmi->file_format] != 0) {
        stream.seekg(BRSTM_formats_audio_off_off[brstmi->file_format]);
        unsigned char audioOff[4];
        stream.read((char*)audioOff,4);
        brstmi->audio_offset = brstm_getSliceAsNumber(audioOff,0,4,BOM) + 64;
        
        if(!stream.good()) return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    } else {
        //If the audio offset for this file in the library is set to 0 then use a default offset.
        brstmi->audio_offset = 8192;
    }
    
    //Get codec
    stream.seekg(BRSTM_formats_codec_off[brstmi->file_format]);
    unsigned char codecBytes[4];
    stream.read((char*)codecBytes,4);
    brstmi->codec = brstm_getStandardCodecNum(brstmi,
        brstm_getSliceAsNumber(codecBytes,0,BRSTM_formats_codec_bytes[brstmi->file_format],BOM)
    );
    
    stream.seekg(0);
    
    if(!stream.good()) return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
    
    if(readAllData) {
        if(debugLevel>1) std::cout << "Reading " << brstmi->audio_offset << " header bytes\n";
        
        //read header into memory
        brstm_header = new unsigned char[brstmi->audio_offset];
        stream.read((char*)brstm_header,brstmi->audio_offset);
        stream.seekg(0);
        
        if(!stream.good()) {
            delete[] brstm_header;
            return brstm_fstream_read_header_fstream_error_handler(stream, debugLevel);
        }
        
        //call main brstm read function
        res = brstm_read(brstmi,brstm_header,debugLevel,false);
        delete[] brstm_header;
    }
    
    return res;
}

/*
 * Read BRSTM directly from an ifstream
 * 
 * brstmi: Your BRSTM struct pointer
 * stream: std::ifstream with an open BRSTM file
 * debugLevel: console debug level, same as brstm_read
 */
__attribute__((warn_unused_result))
unsigned char brstm_fstream_read(Brstm * brstmi, std::ifstream& stream, signed int debugLevel) {
    return brstm_fstream_read_header(brstmi, stream, debugLevel, true);
}

/*
 * Get base file information (file format, codec, offset to audio) so you can decide how to read the file
 * This function is like brstm_fstream_read but it doesn't call the full brstm_read function
 */
__attribute__((warn_unused_result))
unsigned char brstm_fstream_getBaseInformation(Brstm * brstmi, std::ifstream& stream, signed int debugLevel) {
    return brstm_fstream_read_header(brstmi, stream, debugLevel, false);
}


/*
 * Get base file information like brstm_fstream_getBaseInformation, but from a part of a file loaded into memory.
 * 
 * brstmi: Your BRSTM struct pointer
 * data: File data
 * dataSize: Bytes in the data pointer
 * debugLevel: Console debug level, same as brstm_read
 */
__attribute__((warn_unused_result))
unsigned char brstm_getBaseInformation(Brstm* brstmi, unsigned char* data, unsigned long dataSize, signed int debugLevel) {
    bool &BOM = brstmi->BOM;
    
    //Check minimum required length of input data to get the file format
    unsigned int minlen = 0;
    unsigned int clen;
    
    for(unsigned int f=0; f<BRSTM_formats_count; f++) {
        unsigned int len = strlen(BRSTM_formats_str[f]);
        if(len > minlen) minlen = len;
    }
    
    if(dataSize < minlen) {
        if(debugLevel>=0) std::cout << "Not enough data was given.\n";
        return 180;
    }
    
    //Find file format
    brstmi->file_format = BRSTM_formats_count;
    const char* magicword;
    
    for(unsigned int f=0; f<BRSTM_formats_count; f++) {
        //Get string from input file
        magicword = brstm_getSliceAsString(data, 0, strlen(BRSTM_formats_str[f]));
        
        //Compare input string and expected string
        if(strcmp(BRSTM_formats_str[f],magicword) == 0) {
            brstmi->file_format = f;
            break;
        }
    }
    
    if(brstmi->file_format >= BRSTM_formats_count) {
        if(debugLevel>=0) {std::cout << "Invalid or unsupported file format.\n";}
        return 210;
    }
    
    //Check minimum required length of input data to read all the base information out of this format
    //Add 2 bytes already for byte order mark
    minlen += 2;
    
    clen = BRSTM_formats_audio_off_off[brstmi->file_format] + 4;
    if(clen > minlen) minlen = clen;
    
    clen = BRSTM_formats_codec_off[brstmi->file_format] + BRSTM_formats_codec_bytes[brstmi->file_format];
    if(clen > minlen) minlen = clen;
    
    if(dataSize < minlen) {
        if(debugLevel>=0) std::cout << "Not enough data was given.\n";
        return 180;
    }
    
    //Read Byte Order Mark
    if(BRSTM_formats_has_bom[brstmi->file_format]) {
        if(brstm_getSliceAsInt16Sample(data,4,1) == -257) {
            BOM = 1; //Big endian
        } else {
            BOM = 0; //Little endian
        }
    } else {
        BOM = BRSTM_formats_default_endian[brstmi->file_format];
    }
    
    //Read offset to audio data
    if(BRSTM_formats_audio_off_off[brstmi->file_format] != 0) {
        brstmi->audio_offset = brstm_getSliceAsNumber(data, BRSTM_formats_audio_off_off[brstmi->file_format], 4, BOM) + 64;
    } else {
        //If the audio offset for this file in the library is set to 0 then use a default offset.
        brstmi->audio_offset = 8192;
    }
    
    //Read codec number
    brstmi->codec = brstm_getStandardCodecNum(brstmi,
        brstm_getSliceAsNumber(data, BRSTM_formats_codec_off[brstmi->file_format], BRSTM_formats_codec_bytes[brstmi->file_format], BOM)
    );
    
    return 0;
}


/* 
 * Initialize all struct variables to default values
 */
void brstm_init(Brstm* brstmi) {
    for(unsigned char i=0;i<16;i++) {
        for(unsigned char j=0;j<16;j++) {
            brstmi->ADPCM_coefs[i][j] = 0;
        }
        brstmi->ADPCM_hsamples_1[i] = nullptr;
        brstmi->ADPCM_hsamples_2[i] = nullptr;
        brstmi->PCM_samples[i] = nullptr;
        brstmi->PCM_buffer[i] = nullptr;
        brstmi->PCM_blockbuffer[i] = nullptr;
        brstmi->ADPCM_data[i] = nullptr;
        brstmi->ADPCM_buffer[i] = nullptr;
    }
    
    brstmi->encoded_file = nullptr;
    
    brstmi->file_format = 0;
    brstmi->codec = 0;
    brstmi->loop_flag = 0;
    brstmi->num_channels = 0;
    brstmi->sample_rate = 0;
    brstmi->loop_start = 0;
    brstmi->total_samples = 0;
    brstmi->audio_offset = 0;
    brstmi->total_blocks = 0;
    brstmi->blocks_size = 0;
    brstmi->blocks_samples = 0;
    brstmi->final_block_size = 0;
    brstmi->final_block_samples = 0;
    brstmi->final_block_size_p = 0;
    
    brstmi->num_tracks = 0;
    brstmi->track_desc_type = 0;
    
    for(unsigned char i=0; i<8; i++) {
        brstmi->track_num_channels[i] = 0;
        brstmi->track_lchannel_id [i] = 0;
        brstmi->track_rchannel_id [i] = 0;
        brstmi->track_volume      [i] = 0;
        brstmi->track_panning     [i] = 0;
    }
    brstmi->PCM_blockbuffer_currentBlock = -1;
    brstmi->audio_stream_format = 0;
    
    brstmi->warn_invalid_track_info = 0;
    brstmi->warn_invalid_sample_rate = 0;
    brstmi->warn_invalid_loop = 0;
    brstmi->warn_unsupported_chunk = 0;
    brstmi->warn_unsupported_channels = 0;
    brstmi->warn_guessed_track_info = 0;
    brstmi->warn_realtime_decoding = 0;
}

/* 
 * Close the BRSTM file (reset variables and free memory)
 */
void brstm_close(Brstm* brstmi) {
    for(unsigned char i=0;i<16;i++) {
        delete[] brstmi->ADPCM_hsamples_1[i];
        delete[] brstmi->ADPCM_hsamples_2[i];
        delete[] brstmi->PCM_samples[i];
        delete[] brstmi->PCM_buffer[i];
        delete[] brstmi->PCM_blockbuffer[i];
        delete[] brstmi->ADPCM_data[i];
        delete[] brstmi->ADPCM_buffer[i];
    }
    
    delete[] brstmi->encoded_file;
    
    brstm_init(brstmi);
}
