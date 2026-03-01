#!/bin/bash

export LIBS="glib-2.0 gio-2.0 sfml-graphics sfml-window sfml-system yaml-cpp"

export PKG_CONFIG=$(pkg-config --cflags --libs $LIBS)

export CMD="g++ *.cpp $PKG_CONFIG -std=c++20 -O0 -g -o key-overlay-cpp"

echo "$CMD"

$CMD
