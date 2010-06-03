TEMPLATE = app
TARGET = siilihai
QMAKE_CXXFLAGS += -g \
    -O0
CONFIG += debug
QT += core \
    webkit \
    gui \
    network \
    xml \
    sql
HEADERS += messageviewwidget.h \
    messageformatting.h \
    threadlistwidget.h \
    settingsdialog.h \
    reportparser.h \
    forumlistwidget.h \
    favicon.h \
    groupsubscriptiondialog.h \
    subscribewizard.h \
    mainwindow.h \
    siilihai.h \
    loginwizard.h \
    forumproperties.h \
    threadproperties.h
SOURCES += messageviewwidget.cpp \
    messageformatting.cpp \
    threadlistwidget.cpp \
    settingsdialog.cpp \
    reportparser.cpp \
    forumlistwidget.cpp \
    favicon.cpp \
    groupsubscriptiondialog.cpp \
    subscribewizard.cpp \
    mainwindow.cpp \
    siilihai.cpp \
    loginwizard.cpp \
    main.cpp \
    forumproperties.cpp \
    threadproperties.cpp
DEPENDPATH = ../parsermaker \
    ../common
BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
target.path = $$BINDIR
INSTALLS += target
FORMS = settingsdialog.ui \
    reportparser.ui \
    groupsubscriptiondialog.ui \
    subscribeforum_verify.ui \
    subscribeforum_login.ui \
    subscribeforum.ui \
    mainwindow.ui \
    forumproperties.ui \
    threadproperties.ui
RESOURCES = ../../siilihairesources.qrc
LIBS += -L../parsermaker \
    -L../common \
    -lsiilihai \
    -lsiilihai-common
!exists(/etc/libosso):LIBS += -lsiilihai-parsermaker
