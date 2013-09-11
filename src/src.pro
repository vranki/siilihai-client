SUBDIRS += reader

TEMPLATE = app
TARGET = siilihai-client
target.path = /usr/bin
INSTALLS += target

ICON = ../data/siilihai.icns

exists(../siilihai-version.h) {
     DEFINES += INCLUDE_SIILIHAI_VERSION
}

CONFIG(debug_info) {
    message(config option debug_info - enabling some extra stuff)
    DEFINES += DEBUG_INFO
}

QT += core webkitwidgets xml widgets network

# DON't strip - let dpkg do it and create a dbg package
unix {
    QMAKE_STRIP = echo
}

contains(MEEGO_EDITION,harmattan): CONFIG += with_lib

android: CONFIG += with_lib

# Use this config flag to build libsiilihai into the binary
CONFIG(with_lib) {
    LIB_PATH = ../../libsiilihai
    !exists("$$LIB_PATH/src") {
       LIB_PATH = libsiilihai
    }
    SOURCES += $$LIB_PATH/src/siilihai/*.cpp
    SOURCES += $$LIB_PATH/src/siilihai/parser/*.cpp
    SOURCES += $$LIB_PATH/src/siilihai/tapatalk/*.cpp
    SOURCES += $$LIB_PATH/src/siilihai/forumdata/*.cpp
    SOURCES += $$LIB_PATH/src/siilihai/forumdatabase/*.cpp
    HEADERS += $$LIB_PATH/src/siilihai/*.h
    HEADERS += $$LIB_PATH/src/siilihai/parser/*.h
    HEADERS += $$LIB_PATH/src/siilihai/tapatalk/*.h
    HEADERS += $$LIB_PATH/src/siilihai/forumdata/*.h
    HEADERS += $$LIB_PATH/src/siilihai/forumdatabase/*.h
    INCLUDEPATH += $$LIB_PATH/src/
    message(Building WITH lib included in binary! Lib source in $$LIB_PATH)
} else {
    LIBS += -lsiilihai
}

HEADERS += reader/messageviewwidget.h \
    reader/threadlistwidget.h \
    reader/settingsdialog.h \
    reader/reportparser.h \
    reader/forumlistwidget.h \
    reader/favicon.h \
    reader/groupsubscriptiondialog.h \
    reader/subscribewizard.h \
    reader/mainwindow.h \
    reader/siilihai.h \
    reader/loginwizard.h \
    reader/forumproperties.h \
    reader/threadproperties.h \
    reader/threadlistmessageitem.h \
    reader/threadlistthreaditem.h \
    reader/threadlistshowmoreitem.h \
    reader/useraccountdialog.h

SOURCES += reader/messageviewwidget.cpp \
    reader/threadlistwidget.cpp \
    reader/settingsdialog.cpp \
    reader/reportparser.cpp \
    reader/forumlistwidget.cpp \
    reader/favicon.cpp \
    reader/groupsubscriptiondialog.cpp \
    reader/subscribewizard.cpp \
    reader/mainwindow.cpp \
    reader/siilihai.cpp \
    reader/loginwizard.cpp \
    reader/main.cpp \
    reader/forumproperties.cpp \
    reader/threadproperties.cpp \
    reader/threadlistmessageitem.cpp \
    reader/threadlistthreaditem.cpp \
    reader/threadlistshowmoreitem.cpp \
    reader/useraccountdialog.cpp

FORMS += common/credentialsdialog.ui
HEADERS += common/credentialsdialog.h
SOURCES += common/credentialsdialog.cpp

HEADERS += parsermaker/openrequestdialog.h \
    parsermaker/messagelistpatterneditor.h \
    parsermaker/grouplistpatterneditor.h \
    parsermaker/threadlistpatterneditor.h \
    parsermaker/patterneditor.h \
    parsermaker/downloaddialog.h \
    parsermaker/parsermaker.h

SOURCES += parsermaker/openrequestdialog.cpp \
    parsermaker/messagelistpatterneditor.cpp \
    parsermaker/grouplistpatterneditor.cpp \
    parsermaker/threadlistpatterneditor.cpp \
    parsermaker/patterneditor.cpp \
    parsermaker/downloaddialog.cpp \
    parsermaker/parsermaker.cpp

FORMS += parsermaker/openrequestdialog.ui \
    parsermaker/patterneditor.ui \
    parsermaker/downloaddialog.ui \
    parsermaker/parsermaker.ui

FORMS += reader/settingsdialog.ui \
    reader/reportparser.ui \
    reader/groupsubscriptiondialog.ui \
    reader/subscribeforum_verify.ui \
    reader/subscribeforum_login.ui \
    reader/subscribeforum.ui \
    reader/mainwindow.ui \
    reader/forumproperties.ui \
    reader/threadproperties.ui \
    reader/useraccountdialog.ui

RESOURCES += ../siilihairesources.qrc

win32 {
    INCLUDEPATH += ../../libsiilihai/src/
}
win32:debug {
    LIBS += -L../../libsiilihai/src/debug
    DEPENDPATH += ../../libsiilihai/src/debug
}
win32:release {
    LIBS += -L../../libsiilihai/src/release
    DEPENDPATH += ../../libsiilihai/src/release
}

QMAKE_CLEAN += src/*.o
