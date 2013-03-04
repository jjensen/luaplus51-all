pushd
cd ..\Modules\lua-curl\src
if exist curl-7.27.0.zip del curl-7.27.0.zip
if exist curl-7.27.0 rmdir /s /q curl-7.27.0
wget http://curl.haxx.se/download/curl-7.27.0.zip
unzip curl-7.27.0.zip
del curl-7.27.0.zip
popd
