#/bin/bash
cp *.spec *.changes *.tar.gz ~/src/home:vranki/siilihai-client
cp ../../siilihai-client_*.tar.gz ../../siilihai-client*.dsc ~/src/home:vranki/siilihai-client
pushd ~/src/home:vranki/siilihai-client
osc commit -m 'new stuff'
popd

