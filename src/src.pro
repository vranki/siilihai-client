TEMPLATE = app
TARGET = siilihai
QT += core \
    webkit \
    gui
HEADERS += 
SOURCES += main.cpp
BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
INCLUDEDIR = $$PREFIX/include/siilihai
target.path = $$BINDIR
INSTALLS += target
