#!/bin/bash

MXEROOT=../mxe/
PATH=$MXEROOT:$PATH

$MXEROOT/usr/i686-w64-mingw32.static/qt5/bin/qmake -recursive
make

