TEMPLATE = lib
TARGET = siilihai-parsermaker
QMAKE_CXXFLAGS += -g -O0
CONFIG += debug
QT += core \
    gui \
    network \
    xml \
    sql

HEADERS += openrequestdialog.h \
    messagelistpatterneditor.h \
    grouplistpatterneditor.h \
    threadlistpatterneditor.h \
    patterneditor.h \
    downloaddialog.h \
    parsermaker.h
SOURCES += openrequestdialog.cpp \
    messagelistpatterneditor.cpp \
    grouplistpatterneditor.cpp \
    threadlistpatterneditor.cpp \
    patterneditor.cpp \
    downloaddialog.cpp \
    parsermaker.cpp
BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share

!contains(QMAKE_HOST.arch, x86_64) {
   LIBDIR = $$PREFIX/lib
} else {
   LIBDIR = $$PREFIX/lib64
}

target.path = $$LIBDIR
INSTALLS += target
FORMS = openrequestdialog.ui \
    patterneditor.ui \
    downloaddialog.ui \
    parsermaker.ui
RESOURCES = ../../siilihairesources.qrc

win32 {
    LIBS += -L../../../libsiilihai/src/debug
    LIBS += -L../common/debug
    INCLUDEPATH += ../../../libsiilihai
    DEPENDPATH += ../../../libsiilihai/src/debug
    DEPENDPATH += -L../common/debug
}

LIBS += -lsiilihai -lsiilihai-common
