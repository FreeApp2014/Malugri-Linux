// swift-tools-version:5.3
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "untitled",
        products: [
            .library(name: "COpenRevolution", targets: ["COpenRevolution"]),
            .library(name: "CRTAudio", targets: ["CRTAudio"]),
            .executable(name: "untitled", targets:["untitled"])
        ],
    dependencies: [
        // Dependencies declare other packages that this package depends on.
        // .package(url: /* package url */, from: "1.0.0"),
        .package(name: "gir2swift", url: "https://github.com/rhx/gir2swift.git", .branch("main")),
        .package(name: "Gtk", url: "https://github.com/rhx/SwiftGtk.git", .branch("gtk3")),
    ],
    targets: [
        // Targets are the basic building blocks of a package. A target can define a module or a test suite.
        // Targets can depend on other targets in this package, and on products in packages this package depends on.
        .target(name: "COpenRevolution", dependencies: []),
        .target(name: "CRTAudio",
                dependencies: [],
                linkerSettings: [
                    .linkedLibrary("rtaudio")
                ]),
        .target(
            name: "untitled",
            dependencies: ["Gtk", "COpenRevolution", "CRTAudio"]),
        .testTarget(
            name: "untitledTests",
            dependencies: ["untitled"]),
    ]
)
