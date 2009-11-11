TEMPLATE = subdirs
SUBDIRS = src
RESOURCES = siilihairesources.qrc
CONFIG += debug

#desktops.path = /usr/share/applications
#desktops.files = data/siilihai-client.desktop

#INSTALLS += desktops

maemo_desktops.path = /usr/share/applications/hildon
maemo_desktops.files = data/siilihai-client.desktop

INSTALLS += maemo_desktops

maemo_services.path = /usr/share/dbus-1
maemo_services.files = data/siilihai-client.service

INSTALLS += maemo_services

icons.path = /usr/share/icons/siilihai
icons.files = data/*.png

INSTALLS += icons
