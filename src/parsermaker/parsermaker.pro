TEMPLATE = lib
TARGET = siilihai-parsermaker

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

# Needed so that uic-generated headers for the forms in
# common work when shadow building
INCLUDEPATH += $$OUT_PWD/../common

win32 {
    INCLUDEPATH += ../../../libsiilihai
}
win32:debug {
    LIBS += -L../../../libsiilihai/src/debug
    LIBS += -L../common/debug
    DEPENDPATH += ../../../libsiilihai/src/debug
    DEPENDPATH += -L../common/debug
}
win32:release {
    LIBS += -L../../../libsiilihai/src/release
    LIBS += -L../common/release
    DEPENDPATH += ../../../libsiilihai/src/release
    DEPENDPATH += -L../common/release
}

LIBS += -lsiilihai -L../common -lsiilihai-common
