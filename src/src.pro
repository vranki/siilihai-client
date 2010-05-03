TEMPLATE = subdirs
CONFIG = ordered

SUBDIRS = common

!exists(/etc/libosso) {
        SUBDIRS += parsermaker reader
}

exists(/etc/libosso) {
        SUBDIRS += reader
}
