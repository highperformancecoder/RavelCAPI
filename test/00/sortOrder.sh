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
source "$here/test/assert.tcl"
Ravel ravel
ravel.addHandle "x" {1990 1991 1992 1993}
set h [ravel.handles.@elem 0]
\$h.displayFilterCaliper 1
\$h.sliceLabels.setCalipers 1991 1992
assert {[ravel.handles(0).sliceLabels]=={1991 1992}}
assert {[ravel.handles(0).minSliceLabel]=="1991"}
assert {[ravel.handles(0).maxSliceLabel]=="1992"}
\$h.sliceLabels.order reverse
assert {[ravel.handles(0).sliceLabels]=={1992 1991}}
assert {[ravel.handles(0).minSliceLabel]=="1992"}
assert {[ravel.handles(0).maxSliceLabel]=="1991"}
EOF

cp $here/test/assert.tcl .
cp $here/PatentsByCountry1980-2011.csv .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
