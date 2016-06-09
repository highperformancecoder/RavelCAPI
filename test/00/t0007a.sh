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

# test initDataSpec here
cat >input.csv <<EOF
comment row
axc,,a,1d,b,c
ax1,ax2,,,,
a,a,1,2,3,4
a,b,2 ,5,4,3
b,a,3, 5,4,6
b,b,4,5,,7
EOF

cat >input2.csv <<EOF
axc,,1980,1981,1982,1983
a,a,1,2,3,4
a,b,2 ,5,4,3
b,a,3, 5,4,6
b,b,4,5,,7
EOF

cat >input3.csv <<EOF
comment row
axc,,a,1d,b,c
ax1
a,1,2,3,4
a,2 ,5,4,3
b,3, 5,4,6
b,4,5,,7
EOF


# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source "assert.tcl"
DataCube dc
dc.separator ","
dc.initSpec input.csv
assert {[dc.commentRows]==0} "input.csv"
assert {[dc.nColAxes]==2} "input.csv"
assert {[dc.nRowAxes]==3} "input.csv"

dc.initSpec input2.csv
assert {[dc.commentRows]==""} "input2.csv"
assert {[dc.nColAxes]==2} "input2.csv"
assert {[dc.nRowAxes]==1} "input2.csv"

dc.initSpec input3.csv
assert {[dc.commentRows]==0} "input3.csv"
assert {[dc.nColAxes]==1} "input3.csv"
assert {[dc.nRowAxes]==3} "input3.csv"


EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
