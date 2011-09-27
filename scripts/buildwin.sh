#!/bin/bash

#Builds the windows version of Siilihai

#Assumes you have libsiilihai and siilihai-client in directory you run this

export QTDIR=$HOME/.wine/drive_c/QtSDK
export Path=C:\\windows\\system32\\\;C:\\windows\\\;C:\\windows\\system32\\wbem\;C:\\QtSDK\\mingw\\bin\\\;C:\\QtSDK\\Desktop\\Qt\\4.7.3\\mingw\\bin\\\;\
C:\\Program\ Files\\NSIS\\

export SH_BINARYPATH=release

function init_build {
rm -rf siilihai-win32
mkdir siilihai-win32
cd siilihai-client
cat debian/changelog | grep siilihai-client | head -n 1 | sed s/\).*/\"/ | sed s/siilihai-client\ \(/#define\ SIILIHAI_CLIENT_VERSION\ \"/ > siilihai-version.h
cd ..
}

function clean_all {
cd libsiilihai
ln -s src siilihai
qmake -recursive
make clean
make distclean
wineconsole --backend=curses qmake.exe -recursive "CONFIG+=release"
wineconsole --backend=curses mingw32-make.exe distclean
cd ..
cd siilihai-client
qmake -recursive
make clean
make distclean
wineconsole --backend=curses qmake.exe -recursive "CONFIG+=release"
wineconsole --backend=curses mingw32-make.exe distclean
cd ..
}

function build_lib {
cd libsiilihai
wineconsole --backend=curses qmake.exe -recursive "CONFIG+=release"
wineconsole --backend=curses mingw32-make.exe
cd ..
}

function install_lib {
cp libsiilihai/src/$SH_BINARYPATH/*.dll siilihai-win32
cd libsiilihai
cd ..
}

function build_app {
cd siilihai-client
wineconsole --backend=curses qmake.exe -recursive "CONFIG+=release"
wineconsole --backend=curses mingw32-make.exe
cd ..
}

function install_app {
cp siilihai-client/src/common/$SH_BINARYPATH/*.dll siilihai-win32
cp siilihai-client/src/parsermaker/$SH_BINARYPATH/*.dll siilihai-win32
cp siilihai-client/src/reader/$SH_BINARYPATH/*.exe siilihai-win32
}

function install_deps {
cp $QTDIR/Desktop/Qt/4.7.3/mingw/bin/mingwm10.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/bin/libgcc_s_dw2-1.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/QtCore4.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/QtGui4.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/QtNetwork4.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/QtWebKit4.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/QtXml4.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/QtXmlPatterns4.dll siilihai-win32
cp $QTDIR/Desktop/Qt/4.7.3/mingw/lib/phonon4.dll siilihai-win32
}

function create_installer {
cp siilihai-client/siilihai.nsi siilihai-win32
cd siilihai-win32
wine makensis.exe siilihai.nsi
}

init_build
clean_all
build_lib
install_lib
build_app
install_app
install_deps
create_installer
clean_all

