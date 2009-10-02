TEMPLATE = app
TARGET = siilihai
QMAKE_CXXFLAGS += -g
QT += core \
    webkit \
    gui \
    network \
    xml \
    sql
HEADERS += groupsubscriptiondialog.h \
    subscribewizard.h \
    mainwindow.h \
    siilihai.h \
    loginwizard.h
SOURCES += groupsubscriptiondialog.cpp \
    subscribewizard.cpp \
    mainwindow.cpp \
    siilihai.cpp \
    loginwizard.cpp \
    main.cpp
BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
target.path = $$BINDIR
INSTALLS += target
FORMS = groupsubscriptiondialog.ui \
    subscribeforum_verify.ui \
    subscribeforum_login.ui \
    subscribeforum.ui \
    mainwindow.ui
RESOURCES = ../siilihairesources.qrc
INCLUDEPATH += /usr/include/siilihai

# DEPENDPATH += /usr/include/siilihai
LIBS += -lsiilihai
