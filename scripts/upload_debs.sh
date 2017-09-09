#!/bin/bash

# Uploads new version of Siilihai to Ubuntu ppa
# Assumes you have libsiilihai and siilihai-client in directory you run this
# TODO: This is quite silly way. Use git to get a clean working dir.

rm *.deb *.changes *.upload *.dsc *.tar.gz *.build

function clean_lib {
cd libsiilihai
qmake -recursive
make clean
make distclean
rm -rf debian/libsiilihai debian/libsiilihai-dbg
rm -rf src/debug src/release src/test/*.o
rm src/lib*.so.* Makefile* src/*.Debug src/*.Release
rm -rf *.pro.user.*
cd ..
}

function clean_app {
cd siilihai-client
qmake -recursive
make clean
make distclean
rm -rf debian/siilihai-client
rm -rf src/common/debug src/common/release
rm -rf src/parsermaker/debug src/parsermaker/release src/parsermaker/*.Release src/parsermaker/*.Debug
rm -rf src/reader/debug src/reader/release src/reader/*.xml src/reader/*.ini src/reader/*.Release src/reader/*.Debug
rm -rf src/*.o src/*.Debug src/*.Release
rm -rf parts prime stage *.pro.user.* *.snap
cd ..
}

function package_lib {
clean_lib
cd libsiilihai
debuild -S -sa -I.git
cd ..
clean_lib
}

function package_app {
clean_app
cd siilihai-client
debuild -S -sa -I.git
cd ..
clean_app
}

package_lib
package_app

dput ppa:ville-ranki/siilihai libsiilihai_*_source.changes
dput ppa:ville-ranki/siilihai siilihai-client_*_source.changes
