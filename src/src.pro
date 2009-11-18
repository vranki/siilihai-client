TEMPLATE = subdirs
CONFIG = ordered

!exists(/etc/libosso) {
	SUBDIRS = parsermaker reader
}

exists(/etc/libosso) {
	SUBDIRS = reader 
}
