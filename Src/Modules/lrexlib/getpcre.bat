if exist pcre-7.8.zip del pcre-7.8.zip
if exist pcre-7.8 rmdir /s /q pcre-7.8
if exist pcre-8.00.zip del pcre-8.00.zip
if exist pcre-8.00 rmdir /s /q pcre-8.00
rem if exist pcre-7.8 del pcre-7.8.tar.gz
rem wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-7.8.tar.gz
rem gunzip pcre-7.8.tar.gz
rem tar xvf pcre-7.8.tar
rem del pcre-7.8.tar
wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.00.zip
unzip pcre-8.00.zip
del pcre-8.00.zip

cd pcre-8.00
if not exist pcre.h copy pcre.h.generic pcre.h
if not exist config.h copy config.h.generic config.h
if not exist pcre_chartables.c copy pcre_chartables.c.dist pcre_chartables.c
cd ..
