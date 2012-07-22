SUBDIRS += reader

TEMPLATE = app
TARGET = siilihai
target.path = $$[QT_INSTALL_BINS]
INSTALLS += target

ICON = ../data/siilihai.icns

#unix {
#    CONFIG += debug
#    CONFIG -= release
#}

exists(../siilihai-version.h) {
     DEFINES += INCLUDE_SIILIHAI_VERSION
}

#CONFIG(debug) {
#    message(Debug build - enabling some extra stuff)
#    DEFINES += DEBUG_INFO
#}

CONFIG(debug_info) {
    message(XBXBMXBMM . enabling some extra stuff)
    DEFINES += DEBUG_INFO
}

QT += core webkit gui network xml

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
    INCLUDEPATH += ../../../libsiilihai
    DEFINES += STORE_FILES_IN_APP_DIR
    message("Win32 build - storing files in app dir")
}
win32:debug {
    LIBS += -L../../../libsiilihai/src/debug
    DEPENDPATH += ../../../libsiilihai/src/
}
win32:release {
    LIBS += -L../../../libsiilihai/src/release
    DEPENDPATH += ../../../libsiilihai/src/
}

LIBS += -lsiilihai
