if exist p4api rmdir /s /q p4api
mkdir p4api
if exist p4api_vs2010_dyn.zip del p4api_vs2010_dyn.zip
if exist p4api-2010.1.260003-vs2010_dyn rmdir /s /q p4api-2010.1.260003-vs2010_dyn
wget ftp://ftp.perforce.com/perforce/r10.1/bin.ntx86/p4api_vs2010_dyn.zip
unzip p4api_vs2010_dyn.zip
if exist p4api-2010.1.259176-vs2010_dyn rmdir /s /q p4api-2010.1.259176-vs2010_dyn
ren p4api-2010.1.260003-vs2010_dyn p4api\release
del p4api_vs2010_dyn.zip

if exist p4api_vs2010_dyn_vsdebug.zip del p4api_vs2010_dyn_vsdebug.zip
if exist p4api-2010.1.260003-vs2010_dyn_vsdebug rmdir /s /q p4api-2010.1.260003-vs2010_dyn_vsdebug
wget ftp://ftp.perforce.com/perforce/r10.1/bin.ntx86/p4api_vs2010_dyn_vsdebug.zip
unzip p4api_vs2010_dyn_vsdebug.zip
if exist p4api-2010.1.259176-vs2010_dyn_vsdebug rmdir /s /q p4api-2010.1.259176-vs2010_dyn_vsdebug
ren p4api-2010.1.260003-vs2010_dyn_vsdebug p4api\debug
del p4api_vs2010_dyn_vsdebug.zip

