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

cat >input.csv <<EOF
axc,,a,d,b,c
ax1,ax2,
a,a,1,2,3,4
a,b,2 ,5,4,3
b,a,3, 5,4,6
b,b,4,5,6,7
EOF

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source "assert.tcl"
DataCube dc
dc.nRowAxes 2
dc.nColAxes 2
dc.separator ","
dc.loadFile input.csv
assert {[dc.dimNames]=="ax1 ax2 axc"} ""
assert {[dc.dimLabels]=="{a b} {a b} { a d b c}"} ""
EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
