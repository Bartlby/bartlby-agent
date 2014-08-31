./autogen.sh
./configure --enable-nrpe --enable-ssl --prefix=$PWD/installation --with-user=travis
make
if [ $? != 0 ]; then
	exit $?;
fi;
make install
ls -l $PWD/installation

exit 0;







