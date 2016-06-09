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

# checks for a bug in rescaling with collapsed handles

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source "assert.tcl"
Ravel r
# need at least two handles for them to get sizes allocated
r.addHandle {} {}
r.addHandle {} {}
r.handles.@elem 0
r.xHandleId 0
r.rescale 100
assert {[r.radius]==100} {}
assert {[r.handles(0).x]==100} {}
assert {[r.handles(0).y]==0} {}
# rescale with handle collapsed to check for a certain bug
r.handles(0).toggleCollapsed
r.rescale 200
r.handles(0).toggleCollapsed
assert {[r.handles(0).x]==200} {}
assert {[r.handles(0).y]==0} {}
EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
