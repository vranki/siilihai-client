TEMPLATE = lib
TARGET = siilihai-common

QT += core \
    gui \
    network \
    xml

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
    INCLUDEPATH += ../../../libsiilihai
}
win32:debug {
    LIBS += -L../../../libsiilihai/src/debug
    DEPENDPATH += ../../../libsiilihai/src/debug
}
win32:release {
    LIBS += -L../../../libsiilihai/src/release
    DEPENDPATH += ../../../libsiilihai/src/release
}

LIBS += -lsiilihai
