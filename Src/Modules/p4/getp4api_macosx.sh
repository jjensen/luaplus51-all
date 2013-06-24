rm -rf p4api
mkdir p4api
if [ -f p4api.tgz ]
then
	rm p4api.tgz
fi
curl ftp://ftp.perforce.com/perforce/r12.1/bin.macosx105x86/p4api.tgz -o p4api.tgz
tar xvfz p4api.tgz
mv -f p4api-2012.1.442152/* p4api
rm -rf p4api-2012.1.442152
rm p4api.tgz

