#!/bin/bash

case $1 in
## Returns flags needed for macOS compilation (experimental)
flags) .build/checkouts/gir2swift/gir2swift-generation-driver.sh c-flags $PWD ;;
## Removes all generaed files
clean) .build/checkouts/gir2swift/gir2swift-generation-driver.sh remove-generated $PWD ;;
## Defaults to generation
*) .build/checkouts/gir2swift/gir2swift-generation-driver.sh generate $PWD ;;
esac
