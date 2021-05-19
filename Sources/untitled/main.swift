import Gtk
import Foundation

let pC = MalugriPlayer(using: MGRtAudioBackend());

let status = Application.run {
    let window = ApplicationWindowRef(application: $0);
    window.title = "Malugri";
    window.setDefaultSize(width: 500, height: 140);
    let box = Box(orientation: .vertical, spacing: 2);
    box.marginStart = 10;
    box.marginEnd = 10;
    box.marginTop = 10;
    box.marginBottom = 10;
    let button = FileChooserButton(title: "Choose file", action: .open);
    let filter = FileFilter();
    filter.name = "OpenRevolution Files";
    filter.add(pattern: "*.brstm");
    filter.add(pattern: "*.bfstm");
    filter.add(pattern: "*.bcstm");
    filter.add(pattern: "*.bwav");
    filter.add(pattern: "*.bcwav");
    filter.add(pattern: "*.bfwav");
    button.set(filter: filter);
    box.add(widget: button);
    let internalBox = Box(orientation: .horizontal, spacing: 2);
    let btn = Button(label: "Play");
    let pb = Scale(range: .horizontal, min: 0.0, max: 100.0, step: 1.0);
    pb.drawValue = false;
    internalBox.add(widget: btn);
    internalBox.packStart(child: pb, expand: true, fill: true, padding: 1);
    let loopSelector = ToggleButton(label: "Loop");
    internalBox.add(widget: loopSelector);
    box.add(widget: internalBox);
    let frame = Frame(label: "File information");
    let intGrid = Grid();
    intGrid.insertColumn(position: 0);
    intGrid.insertColumn(position: 1);
    intGrid.columnSpacing = 140;
    let ftLbl = LabelRef(str: "File Type: ");
    ftLbl.xalign = 0.0;
    ftLbl.marginStart = 5;
    let cdLbl = LabelRef(str: "Codec: ");
    cdLbl.xalign = 0.0;
    cdLbl.marginStart = 5;
    let lLbl = LabelRef(str: "Loop: ");
    lLbl.xalign = 0.0;
    lLbl.marginStart = 5;
    let lpLbl = LabelRef(str: "Loop Point: ");
    lpLbl.xalign = 0.0;
    lpLbl.marginStart = 5;
    let bsLbl = LabelRef(str: "Block Samples: ");
    bsLbl.xalign = 0.0;
    bsLbl.marginStart = 5;
    let tbLbl = LabelRef(str: "Total Blocks: ");
    tbLbl.xalign = 0.0;
    tbLbl.marginStart = 5;
    let tsLbl = LabelRef(str: "Total Samples: ");
    tsLbl.xalign = 0.0;
    tsLbl.marginStart = 5;
    intGrid.attach(child: ftLbl, left: 0, top: 0, width: 1, height: 1);
    intGrid.attach(child: bsLbl, left: 1, top: 0, width: 1, height: 1);
    intGrid.attach(child: cdLbl, left: 0, top: 1, width: 1, height: 1);
    intGrid.attach(child: lLbl, left: 0, top: 2, width: 1, height: 1);
    intGrid.attach(child: lpLbl, left: 0, top: 3, width: 1, height: 1);
    intGrid.attach(child: tbLbl, left: 1, top: 1, width: 1, height: 1);
    intGrid.attach(child: tsLbl, left: 1, top: 2, width: 1, height: 1);
    frame.add(widget: intGrid);
    frame.marginTop = 8;
    box.add(widget: frame);
    window.add(widget: box);
    window.showAll();
    btn.connect(signal: .clicked) { () -> () in
        if (pC.backend.state) {
            pC.backend.pause();
            btn.label = "Play";
        } else {
            pC.backend.resume();
            btn.label = "Pause";
        }
    }
    button.connect(signal: .fileSet, handler: {
        if (pC.backend.state) {
            pC.backend.stop();
        }
        if (pC.currentFile != nil) {
            pC.closeFile();
        }
       do {
           try pC.loadFile(file: button.filename!);
           ftLbl.label = "File Type: " + pC.fileInformation.fileType;
           cdLbl.label = "Codec: " +  pC.fileInformation.codecString;
           tbLbl.label = "Total Blocks: " + String(pC.fileInformation.totalBlocks);
           lLbl.label = "Loop: " + (pC.fileInformation.looping ? "Yes" : "No");
           lpLbl.label = "Loop Point: " + String(pC.fileInformation.loopPoint);
           tsLbl.label = "Total Samples: " + String(pC.fileInformation.totalSamples);
           bsLbl.label = "Block Samples: " + String(pC.fileInformation.blockSize);
           window.title = String(button.filename!.split(separator: "/").last!) + " - Malugri";
           btn.label = "Pause";
           DispatchQueue.global().async {
               while (pC.backend.state) {
                   scaleValue(scale: pb, value: Int(pC.backend.currentSampleNumber * 100 / pC.fileInformation.totalSamples))
                   Thread.sleep(forTimeInterval: 0.05)
               }
           }
           pC.backend.play();
       } catch MGError.brstmReadError(let code, let desc) {
           MalugriUtil.popupAlert(window: window, title: "Error opening file",
                   message: "brstm_read: " + desc + " (code " + String(code) + ")");
       } catch MGError.ifstreamError(let code) {
           MalugriUtil.popupAlert(window: window, title: "Error opening file",
                   message: "ifstream::open returned error code " + String(code))
       } catch {
           print(error);
           MalugriUtil.popupAlert(window: window, title: "Internal error",
                   message: "An unexpected error has occurred.")
       }
    });
}