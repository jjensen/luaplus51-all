cd ../Modules/lrexlib
if [ -d pcre-8.30 ]
then
	rm -rf pcre-8.30
fi
curl ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.30.tar.bz2 -o pcre-8.30.tar.bz2
tar -xjf pcre-8.30.tar.bz2

cd pcre-8.30
./configure

if ! [ -f pcre_chartables.c ]
then
	cp pcre_chartables.c.dist pcre_chartables.c
fi

cd ..

rm pcre-8.30.tar.bz2
