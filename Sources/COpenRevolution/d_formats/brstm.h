//OpenRevolution BRSTM reader
//Copyright (C) 2020 IC

unsigned char brstm_formats_read_brstm(Brstm* brstmi,const unsigned char* fileData,signed int debugLevel,uint8_t decodeAudio) {
    bool &BOM = brstmi->BOM;
    //Useless BRSTM specific variables
    //Header
    unsigned long file_size;
    unsigned int header_size;
    unsigned int num_chunks;
    unsigned long HEAD_offset;
    unsigned long HEAD_size;
    unsigned long ADPC_offset;
    unsigned long ADPC_size;
    unsigned long DATA_offset;
    unsigned long DATA_size;
    //HEAD
    unsigned long HEAD_length;
    unsigned long HEAD1_offset;
    unsigned long HEAD2_offset;
    unsigned long HEAD3_offset;
    //HEAD1
    unsigned long HEAD1_samples_per_ADPC;
    unsigned long HEAD1_bytes_per_ADPC;
    //HEAD2
    unsigned long HEAD2_track_info_offsets[8] = {0,0,0,0,0,0,0,0};
    //HEAD3
    unsigned long HEAD3_ch_info_offsets  [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned int  HEAD3_ch_gain          [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned int  HEAD3_ch_initial_scale [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      signed int  HEAD3_ch_hsample_1     [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      signed int  HEAD3_ch_hsample_2     [16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned int  HEAD3_ch_loop_ini_scale[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      signed int  HEAD3_ch_loop_hsample_1[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
      signed int  HEAD3_ch_loop_hsample_2[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //ADPC
    unsigned long ADPC_total_length;
    unsigned long ADPC_total_entries;
    //DATA
    unsigned long DATA_total_length;
    
    
    //Create references to our struct for the variables we'll care about later
    //This is kinda ugly but it makes the variable names in this reader nice and it works perfectly fine
    unsigned int  &HEAD1_codec = brstmi->codec;
    bool          &HEAD1_loop = brstmi->loop_flag;
    unsigned int  &HEAD1_num_channels = brstmi->num_channels;
    unsigned long &HEAD1_sample_rate = brstmi->sample_rate;
    unsigned long &HEAD1_loop_start = brstmi->loop_start;
    unsigned long &HEAD1_total_samples = brstmi->total_samples;
    unsigned long &HEAD1_ADPCM_offset = brstmi->audio_offset;
    unsigned long &HEAD1_total_blocks = brstmi->total_blocks;
    unsigned long &HEAD1_blocks_size = brstmi->blocks_size;
    unsigned long &HEAD1_blocks_samples = brstmi->blocks_samples;
    unsigned long &HEAD1_final_block_size = brstmi->final_block_size;
    unsigned long &HEAD1_final_block_samples = brstmi->final_block_samples;
    unsigned long &HEAD1_final_block_size_p = brstmi->final_block_size_p;
    
    unsigned int  &HEAD2_num_tracks = brstmi->num_tracks;
    unsigned int  &HEAD2_track_type = brstmi->track_desc_type;
    
    unsigned int (&HEAD2_track_num_channels)[8] = brstmi->track_num_channels;
    unsigned int (&HEAD2_track_lchannel_id) [8] = brstmi->track_lchannel_id;
    unsigned int (&HEAD2_track_rchannel_id) [8] = brstmi->track_rchannel_id;
    
    unsigned int (&HEAD2_track_volume)      [8] = brstmi->track_volume;
    unsigned int (&HEAD2_track_panning)     [8] = brstmi->track_panning;
    
    unsigned int  &HEAD3_num_channels = brstmi->num_channels;
    
    int16_t* (&PCM_samples)[16] = brstmi->PCM_samples;
    //int16_t* (&PCM_buffer)[16] = brstmi->PCM_buffer;
    
    unsigned char* (&ADPCM_data)  [16] = brstmi->ADPCM_data;
    //unsigned char* (&ADPCM_buffer)[16] = brstmi->ADPCM_buffer;
    int16_t  (&HEAD3_int16_adpcm) [16][16] = brstmi->ADPCM_coefs;
    int16_t* (&ADPC_hsamples_1)   [16] = brstmi->ADPCM_hsamples_1;
    int16_t* (&ADPC_hsamples_2)   [16] = brstmi->ADPCM_hsamples_2;
    
    //Check if the header matches RSTM
    char* magicstr=brstm_getSliceAsString(fileData,0,4);
    char  emagic1[5]="RSTM";
    char  emagic2[5]="HEAD";
    char  emagic3[5]="ADPC";
    char  emagic4[5]="DATA";
    if(strcmp(magicstr,emagic1) == 0) {
        //Byte Order Mark
        if(brstm_getSliceAsInt16Sample(fileData,0x04,1) == -257) {
            BOM = 1; //Big endian
        } else {
            BOM = 0; //Little endian
        }
        if(debugLevel>0) {std::cout << "Byte order: " << (BOM ? "Big endian" : "Little endian") << '\n';}
        //Start reading header
        file_size   = brstm_getSliceAsNumber(fileData,0x08,4,BOM);
        header_size = brstm_getSliceAsNumber(fileData,0x0C,2,BOM);
        num_chunks  = brstm_getSliceAsNumber(fileData,0x0E,2,BOM);
        HEAD_offset = brstm_getSliceAsNumber(fileData,0x10,4,BOM);
        HEAD_size   = brstm_getSliceAsNumber(fileData,0x14,4,BOM);
        ADPC_offset = brstm_getSliceAsNumber(fileData,0x18,4,BOM);
        ADPC_size   = brstm_getSliceAsNumber(fileData,0x1C,4,BOM);
        DATA_offset = brstm_getSliceAsNumber(fileData,0x20,4,BOM);
        DATA_size   = brstm_getSliceAsNumber(fileData,0x24,4,BOM);
        
        if(debugLevel>1) {std::cout << "File size: " << file_size << "\nHeader size: " << header_size << "\nChunks: " << num_chunks << "\nHEAD offset: " << HEAD_offset << "\nHEAD size: " << HEAD_size << "\nADPC offset: " << ADPC_offset << "\nADPC size: " << ADPC_size << "\nDATA offset: " << DATA_offset << "\nDATA size: " << DATA_size << "\n\n";}
        
        //HEAD
        magicstr=brstm_getSliceAsString(fileData,HEAD_offset,4);
        if(strcmp(magicstr,emagic2) == 0) {
            //Start reading HEAD
            HEAD_length  = brstm_getSliceAsNumber(fileData,HEAD_offset+0x04,4,BOM);
            HEAD1_offset = brstm_getSliceAsNumber(fileData,HEAD_offset+0x0C,4,BOM);
            HEAD2_offset = brstm_getSliceAsNumber(fileData,HEAD_offset+0x14,4,BOM);
            HEAD3_offset = brstm_getSliceAsNumber(fileData,HEAD_offset+0x1C,4,BOM);
            
            if(debugLevel>1) {std::cout << "HEAD length: " << HEAD_length << "\nHEAD1 offset: " << HEAD1_offset << "\nHEAD2 offset: " << HEAD2_offset << "\nHEAD3 offset: " << HEAD3_offset << "\n\n";}
            
            //HEAD1
            HEAD1_offset+=8;
            HEAD1_codec               = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x00,1,BOM);
            HEAD1_loop                = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x01,1,BOM);
            HEAD1_num_channels        = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x02,1,BOM);
            HEAD1_sample_rate         = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x04,2,BOM);
            HEAD1_loop_start          = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x08,4,BOM);
            HEAD1_total_samples       = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x0C,4,BOM);
            HEAD1_ADPCM_offset        = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x10,4,BOM);
            HEAD1_total_blocks        = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x14,4,BOM);
            HEAD1_blocks_size         = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x18,4,BOM);
            HEAD1_blocks_samples      = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x1C,4,BOM);
            HEAD1_final_block_size    = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x20,4,BOM);
            HEAD1_final_block_samples = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x24,4,BOM);
            HEAD1_final_block_size_p  = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x28,4,BOM);
            HEAD1_samples_per_ADPC    = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x2C,4,BOM);
            HEAD1_bytes_per_ADPC      = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD1_offset+0x30,4,BOM);
            HEAD1_offset-=8;
            
            if(HEAD1_codec > 2) { if(debugLevel>=0) {std::cout << "Unsupported codec.\n";} return 220; }
            
            if(debugLevel>0) {std::cout << "Codec: " << BRSTM_codecs_usr_str[HEAD1_codec] << "\nLoop: " << HEAD1_loop << "\nChannels: " << HEAD1_num_channels << "\nSample rate: " << HEAD1_sample_rate << "\nLoop start: " << HEAD1_loop_start << "\nTotal samples: " << HEAD1_total_samples << "\nOffset to ADPCM data: " << HEAD1_ADPCM_offset << "\nTotal blocks: " << HEAD1_total_blocks << "\nBlock size: " << HEAD1_blocks_size << "\nSamples per block: " << HEAD1_blocks_samples << "\nFinal block size: " << HEAD1_final_block_size << "\nFinal block samples: " << HEAD1_final_block_samples << "\nFinal block size with padding: " << HEAD1_final_block_size_p << "\nSamples per entry in ADPC: " << HEAD1_samples_per_ADPC << "\nBytes per entry in ADPC: " << HEAD1_bytes_per_ADPC << "\n\n";}
            
            //safety
            if(HEAD1_num_channels>16) { if(debugLevel>=0) {std::cout << "Too many channels, max supported is 16.\n";} return 249;}
            
            //HEAD2
            HEAD2_offset+=8;
            HEAD2_num_tracks = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_offset+0x00,1,BOM);
            HEAD2_track_type = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_offset+0x01,1,BOM);
            
            //safety
            if(HEAD2_num_tracks>8) { if(debugLevel>=0) {std::cout << "Too many tracks, max supported is 8.\n";} return 248;}
            
            //read info for each track
            for(unsigned char i=0;i<HEAD2_num_tracks;i++) {
                unsigned int readOffset = HEAD_offset+HEAD2_offset+0x04+4+(i*8);
                unsigned int infoOffset = brstm_getSliceAsNumber(fileData,readOffset,4,BOM);
                HEAD2_track_info_offsets[i]=infoOffset+8;
                if(HEAD2_track_type==0) {
                    HEAD2_track_num_channels[i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x00,1,BOM);
                    HEAD2_track_lchannel_id [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x01,1,BOM);
                    HEAD2_track_rchannel_id [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x02,1,BOM);
                } else if(HEAD2_track_type==1) {
                    HEAD2_track_num_channels[i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x08,1,BOM);
                    HEAD2_track_lchannel_id [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x09,1,BOM);
                    HEAD2_track_rchannel_id [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x0A,1,BOM);
                    //type 1 stuff
                    HEAD2_track_volume      [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x00,1,BOM);
                    HEAD2_track_panning     [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD2_track_info_offsets[i]+0x01,1,BOM);
                } else { if(debugLevel>=0) {std::cout << "Unknown track type.\n";} return 244;}
                HEAD2_track_info_offsets[i]-=8;
            }
            
            
            if(debugLevel>0) {std::cout << "Tracks: " << HEAD2_num_tracks << "\nTrack type: " << HEAD2_track_type << "\n";}
            if(debugLevel>1) {for(unsigned char i=0;i<HEAD2_num_tracks;i++) {
                std::cout << "\nTrack " << i+1 << "\nOffset: " << HEAD2_track_info_offsets[i] << "\nVolume: " << HEAD2_track_volume[i] << "\nPanning: " << HEAD2_track_panning[i] << "\nChannels: " << HEAD2_track_num_channels[i] << "\nLeft channel ID:  " << HEAD2_track_lchannel_id[i] << "\nRight channel ID: " << HEAD2_track_rchannel_id[i] << "\n\n";
            }}
            
            HEAD2_offset-=8;
            
            //HEAD3
            HEAD3_offset+=8;
            HEAD3_num_channels = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD3_offset+0x00,1,BOM);
            
            //safety
            if(HEAD3_num_channels>16) { if(debugLevel>=0) {std::cout << "Too many channels, max supported is 16.\n";} return 249;}
            
            for(unsigned char i=0;i<HEAD3_num_channels;i++) {
                unsigned int readOffset = HEAD_offset+HEAD3_offset+0x04+4+(i*8);
                unsigned int infoOffset = brstm_getSliceAsNumber(fileData,readOffset,4,BOM);
                HEAD3_ch_info_offsets[i]=infoOffset+8;
                //This information exists only in ADPCM files
                if(HEAD1_codec == 2) {
                    for(unsigned char x=0;x<32;x+=2) {
                        HEAD3_int16_adpcm[i][x/2] = brstm_getSliceAsInt16Sample(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x08+x,BOM);
                    }
                    HEAD3_ch_gain          [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x28,2,BOM);
                    HEAD3_ch_initial_scale [i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x2A,2,BOM);
                    HEAD3_ch_hsample_1     [i] = brstm_getSliceAsInt16Sample(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x2C,BOM);
                    HEAD3_ch_hsample_2     [i] = brstm_getSliceAsInt16Sample(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x2E,BOM);
                    HEAD3_ch_loop_ini_scale[i] = brstm_getSliceAsNumber(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x30,2,BOM);
                    HEAD3_ch_loop_hsample_1[i] = brstm_getSliceAsInt16Sample(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x32,BOM);
                    HEAD3_ch_loop_hsample_2[i] = brstm_getSliceAsInt16Sample(fileData,HEAD_offset+HEAD3_ch_info_offsets[i]+0x34,BOM);
                }
                HEAD3_ch_info_offsets[i]-=8;
            }
            
            if(debugLevel>0) {std::cout << "Channels: " << HEAD3_num_channels << "\n";}
            if(debugLevel>1) {for(unsigned char i=0;i<HEAD3_num_channels;i++) {
                std::cout
                << "\nChannel " << i+1
                << "\nOffset: " << HEAD3_ch_info_offsets[i];
                if(HEAD1_codec == 2) {
                    std::cout
                    << "\nGain: " << HEAD3_ch_gain[i]
                    << "\nInitial scale: " << HEAD3_ch_initial_scale[i]
                    << "\nHistory sample 1: " << HEAD3_ch_hsample_1[i]
                    << "\nHistory sample 2: " << HEAD3_ch_hsample_2[i]
                    << "\nLoop initial scale: " << HEAD3_ch_loop_ini_scale[i]
                    << "\nLoop history sample 1: " << HEAD3_ch_loop_hsample_1[i]
                    << "\nLoop history sample 2: " << HEAD3_ch_loop_hsample_2[i]
                    << "\nADPCM coefficients: ";
                    for(unsigned char x=0;x<16;x++) {
                        std::cout << HEAD3_int16_adpcm[i][x] << ' ';
                    }
                }
                std::cout << "\n\n";
            }}
            
            HEAD3_offset-=8;
            
            //Allocate hsamples
            for(unsigned int c=0;c<HEAD3_num_channels;c++) {
                ADPC_hsamples_1[c] = new int16_t[HEAD1_total_blocks];
                ADPC_hsamples_2[c] = new int16_t[HEAD1_total_blocks];
            }
            
            //ADPC chunk (Only for ADPCM codec)
            if(HEAD1_codec == 2) {
                magicstr=brstm_getSliceAsString(fileData,ADPC_offset,4);
                if(strcmp(magicstr,emagic3) == 0) {
                    //Start reading ADPC
                    ADPC_total_length  = brstm_getSliceAsNumber(fileData,ADPC_offset+0x04,4,BOM);
                    for(unsigned int n=0;n<HEAD3_num_channels;n++) {
                        if(debugLevel>1) {std::cout << "Channel " << n << ": ";}
                        
                        unsigned int it;
                        for(unsigned int i=0;i<HEAD1_total_blocks;i++) {
                            unsigned int offset=ADPC_offset+8+(n*HEAD1_bytes_per_ADPC)+((i*HEAD1_bytes_per_ADPC)*HEAD3_num_channels);
                            ADPC_hsamples_1[n][i] = brstm_getSliceAsInt16Sample(fileData,offset,BOM);
                            ADPC_hsamples_2[n][i] = brstm_getSliceAsInt16Sample(fileData,offset+2,BOM);
                            it=i+1;
                        }
                        if(debugLevel>1) {std::cout << it << " history sample pairs read.\n";}
                    }
                    if(debugLevel>1) {std::cout << "ADPC length: " << ADPC_total_length << "\nTotal entries: " << ADPC_total_entries << "\n\n";}
                } else { if(debugLevel>=0) {std::cout << "Invalid ADPC chunk.\n";} return 240;}
            }
            
            //Copy current state of history samples for the check after decoding (which overwrites history samples).
            int16_t* check_ADPC_hsamples_1[16];
            int16_t* check_ADPC_hsamples_2[16];
            if(debugLevel >= 1 && decodeAudio == 1 && HEAD1_codec == 2) {
                for(unsigned int c=0; c<HEAD3_num_channels; c++) {
                    check_ADPC_hsamples_1[c] = new int16_t[HEAD1_total_blocks];
                    check_ADPC_hsamples_2[c] = new int16_t[HEAD1_total_blocks];
                    for(unsigned int b=0; b<HEAD1_total_blocks; b++) {
                        check_ADPC_hsamples_1[c][b] = ADPC_hsamples_1[c][b];
                        check_ADPC_hsamples_2[c][b] = ADPC_hsamples_2[c][b];
                    }
                }
            }
            
            //DATA chunk
            magicstr=brstm_getSliceAsString(fileData,DATA_offset,4);
            if(strcmp(magicstr,emagic4) == 0) {
                //Start reading DATA
                DATA_total_length = brstm_getSliceAsNumber(fileData,DATA_offset+0x04,4,BOM);
                
                if(debugLevel>1) {std::cout << "DATA length: " << DATA_total_length << '\n';}
                
                if(decodeAudio) {
                    unsigned char awrite_res = brstm_doStandardAudioWrite(brstmi, fileData, debugLevel, decodeAudio);
                    if(awrite_res > 128) return awrite_res;
                }
                
                //History sample check when fully decoding ADPCM files.
                if(debugLevel >= 1 && decodeAudio == 1 && HEAD1_codec == 2) {
                    unsigned long hserrorcount = 0;
                    unsigned long hstotalcount = 0;
                    for(unsigned int c=0; c<HEAD3_num_channels; c++) {
                        for(unsigned int b=1; b<HEAD1_total_blocks; b++) {
                            if(
                                (check_ADPC_hsamples_1[c][b] < PCM_samples[c][b*HEAD1_blocks_samples - 1] - 256) ||
                                (check_ADPC_hsamples_1[c][b] > PCM_samples[c][b*HEAD1_blocks_samples - 1] + 256) ||
                                (check_ADPC_hsamples_2[c][b] < PCM_samples[c][b*HEAD1_blocks_samples - 2] - 256) ||
                                (check_ADPC_hsamples_2[c][b] > PCM_samples[c][b*HEAD1_blocks_samples - 2] + 256)
                            ) {hserrorcount++;}
                            
                            hstotalcount++;
                        }
                        delete[] check_ADPC_hsamples_1[c];
                        delete[] check_ADPC_hsamples_2[c];
                    }
                    
                    if(hserrorcount == 0) {
                        std::cout << "History samples seem to be correct.\n";
                    } else {
                        std::cout << hserrorcount << " out of " << hstotalcount << " history samples seem to be incorrect.\n";
                    }
                }
                
                //end
                
            } else { if(debugLevel>=0) {std::cout << "Invalid DATA chunk.\n";} return 230;}
            
        } else { if(debugLevel>=0) {std::cout << "Invalid HEAD chunk.\n";} return 250;}
        
    } else { if(debugLevel>=0) {std::cout << "Invalid BRSTM file.\n";} return 255;}
    
    return 0;
}
