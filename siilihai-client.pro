TEMPLATE = subdirs
SUBDIRS = src
RESOURCES = siilihairesources.qrc
CONFIG += debug

desktops.path = /usr/share/applications
desktops.files = data/siilihai-client.desktop

INSTALLS += desktops

icons.path = /usr/share/icons/siilihai
icons.files = data/siilis_icon.png

INSTALLS += icons
