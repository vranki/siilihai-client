#!/bin/bash
# Note: Doesn't work yet as webenginewidgets doesn't work on mingw.
MXEROOT=`pwd`/../mxe
pushd $MXEROOT
make MXE_TARGETS='x86_64-w64-mingw32.static' qt5
popd
qmake -recursive
make clean distclean
PATH=$MXEROOT/usr/bin:$PATH
$MXEROOT/usr/x86_64-w64-mingw32.static/qt5/bin/qmake -recursive
make

