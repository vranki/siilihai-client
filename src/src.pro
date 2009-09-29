TEMPLATE = app
TARGET = siilihai
QT += core \
    webkit \
    gui \
    network \
    xml \
    sql
HEADERS += subscribewizard.h \
    mainwindow.h \
    siilihai.h \
    loginwizard.h
SOURCES += subscribewizard.cpp \
    mainwindow.cpp \
    siilihai.cpp \
    loginwizard.cpp \
    main.cpp
BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share
target.path = $$BINDIR
INSTALLS += target
FORMS = subscribeforum_verify.ui \
    subscribeforum_login.ui \
    subscribeforum.ui \
    mainwindow.ui
RESOURCES = ../siilihairesources.qrc
INCLUDEPATH += /usr/include/siilihai

# DEPENDPATH += /usr/include/siilihai
LIBS += -lsiilihai
