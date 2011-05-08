TEMPLATE = app
TARGET = siilihai

isEmpty(PREFIX) {
  PREFIX = /usr
}

BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share
DESTDIR = .

!contains(QMAKE_HOST.arch, x86_64) {
   LIBDIR = $$PREFIX/lib
} else {
   LIBDIR = $$PREFIX/lib64
}

exists(../../siilihai-version.h) {
     DEFINES += INCLUDE_SIILIHAI_VERSION
}

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
    threadproperties.h \
    threadlistmessageitem.h \
    threadlistthreaditem.h \
    threadlistshowmoreitem.h

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
    threadproperties.cpp \
    threadlistmessageitem.cpp \
    threadlistthreaditem.cpp \
    threadlistshowmoreitem.cpp

DEPENDPATH += ../parsermaker ../common

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

win32 {
    INCLUDEPATH += ../../../libsiilihai
    DEFINES += STORE_FILES_IN_APP_DIR
    message("Win32 build - storing files in app dir")
}
win32:debug {
    LIBS += -L../../../libsiilihai/src/debug
    LIBS += -L../common/debug
    LIBS += -L../parsermaker/debug
    DEPENDPATH += ../../../libsiilihai/src/debug
    DEPENDPATH += -L../common/debug
}
win32:release {
    LIBS += -L../../../libsiilihai/src/release
    LIBS += -L../common/release
    LIBS += -L../parsermaker/release
    DEPENDPATH += ../../../libsiilihai/src/release
    DEPENDPATH += -L../common/release
}

LIBS += -L../parsermaker \
    -L../common \
    -lsiilihai \
    -lsiilihai-common \
    -lsiilihai-parsermaker
