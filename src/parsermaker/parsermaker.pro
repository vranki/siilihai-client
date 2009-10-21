TEMPLATE = app
TARGET = siilihai-parsermaker
QMAKE_CXXFLAGS += -g \
    -O0
CONFIG += debug
QT += core \
    webkit \
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
    parsermaker.cpp \
    main.cpp
BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
target.path = $$BINDIR
INSTALLS += target
FORMS = openrequestdialog.ui \
    patterneditor.ui \
    downloaddialog.ui \
    parsermaker.ui
RESOURCES = ../../siilihairesources.qrc
INCLUDEPATH += /usr/include/siilihai

# DEPENDPATH += /usr/include/siilihai
LIBS += -lsiilihai
