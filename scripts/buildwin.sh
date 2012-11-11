#!/bin/bash

#Builds the windows version of Siilihai

#Assumes you have libsiilihai and siilihai-client in directory you run this

set -v 

export QTDIR=$HOME/.wine/drive_c/QtSDK
export QTVERSIONDIR=$QTDIR/Desktop/Qt/4.8.1/
export Path=C:\\windows\\system32\\\;C:\\windows\\\;C:\\windows\\system32\\wbem\;C:\\QtSDK\\mingw\\bin\\\;C:\\Program\ Files\\NSIS\\\;C:\\QtSDK\\Desktop\\Qt\\4.8.1\\mingw\\bin\\\;

export SH_BINARYPATH=release
export WINE_CMD="wine cmd /c"

function init_build {
echo *** Init build ***
rm -rf siilihai-win32
mkdir siilihai-win32
cd siilihai-client
cat debian/changelog | grep siilihai-client | head -n 1 | sed s/\).*/\"/ | sed s/siilihai-client\ \(/#define\ SIILIHAI_CLIENT_VERSION\ \"/ > siilihai-version.h
cd ..
}

function clean_all {
echo *** Clean all ***
cd libsiilihai
#ln -sf src ./siilihai
qmake -recursive
make clean
make distclean
$WINE_CMD qmake.exe -recursive CONFIG=release
$WINE_CMD mingw32-make.exe distclean
cd ..
cd siilihai-client
qmake -recursive
make clean
make distclean
$WINE_CMD qmake.exe -recursive CONFIG=release
$WINE_CMD mingw32-make.exe distclean
cd ..
}

function build_lib {
echo *** Build Lib ***
cd libsiilihai
$WINE_CMD qmake.exe -recursive CONFIG+=release
$WINE_CMD mingw32-make.exe
cd ..
}

function install_lib {
echo ***  Install lib ***
cp libsiilihai/src/$SH_BINARYPATH/*.dll siilihai-win32
}

function build_app {
echo *** Build app ***
cd siilihai-client
$WINE_CMD qmake.exe -recursive CONFIG+=release CONFIG+=with_lib
$WINE_CMD mingw32-make.exe
cd ..
}

function install_app {
echo *** Install app ***
cp -v siilihai-client/src/$SH_BINARYPATH/*.exe siilihai-win32
cp -v siilihai-client/data/*.ico siilihai-win32
}

function install_deps {
echo *** Install deps ***
cp $QTDIR/mingw/bin/mingwm10.dll siilihai-win32
cp $QTDIR/mingw/bin/libgcc_s_dw2-1.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/QtCore4.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/QtGui4.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/QtNetwork4.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/QtWebKit4.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/QtXml4.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/QtXmlPatterns4.dll siilihai-win32
cp $QTVERSIONDIR/mingw/lib/phonon4.dll siilihai-win32
}

function create_installer {
echo *** Create installer ***
cp siilihai-client/siilihai.nsi siilihai-win32
cd siilihai-win32
$WINE_CMD makensis.exe siilihai.nsi
cd ..
}

#wine cmd 
#exit 0

init_build
clean_all
build_app
install_app
install_deps
create_installer
clean_all

