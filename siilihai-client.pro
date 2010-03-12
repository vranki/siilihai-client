TEMPLATE = subdirs
SUBDIRS = src
RESOURCES = siilihairesources.qrc
OTHER_FILES = debian/control debian/rules debian/changelog
CONFIG += qt debug

maemo_desktops.path = /usr/share/applications/hildon
maemo_desktops.files = data/siilihai-client.desktop

INSTALLS += maemo_desktops

maemo_services.path = /usr/share/dbus-1
maemo_services.files = data/siilihai-client.service

INSTALLS += maemo_services

icons.path = /usr/share/pixmaps
icons.files = data/*.png

INSTALLS += icons

blankmessage.path = /usr/share/siilihai
blankmessage.files = data/blankmessage

INSTALLS += blankmessage
