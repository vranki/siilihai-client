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
HEADERS += downloaddialog.h \
    parsermaker.h
SOURCES += downloaddialog.cpp \
    parsermaker.cpp \
    main.cpp
BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
target.path = $$BINDIR
INSTALLS += target
FORMS = downloaddialog.ui \
    parsermaker.ui
RESOURCES = ../../siilihairesources.qrc
INCLUDEPATH += /usr/include/siilihai

# DEPENDPATH += /usr/include/siilihai
LIBS += -lsiilihai
