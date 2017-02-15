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

# test of mouse handling functionality

# insert ecolab script code here
# use \$ in place of $ to refer to variable contents
# exit 0 to indicate pass, and exit 1 to indicate failure
cat >input.tcl <<EOF
source assert.tcl
Ravel ravel
ravel.x 0
ravel.y 0
ravel.addHandle x {a b c}
ravel.addHandle y {a b c}
ravel.addHandle z {a b c}
assert {[ravel.handles.size]>=3} ""
ravel.handles.@elem 0
ravel.handles.@elem 1
ravel.handles.@elem 2

ravel.rescale 100
assert {[ravel.radius]==100} ""

# swap handles using mouse
ravel.onMouseDown 50 0
assert {[lindex [ravel.handleIds] 0]==0} ""
assert {[ravel.onMouseMotion 0 50]==1} ""
assert {[lindex [ravel.handleIds] 0]==0} "mouse move"
ravel.onMouseUp 0 50
assert {[lindex [ravel.handleIds] 0]==1} ""

# swap with third handle - exercises different code path
ravel.onMouseDown 50 0
ravel.onMouseUp -50 -50
assert {[lindex [ravel.handleIds] 0]==2} ""
# and revert
ravel.onMouseDown -50 -50
ravel.onMouseUp 50 0
assert {[lindex [ravel.handleIds] 0]==1} ""
ravel.onMouseDown 0 50
ravel.onMouseUp -50 -50
assert {[lindex [ravel.handleIds] 1]==2} ""
# and revert
ravel.onMouseDown -50 -50
ravel.onMouseUp 0 50
assert {[lindex [ravel.handleIds] 1]==0} ""

# move slicer
assert {[ravel.handles(2).sliceIndex]==0} ""
assert {[ravel.handles(2).sliceLabel]=="a"} ""
set incr [expr [ravel.handles(2).x]/(1+[ravel.handles(2).sliceLabels.size])]
set fincr [expr 2*\$incr]
ravel.onMouseDown \$incr \$incr
ravel.onMouseMotion \$fincr \$fincr
ravel.onMouseUp \$fincr \$fincr
assert {[ravel.handles(2).sliceIndex]==1} ""
assert {[ravel.handles(2).sliceLabel]=="b"} ""

# checks slicer movement stays within bounds
ravel.onMouseDown \$fincr \$fincr
ravel.onMouseUp 1 1
assert {[ravel.handles(2).sliceIndex]==0} ""
ravel.onMouseDown \$incr \$incr
ravel.onMouseUp -200 -200
assert {[ravel.handles(2).sliceIndex]==2} ""
ravel.handles(2).sliceIndex 1

# now move filter caliper
ravel.handles(1).displayFilterCaliper 1
set incr [expr [ravel.handles(1).x]/(1+[ravel.handles(1).sliceLabels.size])]
set fincr [expr 2*\$incr]
assert {[ravel.handles(1).sliceMin]==0} ""
assert {[ravel.handles(1).minSliceLabel]=="a"} ""
ravel.onMouseDown \$incr 0
ravel.onMouseUp \$fincr 0
assert {[ravel.handles(1).sliceMin]==1} ""
assert {[ravel.handles(1).minSliceLabel]=="b"} ""

assert {[ravel.handles(1).sliceMax]>=[expr [ravel.handles(2).sliceLabels.size]-1]} ""
assert {[ravel.handles(1).maxSliceLabel]=="c"} ""
ravel.onMouseDown [expr 3*\$incr] 0
ravel.onMouseUp \$fincr 0
assert {[ravel.handles(1).sliceMax]==1} ""
assert {[ravel.handles(1).maxSliceLabel]=="b"} ""


EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
