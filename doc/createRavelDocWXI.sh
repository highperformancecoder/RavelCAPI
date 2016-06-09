#!/bin/bash

# should be run from doc directory

WXIOUT=../Ravel/Installer/ravelDoc.wxi
cat >$WXIOUT <<EOF
<Include>
 <Directory Id='DocsDir' Name='RavelDoc'>
  <Component Id='RavelDoc' DiskId='1' Guid='516bc6f9-7943-47d0-9d9d-782518bfbc65'>
   <File Id='index.html' Name='index.html' Source='../../doc/ravelDoc/index.html' KeyPath='yes'/>
EOF

id=0
for i in ravelDoc/*; do
    if [ ${i##*/} != "index.html" ]; then
        let id++
        echo "   <File Id='fid$id' Name='${i##*/}' Source='../../doc/$i' KeyPath='no'/>" >>$WXIOUT
    fi
done

cat >>$WXIOUT <<EOF
  </Component>
 </Directory>
</Include>
EOF
