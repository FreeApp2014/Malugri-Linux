//
// Created by thinker on 5/16/21.
//

import Foundation
import Gtk
import CGtk

func scaleValue(scale: Scale, value: Int) {
    let ump: UnsafeMutablePointer<GtkRange> = UnsafeMutablePointer<GtkRange>.allocate(capacity: 1);
    ump.pointee = scale.range;
    let range = Range(ump);
    range.set(value: Double(value));
}