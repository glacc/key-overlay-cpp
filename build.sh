#!/bin/bash

export OUTPUT="key-overlay-cpp"

export LIBS="glib-2.0 gio-2.0 sfml-graphics sfml-window sfml-system yaml-cpp"

export PKG_CONFIG=$(pkg-config --cflags --libs $LIBS)

export OPTIMIZATION_FLAGS_DEBUG="-O0 -g"
export OPTIMIZATION_FLAGS_RELEASE="-O2"

export OPTIMIZATION_FLAGS_DEFAULT=$OPTIMIZATION_FLAGS_RELEASE

case $1 in
    'debug')
        export OPTIMIZATION_FLAGS=$OPTIMIZATION_FLAGS_DEBUG
    ;;
    'release')
        export OPTIMIZATION_FLAGS=$OPTIMIZATION_FLAGS_RELEASE
    ;;
    *)
        export OPTIMIZATION_FLAGS=$OPTIMIZATION_FLAGS_DEFAULT
    ;;
esac

export CMD="g++ *.cpp $PKG_CONFIG -std=c++20 $OPTIMIZATION_FLAGS -o $OUTPUT"

echo "$CMD"

$CMD
