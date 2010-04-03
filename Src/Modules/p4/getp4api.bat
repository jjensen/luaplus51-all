if exist p4api rmdir /s /q p4api
mkdir p4api
if exist p4api_vs2005_dyn.zip del p4api_vs2005_dyn.zip
if exist p4api-2008.2.179173-vs2005_dyn rmdir /s /q p4api-2008.2.179173-vs2005_dyn
if exist p4api_vs2008_dyn.zip del p4api_vs2008_dyn.zip
if exist p4api-2009.1.209442-vs2008_dyn rmdir /s /q p4api-2009.1.209442-vs2008_dyn
wget ftp://ftp.perforce.com/perforce/r09.1/bin.ntx86/p4api_vs2008_dyn.zip
unzip p4api_vs2008_dyn.zip
ren p4api-2009.1.209442-vs2008_dyn p4api\release
del p4api_vs2008_dyn.zip

if exist p4api_vs2005_dyn_vsdebug.zip del p4api_vs2005_dyn_vsdebug.zip
if exist p4api-2008.2.179173-vs2005_dyn_vsdebug rmdir /s /q p4api-2008.2.179173-vs2005_dyn_vsdebug
if exist p4api_vs2008_dyn_vsdebug.zip del p4api_vs2008_dyn_vsdebug.zip
if exist p4api-2009.1.209442-vs2008_dyn_vsdebug rmdir /s /q p4api-2009.1.209442-vs2008_dyn_vsdebug
wget ftp://ftp.perforce.com/perforce/r09.1/bin.ntx86/p4api_vs2008_dyn_vsdebug.zip
unzip p4api_vs2008_dyn_vsdebug.zip
ren p4api-2009.1.209442-vs2008_dyn_vsdebug p4api\debug
del p4api_vs2008_dyn_vsdebug.zip

