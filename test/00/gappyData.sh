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
cat >input.csv <<EOF
ax1,ax2,a,1b,c,d
a,a,1,2,,4
a,b,2 ,3,4,5
b,a,, 4,5,6
b,b,4,5,6,7
EOF

cat >input.tcl <<EOF
source "assert.tcl"
DataCube dc
dc.nColAxes 2
dc.nRowAxes 1
dc.separator ","

dc.loadFile input.csv
assert {[dc.ravel.handleIds]=="2 0"} ""

dc.ravel.handles.@elem 0
dc.ravel.handles.@elem 1
dc.ravel.handles.@elem 2

assert {[dc.ravel.handles(1).sliceIndex]==0} ""
assert {[dc.ravel.handles(1).sliceLabel]=="a"} ""
assert {[dc.ravel.handles(1).sliceLabels]=="a b"} ""
dc.populateArray data

# first check labels
set aa "a"
set bb "b"
assert "\"\$data(0,1)\"==\"a\"" "d(0,1) sl0"
assert "\"\$data(0,2)\"==\"1b\"" "d(0,2) sl0"
assert "\"\$data(1,0)\"==\"a\"" "d(1,0) sl0"
assert "\"\$data(2,0)\"==\"b\"" "d(2,0) sl0"

# now data
assert "\$data(1,1)==1" "d(1,1) sl0"
assert "\$data(1,2)==2" "d(1,2) sl0"
assert "![info exists data(2,1)]" "d(2,1) sl0"
assert "\$data(2,2)==4" "d(2,2) sl0"

# check another slice
dc.ravel.handles(1).sliceIndex 1
dc.populateArray data
assert "\$data(1,1)==2" "d(1,1) sl1"
assert "\$data(1,2)==3" "d(1,2) sl1"
assert "\$data(2,1)==4" "d(2,1) sl1"
assert "\$data(2,2)==5" "d(2,2) sl1"

# collapse handle 1
dc.ravel.handles(1).moveTo [expr 0.1*[dc.ravel.handles(1).x]] [expr 0.1*[dc.ravel.handles(1).y]] 0
assert {[dc.ravel.handles(1).collapsed]} {}

dc.ravel.handles(1).reductionOp sum
dc.populateArray data
assert "\$data(1,1)==3" "d(1,1) sum"
assert "\$data(1,2)==5" "d(1,2) sum"
assert "\$data(2,1)==4" "d(2,1) sum"
assert "\$data(2,2)==9" "d(2,2) sum"
assert "\$data(1,3)==4" "d(2,1) sum"
assert "\$data(2,3)==11" "d(2,2) sum"
assert "\$data(1,4)==9" "d(2,1) sum"
assert "\$data(2,4)==13" "d(2,2) sum"

dc.ravel.handles(1).reductionOp prod
dc.populateArray data
assert "\$data(1,1)==2" "d(1,1) prod"
assert "\$data(1,2)==6" "d(1,2) prod"
assert "\$data(2,1)==4" "d(2,1) prod"
assert "\$data(2,2)==20" "d(2,2) prod"

dc.ravel.handles(1).reductionOp av
dc.populateArray data
assert "\$data(1,1)==1.5" "d(1,1) av"
assert "\$data(1,2)==2.5" "d(1,2) av"
assert "\$data(2,1)==4" "d(2,1) av"
assert "\$data(2,2)==4.5" "d(2,2) av"

dc.ravel.handles(1).reductionOp stddev
dc.populateArray data
assert "\$data(1,1)==.5" "d(1,1) stddev"
assert "\$data(1,2)==.5" "d(1,2) stddev"
assert "\$data(2,1)==0" "d(2,1) stddev"
assert "\$data(2,2)==.5" "d(2,2) stddev"

dc.ravel.handles(1).reductionOp max
dc.populateArray data
assert "\$data(1,1)==2" "d(1,1) max"
assert "\$data(1,2)==3" "d(1,2) max"
assert "\$data(2,1)==4" "d(2,1) max"
assert "\$data(2,2)==5" "d(2,2) max"

dc.ravel.handles(1).reductionOp min
dc.populateArray data
assert "\$data(1,1)==1" "d(1,1) min"
assert "\$data(1,2)==2" "d(1,2) min"
assert "\$data(2,1)==4" "d(2,1) min"
assert "\$data(2,2)==4" "d(2,2) min"

dc.ravel.handles(1).reductionOp sum

# swap x & y handles
dc.ravel.handles(2).moveTo [dc.ravel.handles(0).x] [dc.ravel.handles(0).y] 1
dc.ravel.snapHandle 2
assert {[dc.ravel.handleIds]=="0 2"} {}
dc.populateArray data
assert "\$data(1,1)==3" "d(1,1) swapped"
assert "\$data(2,1)==5" "d(2,1) swapped"
assert "\$data(1,2)==4" "d(1,2) swapped"
assert "\$data(2,2)==9" "d(2,2) swapped"

# collapse x handle
dc.ravel.handles(0).moveTo [expr 0.1*[dc.ravel.handles(0).x]] [expr 0.1*[dc.ravel.handles(0).y]] 0
assert {[dc.ravel.handles(0).collapsed]} {}
assert {[dc.ravel.handles(0).reductionOp]=="sum"} {}
dc.populateArray data
assert "\$data(1,1)==7" "d(1,1) x summed"
assert "\$data(2,1)==14" "d(2,1) x summed"
assert "\$data(3,1)==15" "d(2,1) x summed"
assert "\$data(4,1)==22" "d(4,1) x summed"

# collapse y handle
dc.ravel.handles(2).moveTo [expr 0.1*[dc.ravel.handles(2).x]] [expr 0.1*[dc.ravel.handles(2).y]] 0
assert {[dc.ravel.handles(2).collapsed]} {}
assert {[dc.ravel.handles(2).reductionOp]=="sum"} {}
dc.populateArray data
assert "\$data(1,1)==58" "d(1,1) y summed"

# expand x handle
dc.ravel.handles(0).moveTo [expr 5*[dc.ravel.handles(0).x]] [expr 5*[dc.ravel.handles(0).y]] 0
assert {![dc.ravel.handles(0).collapsed]} {}
dc.populateArray data
assert "\$data(1,1)==21" "d(1,1) x summed"
assert "\$data(1,2)==37" "d(1,2) x summed"


EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

pass
