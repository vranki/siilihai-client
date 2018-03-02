TEMPLATE = subdirs
SUBDIRS = src src/qmlreader
RESOURCES = siilihairesources.qrc
OTHER_FILES += debian/control debian/rules debian/changelog debian/siilihai-client.install debian/copyright \
    data/blankmessage/* rpm/* \
    android/res/values-ja/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/layout/splash.xml \
    android/res/values-nb/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-ms/strings.xml \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/version.xml \
    android/AndroidManifest.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/values/libs.xml \
    android/res/drawable-hdpi/icon.png

OTHER_FILES += scripts/upload_debs.sh scripts/buildwin.sh
OTHER_FILES += siilihai.nsi data/siilihai-client.desktop README.md

DATADIR = $$[QT_INSTALL_PREFIX]/share

desktops.path = $${DATADIR}/applications
desktops.files = data/siilihai-client.desktop

INSTALLS += desktops

icons.path = $${DATADIR}/icons/hicolor/scalable/apps
icons.files = data/siilihai-client.svg

INSTALLS += icons

QMAKE_DISTCLEAN += src/*.o src/moc_*.cpp debian/*.log -r src/debug src/release debian/tmp debian/siilihai-client debian/siilihai-client-dbg debian/libsiilihai
