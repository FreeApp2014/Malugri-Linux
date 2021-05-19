//
// Created by thinker on 5/15/21.
//

import Foundation
import Gtk

extension Int {
    var hmsString: String {
        get {
            let hours = Int(Double(self) / 3600.0);
            let minutes = (self % 3600) / 60;
            let seconds = (self % 3600) % 60;
            return (hours != 0 ? String(hours) + ":" : "") + (minutes < 10 ? "0" + String(minutes) : String(minutes)) + ":" + (seconds < 10 ? "0" + String(seconds) : String(seconds))
        }
    }
}

class MalugriUtil {
    static func resolveAudioFormat(_ formatCode: UInt) -> String {
        switch (formatCode) {
        case 1: return "BRSTM";
        case 2: return "BCSTM";
        case 3: return "BFSTM";
        case 4: return "BWAV";
        case 5: return "ORSTM";
        case 6: return "BRWAV";
        case 7: return "BCWAV";
        case 8: return "BFWAV";
        default: return "Unknown format"
        }
    }
    static func resolveAudioCodec(_ codecCode: UInt) -> String {
        switch (codecCode){
        case 0: return "8bit PCM";
        case 1: return "16bit PCM";
        case 2: return "DSP-ADPCM";
        default: return "Unknown codec";
        }
    }
    static func popupAlert(window: ApplicationWindowRef, title: String, message: String){
        let alert = MessageDialog(parent: window, flags: .modal, type: .error, buttons: .ok, text: title, secondaryText: message);
        alert.run();
        Widget.destroy(alert)();
    }
    static let brstmReadErrorCode: [UInt8: String] = [
        0: "No error",
        255: "Invalid file",
        250: "Invalid file",
        249: "Too many channels in file",
        248: "Too many tracks in file",
        244: "Unknown track description type",
        240: "Invalid file",
        230: "Invalid file",
        220: "Audio codec not supported",
        210: "Invalid or unsupported file format."
    ]
}