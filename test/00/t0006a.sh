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

# histogram testing

trap "fail" 1 2 3 15

cat >input.csv <<EOF
comment row
comment,ax1,ax2,a,1d,b,c
,a,a,1,2,3,4
,a,b,2 ,5,4,3
,b,a,3, 5,4,6
,b,b,4,5,,7
EOF

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source "assert.tcl"
DataCube dc
dc.nColAxes 3
dc.nRowAxes 2
dc.commentCols 0
dc.commentRows 0
dc.separator ","
dc.loadFile input.csv

dc.resizeHistogram 6
dc.populateArray data
assert {[dc.histogram]=={1 1 2 2 1 1}} {}


dc.resizeHistogram 3
dc.populateArray data
assert {[dc.histogram]=={2 4 2}} {}
EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
