TEMPLATE = subdirs

!exists(/etc/libosso) {
SUBDIRS = parsermaker reader 
}

exists(/etc/libosso) {
SUBDIRS = reader 
}
