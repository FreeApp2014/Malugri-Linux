//
// Created by thinker on 5/15/21.
//

import Foundation
import CRTAudio
import COpenRevolution

class MGRtAudioBackend: MGAudioBackend {

    let dac: rtaudio_t = rtaudio_create(RTAUDIO_API_LINUX_PULSE);

    var currentSampleNumber: UInt {
        get {
            return counter;
        }
        set (x) {
            counter = x
        }
    }
    var currentTrack: UInt32 = 0

    private var sOptions = rtaudio_stream_options();
    private var sParameters = rtaudio_stream_parameters();


    private var needLoop = true;
    private var counter: UInt = 0;

    private var format: MGFileInformation = MGFileInformation(fileType: "", codecCode: 0, codecString: "", sampleRate: 0, looping: false, duration: 1, channelCount: 1, totalSamples: 1, loopPoint: 0, blockSize: 0, totalBlocks: 0, numTracks: 0);

    func initialize(format: MGFileInformation) {
        self.format = format;
        var a: (Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8, Int8) = (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        let str = "Malugri RtAudio Backend";
        withUnsafeMutableBytes(of: &a) { ptr in
            ptr.copyBytes(from: str.utf8.prefix(ptr.count))
        }
        sOptions.name = a;
        sParameters.device_id = rtaudio_get_default_output_device(dac);
        sParameters.num_channels = 2;
        sParameters.first_channel = 0;
        var sOBF: UInt32 = 256;

        let info = UnsafeMutablePointer<MGRtAudioBackend>(OpaquePointer(Unmanaged.passUnretained(self).toOpaque()));

        let rtAudioCB: rtaudio_cb_t = {
            (outputBuffer: UnsafeMutableRawPointer?, inputBuffer: UnsafeMutableRawPointer?, nBufferFrames: UInt32, streamTime: Double, status: rtaudio_stream_status_t,  userData: UnsafeMutableRawPointer?) -> Int32 in
            let mySelf = Unmanaged<MGRtAudioBackend>.fromOpaque(userData!).takeUnretainedValue()
            if (mySelf.counter >= gHEAD1_total_samples()) {
                if (mySelf.needLoop) {
                    mySelf.counter = gHEAD1_loop_start();
                } else {
                    //rtaudio_stop_stream(dac);
                    return 0;
                }
            }
            // Check whether the file has less samples than requested so that the loop can be seamless. Only applies to looping situation.
            if (nBufferFrames > gHEAD1_total_samples() - mySelf.counter && mySelf.needLoop){
                // Number of samples that can be read from the file before over-read
                let supportedNo = UInt32(gHEAD1_total_samples() - mySelf.counter);

                // Start with filling the buffer with the remainder of the file
                var samples = getbuffer(mySelf.counter, supportedNo);
                let audioBuffer: UnsafeMutablePointer<Int16> = outputBuffer!.assumingMemoryBound(to: Int16.self);
                var i = 0, j = 0;
                while (i < (supportedNo)*2){
                    audioBuffer[Int(i)] = samples![Int(pState.lChId)]![j];
                    audioBuffer[Int(i)+1] = samples![Int(pState.rChId)]![j]
                    i+=2;
                    j+=1;
                }

                // Reset the counter to the loop point
                mySelf.counter = gHEAD1_loop_start();

                // Fill the remainder of the buffer with new portion of the file
                samples = getbuffer(mySelf.counter, nBufferFrames - supportedNo);
                j = 0;
                while (i < nBufferFrames*2){
                    audioBuffer[Int(i)] = samples![Int(pState.lChId)]![j];
                    audioBuffer[Int(i)+1] = samples![Int(pState.rChId)]![j]
                    i+=2;
                    j+=1;
                }
                // Correctly increment the counter
                mySelf.counter += UInt(nBufferFrames - supportedNo);
            } else { // Play normally (gets called most of the time)
                let samples = getbuffer(mySelf.counter, nBufferFrames);
                let audioBuffer: UnsafeMutablePointer<Int16> = outputBuffer!.assumingMemoryBound(to: Int16.self);
                var i = 0, j = 0;
                while (i < nBufferFrames*2){
                    audioBuffer[Int(i)] = samples![Int(pState.lChId)]![j];
                    audioBuffer[Int(i)+1] = samples![Int(pState.rChId)]![j]
                    i+=2;
                    j+=1;
                }
                mySelf.counter += UInt(nBufferFrames);
            }
            return 0;
        }

        let errCB: rtaudio_error_cb_t =  {
            (e: rtaudio_error, a: Optional<UnsafePointer<Int8>>) in
                return;
        }

        rtaudio_open_stream(dac, &sParameters, nil, rtaudio_format_t(RTAUDIO_FORMAT_SINT16), UInt32(format.sampleRate), &sOBF, rtAudioCB, info, &sOptions, errCB)
    }

    func resume() {
        self.initialize(format: self.format);
        self.play();
    }

    func pause() {
        rtaudio_stop_stream(dac);
        rtaudio_close_stream(dac);
    }

    func stop() {
        rtaudio_stop_stream(dac);
        rtaudio_close_stream(dac);
        counter = 0;
    }

    var state: Bool {
        get {
            return (rtaudio_is_stream_running(dac) == 1 ? true : false);
        }
    }
    var needsLoop: Bool = false

    func play() {
        rtaudio_start_stream(dac);
    }

}

fileprivate struct CurrentPlayingBrstm {
    var chCount, lChId, rChId: UInt32;
}

fileprivate var pState = CurrentPlayingBrstm(chCount: gHEAD3_num_channels(0), lChId: gLChId(0), rChId: gHEAD3_num_channels(0) == 2 ? gRChId(0) : gLChId(0))