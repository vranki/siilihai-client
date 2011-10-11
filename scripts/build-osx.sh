#!/bin/sh
# Builds an OS X app bundle of Siilihai and packages it into a .dmg.

# OS X doesn't have GNU readlink -f equivalent, so...
SCRIPT=$(python -c 'import os,sys;print os.path.realpath(sys.argv[1])' $0)
SCRIPTDIR=$(dirname $SCRIPT)
SRCBASE=$SCRIPTDIR/../..
BUILDDIR=$(mktemp -d -t siilihai-build)
LOGFILE=$BUILDDIR/build.log

die() {
	echo $* >&2
	exit 1
}

status() {
	echo "*******************************************************************************"
	echo "* $*"
}

status "Build started $(date)"

[ -n "$(which qmake)" ] || die "No qmake found in PATH"
[ -n "$(which macdeployqt)" ] || die "No macdeployqt found in PATH"
QTVER=$(qmake -v|sed -n 's/^Using //p')
status "Building with $QTVER"

[ -n "$(which install_name_tool)" ] || die "install_name_tool not in path"
[ -n "$(which otool)" ] || die "otool not in path"

[ -d "$BUILDDIR" ] || die "Unable to create build directory"

LIBSRCDIR=$SRCBASE/libsiilihai
CLIENTSRCDIR=$SRCBASE/siilihai-client

[ -d "$LIBSRCDIR" ] || die "Expected to find libsiilihai source in $LIBSRCDIR"
[ -d "$CLIENTSRCDIR" ] || die "Expected to find siilihai-client source in $CLIENTSRCDIR"

LIBBUILDDIR=$BUILDDIR/libsiilihai
CLIENTBUILDDIR=$BUILDDIR/siilihai-client

mkdir "$LIBBUILDDIR" || die "Unable to create directory $LIBBUILDDIR"
mkdir "$CLIENTBUILDDIR" || die "Unable to create directory $CLIENTBUILDDIR"

(
	status "Building libsiilihai in $LIBBUILDDIR"
	cd "$LIBBUILDDIR"
	qmake CONFIG+=release "$LIBSRCDIR" >> "$LOGFILE" 2>&1 \
		&& make >> "$LOGFILE" 2>&1
) || die "Building libsiilihai failed, see $LOGFILE"

(
	status "Building siilihai-client in $CLIENTBUILDDIR"
	cd "$CLIENTBUILDDIR"
	qmake CONFIG+=release \
		LIBS+=-L"$LIBBUILDDIR/src" \
		INCLUDEPATH+="$LIBSRCDIR/src" \
		"$CLIENTSRCDIR" >> "$LOGFILE" 2>&1 \
		&& make >> "$LOGFILE" 2>&1
) || die "Building siilihai-client failed, see $LOGFILE"

status "Copying libsiilihai dylibs into client bundle"
BUNDLEDIR=$CLIENTBUILDDIR/src/reader/siilihai.app
FWKDIR=$BUNDLEDIR/Contents/Frameworks
mkdir -p "$FWKDIR"
for LIB in "$LIBBUILDDIR"/src/*.dylib "$CLIENTBUILDDIR"/src/common/*.dylib "$CLIENTBUILDDIR"/src/parsermaker/*.dylib; do
	cp -a "$LIB" "$FWKDIR"
	LIBBASENAME=$(basename $LIB)
	LIB=$FWKDIR/$LIBBASENAME
	install_name_tool -id "@executable_path/../Frameworks/$LIBBASENAME" \
		"$LIB"
	QTDEPS=$(otool -L $LIB|egrep 'Qt.*\.framework'|egrep -v '@executable_path'|awk '{print $1}'|tr '\n' ' ')
	for DEP in $(echo $QTDEPS); do
		NEWDEP=$(echo $DEP|sed 's%.*gcc/lib/%%')
		install_name_tool -change "$DEP" \
			"@executable_path/../Frameworks/$NEWDEP" "$LIB"
	done
	DEPS=$(otool -L $LIB|egrep 'libsiilihai'|egrep -v '@executable_path'|awk '{print $1}'|tr '\n' ' ')
	for DEP in $(echo $DEPS); do
		install_name_tool -change "$DEP" \
			"@executable_path/../Frameworks/$DEP" "$LIB"
	done
done

status "Running macdeployqt on $BUNDLEDIR"
macdeployqt "$BUNDLEDIR"

if [ -e siilihai.app ]; then
	for ((i=1;$i<=1000;i=$i+1)); do
		TEMPNAME=siilihai.app.$i
		if [ ! -e "$TEMPNAME" ]; then
			break
		elif [ $i -eq 1000 ]; then
			die "Refusing to make more than 1000 backups"
		fi
	done
	status "Moving old siilihai.app to $TEMPNAME"
	mv siilihai.app "$TEMPNAME"
fi

status "Copying app bundle to current dir"
cp -a "$BUNDLEDIR" .

status "Build completed $(date), log is $LOGFILE"
