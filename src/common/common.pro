TEMPLATE = lib
TARGET = siilihai-common
QMAKE_CXXFLAGS += -g -O0
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
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
target.path = $$LIBDIR
INSTALLS += target
RESOURCES += ../../siilihairesources.qrc

win32 {
    LIBS += -L../../../libsiilihai/src/debug
    INCLUDEPATH += ../../../libsiilihai
    DEPENDPATH += ../../../libsiilihai/src/debug
}

LIBS += -lsiilihai
