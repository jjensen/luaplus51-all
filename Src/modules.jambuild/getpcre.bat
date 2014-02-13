pushd
cd ..\Modules\lrexlib
if exist pcre-8.34.zip del pcre-8.34.zip
if exist pcre-8.34 rmdir /s /q pcre-8.34
wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.34.zip
unzip pcre-8.34.zip
del pcre-8.34.zip

cd pcre-8.34
if not exist pcre.h copy pcre.h.generic pcre.h
if not exist config.h copy config.h.generic config.h
if not exist pcre_chartables.c copy pcre_chartables.c.dist pcre_chartables.c
cd ..
popd
