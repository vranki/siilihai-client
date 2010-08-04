TEMPLATE = lib
TARGET = siilihai-common
CONFIG += debug
QT += core \
    gui \
    network \
    xml \
    sql

FORMS += credentialsdialog.ui
HEADERS += credentialsdialog.h
SOURCES += credentialsdialog.cpp

BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share

!contains(QMAKE_HOST.arch, x86_64) {
   LIBDIR = $$PREFIX/lib
} else {
   LIBDIR = $$PREFIX/lib64
}

target.path = $$LIBDIR
INSTALLS += target
RESOURCES += ../../siilihairesources.qrc

win32 {
    LIBS += -L../../../libsiilihai/src/debug
    INCLUDEPATH += ../../../libsiilihai
    DEPENDPATH += ../../../libsiilihai/src/debug
}

LIBS += -lsiilihai
