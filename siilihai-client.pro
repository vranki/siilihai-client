TEMPLATE = subdirs
SUBDIRS = src
RESOURCES = siilihairesources.qrc
OTHER_FILES += debian/control debian/rules debian/changelog debian/siilihai-client.install debian/copyright
OTHER_FILES += scripts/upload_debs.sh scripts/buildwin.sh
OTHER_FILES += siilihai.nsi data/siilihai-client.desktop

CONFIG += qt

DATADIR = $$[QT_INSTALL_PREFIX]/share

maemo_desktops.path = $${DATADIR}/applications/hildon
maemo_desktops.files = data/siilihai-client.desktop

INSTALLS += maemo_desktops

maemo_services.path = $${DATADIR}/dbus-1
maemo_services.files = data/siilihai-client.service

INSTALLS += maemo_services

icons.path = $${DATADIR}/pixmaps
icons.files = data/siilis_icon_48.png

INSTALLS += icons
