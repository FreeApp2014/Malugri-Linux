## brstm.h and brstm_encode.h usage
The BRSTM struct
```cpp
struct Brstm {
    //Byte order mark
    bool BOM;
    //File type, 0 = WAV, 1 = BRSTM, see brstm.h file for full list
    unsigned int  file_format;
    //Audio codec, 0 = PCM8, 1 = PCM16, 2 = DSPADPCM
    unsigned int  codec;
    bool          loop_flag;
    unsigned int  num_channels;
    unsigned long sample_rate;
    unsigned long loop_start;
    unsigned long total_samples;
    unsigned long audio_offset;
    unsigned long total_blocks;
    unsigned long blocks_size;
    unsigned long blocks_samples;
    unsigned long final_block_size;
    unsigned long final_block_samples;
    unsigned long final_block_size_p;
    
    //track information
    unsigned int  num_tracks;
    unsigned int  track_desc_type;
    unsigned int  track_num_channels[8];
    unsigned int  track_lchannel_id [8];
    unsigned int  track_rchannel_id [8];
    unsigned int  track_volume      [8];
    unsigned int  track_panning     [8];
    
    int16_t* PCM_samples[16];
    int16_t* PCM_buffer[16];
    
    unsigned char* ADPCM_data  [16];
    unsigned char* ADPCM_buffer[16]; //Not used yet
    int16_t  ADPCM_coefs [16][16];
    int16_t* ADPCM_hsamples_1   [16];
    int16_t* ADPCM_hsamples_2   [16];
    
    //Encoder
    unsigned char* encoded_file;
    unsigned long  encoded_file_size;
    
    //Things you probably shouldn't touch
    //block cache
    int16_t* PCM_blockbuffer[16];
    int PCM_blockbuffer_currentBlock = -1;
    bool getbuffer_useBuffer = true;
    unsigned int audio_stream_format = 0; //See full description in brstm.h
    
    //Read warning flags
    //Invalid track information, guessed track information
    bool warn_invalid_track_info = 0;
    //Invalid sample rate, used default sample rate
    bool warn_invalid_sample_rate = 0;
    //Invalid loop, removed loop point and flag
    bool warn_invalid_loop = 0;
    //Unsupported file chunk
    bool warn_unsupported_chunk = 0;
    //Unsupported channels (more than 2 in a single track)
    bool warn_unsupported_channels = 0;
    //File doesn't include track information
    bool warn_guessed_track_info = 0;
    //File format does not support realtime decoding
    bool warn_realtime_decoding = 0;
};
```

Include the files:
```cpp
#include "brstm.h"
```
If you want to use the encoder then include this too:
```cpp
#include "brstm_encode.h"
```
Create your BRSTM struct:
```cpp
Brstm* brstm = new Brstm;
```

Initialize the BRSTM struct (always do this after creating it):
```cpp
brstm_init(brstm);
```

Read a file:
```
brstm_read (

Your BRSTM struct pointer (Brstm*),

Raw file data (const unsigned char*),

Console debug level:
-1 = Never output anything
 0 = Only output errors/warnings
 1 = Output information about the BRSTM
 2 = Output all information about the BRSTM (offsets, sizes, ADPCM information etc.)
(signed int),

Decode audio data flag:
0 = Don't decode the audio data (Don't read the DATA chunk)
1 = Decode the audio data into PCM_samples[channel][sample offset]
2 = Write the raw ADPCM data into ADPCM_data
(unsigned char)

)
```
Returns error code (>127) or warning code (<128) (see brstm.h file for full list of error/warning codes). (unsigned char)

You can then read information about the BRSTM from your struct.

Decode a small part of the audio data into a buffer:
```
brstm_getbuffer (

Your BRSTM struct pointer (Brstm*),

Raw file data (const unsigned char*),

Sample offset (unsigned long),

Amount of samples in the buffer
(can't be bigger than the amount of samples per block (HEAD1_blocks_samples)!)
(unsigned int)

)
```
You can then read the requested buffer from PCM_buffer[channel][sample offset (from the sample offset passed to brstm_getbuffer)].

### Streaming
Read a file:
```
brstm_fstream_read (

Your BRSTM struct pointer (Brstm*),

std::ifstream with an open BRSTM file (std::ifstream&),

Console debug level (same as brstm_read) (signed int)

)
```
Returns error/warning codes like brstm_read. (unsigned char)

You can then read information about the BRSTM just like with brstm_read.

Get buffer:
```
brstm_fstream_safe_getbuffer (

Your BRSTM struct pointer (Brstm*),

std::ifstream with an open BRSTM file (std::ifstream&),

Sample offset (unsigned long),

Amount of samples in the buffer
(can't be bigger than the amount of samples per block (HEAD1_blocks_samples)!)
(unsigned int)

Debug level for error output (same as brstm_read) (signed int)

)
```
Can return an error code for file stream errors. (unsigned char)

You can then read the requested buffer from PCM_buffer[channel][sample offset (from the sample offset passed to the function)].

Remember to close the file (free any buffers and allocated memory) with brstm_close(Brstm*) before deleting your struct!
```cpp
brstm_close(brstm);
delete brstm;
```

### Other functions
```cpp
unsigned char brstm_fstream_getBaseInformation(Brstm*,std::ifstream&,signed int debugLevel)
```
Get base file information (file format, codec, offset to audio) so you can decide how to read the file

Similar to brstm_fstream_read but it doesn't call the full brstm_read function


```cpp
unsigned char brstm_getBaseInformation(Brstm*, unsigned char* data, unsigned long dataSize ,signed int debugLevel)
```
Get base file information (like brstm_fstream_getBaseInformation), but using a part of a file from memory instead.

data: Pointer to the beginning of file data

dataSize: Bytes of available data in the data pointer


```cpp
const char* brstm_getErrorString(unsigned char code)
```
Returns error string from error code


```cpp
const char* brstm_getShortFormatString(Brstm*)
const char* brstm_getShortFormatString(unsigned int format)
```
Returns short file format string


```cpp
const char* brstm_getLongFormatString(Brstm*)
const char* brstm_getLongFormatString(unsigned int format)
```
Returns long file format string


```cpp
const char* brstm_getCodecString(Brstm*)
const char* brstm_getCodecString(unsigned int codec)
```
Returns codec string


```cpp
const char* brstm_getVersionString()
```
Returns library version string


### Encoder

Create a BRSTM struct like with reading (or even read into that struct to reencode/rebuild a file)

Write your Int16 PCM audio data to PCM_samples[c] for each channel

You can also write raw DSPADPCM data to ADPCM_data[c] and the coefs to ADPCM_coefs[c]

Set audio and BRSTM details:
```cpp
file_format   = Encoded file format
codec         = BRSTM codec
loop_flag     = 0 or 1, loop flag
num_channels  = Number of audio channels
sample_rate   = Audio sample rate
loop_start    = Loop start point (sample number in a single channel)
total_samples = Total samples (in a single channel)

/*/--- TRACKS ---/*/

num_tracks      = Number of tracks
track_desc_type = Track description type
//You can set this to 0 or 1 (1 can store volume and panning information, 0 doesn't)
//See the WiiBrew page about BRSTM files for more information.

track_num_channels[track] = Number of channels in the track. (1 or 2)
track_lchannel_id [track] = BRSTM Channel ID for the left channel of this track.
track_rchannel_id [track] = BRSTM Channel ID for the right channel of this track.
//Set to 0 if this is a mono track. It's recommended to always set the channel IDs in order
//(from 0) because some decoders probably don't care about this information and all
//BRSTM files are like that.

track_volume [track] = Volume of the track. (0x00 to 0x7F)
track_panning[track] = Left to right panning of the track. (0x00 to 0x7F)
//Description type 1 only.
//Most decoders/players (including this one) probably don't care about this information.

// Example - standard stereo BRSTM

Brstm* brstm = new Brstm;
brstm_init(brstm);

brstm->file_format   = 1;
brstm->codec         = 2;
brstm->loop_flag     = 1;
brstm->num_channels  = 2;
brstm->sample_rate   = 44100;
brstm->loop_start    = 41234;
brstm->total_samples = 500000;

brstm->PCM_samples[0] = new int16_t[500000];
brstm->PCM_samples[1] = new int16_t[500000];

brstm->num_tracks      = 1;
brstm->track_desc_type = 1;

brstm->track_num_channels[0] = 2;
brstm->track_lchannel_id [0] = 0;
brstm->track_rchannel_id [0] = 1;

brstm->track_volume      [0] = 0x7F;
brstm->track_panning     [0] = 0x40;

brstm_encode(/*

Your BRSTM struct pointer (Brstm*)

Console debug level:
-1 = Never output anything
 0 = Only output errors/warnings
 1 = Log encoding progress
(signed int),

Encode ADPCM flag:
 0 = Use the ADPCM data from ADPCM_data (requires coefs in ADPCM_coefs[ch][coef])
 1 = Use PCM_samples (and encode into ADPCM)
 
(Optional) Byte order for the output file:
 0 = Little endian
 1 = Big endian
If not used, the default for the chosen file format will be used. 

Returns error code (>127) or warning code (<128)
(see brstm_encode.h file for full list of error/warning codes). (unsigned char)

*/);

file.write(brstm->encoded_file,brstm->encoded_file_size);

brstm_close(brstm); //This will delete the encoded data and PCM_samples
delete brstm;

```
