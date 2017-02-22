#! /bin/sh

here=`pwd`
if test $? -ne 0; then exit 2; fi
tmp=/tmp/$$
mkdir $tmp
if test $? -ne 0; then exit 2; fi
cd $tmp
if test $? -ne 0; then exit 2; fi

fail()
{
    echo "FAILED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 1
}

pass()
{
    echo "PASSED" 1>&2
    cd $here
    chmod -R u+w $tmp
    rm -rf $tmp
    exit 0
}

trap "fail" 1 2 3 15

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
DataCube dc
dc.separator ","
dc.initSpec "PatentsByCountry1980-2011.csv"

dc.loadFile "PatentsByCountry1980-2011.csv"
dc.ravel.handles.@elem 2
dc.renameAxis 2 "year"
dc.populateArray dataCube
EOF

cp $here/test/assert.tcl .
cp $here/PatentsByCountry1980-2011.csv .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
