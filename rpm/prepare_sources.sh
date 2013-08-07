#/bin/bash
version=2.2.7
target=siilihai-client

rm -rf /tmp/$target-*
rm *.tar.gz
rm ../../$target*.tar.gz ../../$target*.dsc

pushd ..
qmake
make clean
make distclean
popd

pushd ../..
cp -r $target /tmp/$target-$version
rm -rf /tmp/$target-$version/.git
popd

pushd /tmp
tar cvfz $target-$version.tar.gz $target-$version
popd

mv /tmp/$target-$version.tar.gz .
pushd ..
debuild -S -us -uc -I.git
popd

