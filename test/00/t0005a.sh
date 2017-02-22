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

# value filtering

cat >input.csv <<EOF
ax1,ax2,a,d,b,c
a,a,1,2,3,4
a,b,2 ,5,4,3
b,a,3, 5,4,6
b,b,4,5,6,7
EOF

cat >inputNum.csv <<EOF
ax1,ax2,100,200,1000,50
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
dc.nColAxes 2
dc.nRowAxes 1
dc.separator ","
dc.loadFile input.csv

dc.filterMin 2
dc.filterMax 5
dc.populateArray data
foreach idx [array names data] {
  if {[string is double \$data(\$idx)]} {
    assert "\$data(\$idx)>=\[dc.filterMin\]" "data(\$idx)"
    assert "\$data(\$idx)<=\[dc.filterMax\]" "data(\$idx)"
  }
}
EOF

cp $here/test/assert.tcl .
$here/ravelTest input.tcl
if test $? -ne 0; then fail; fi

# test sorting options

cat >input.tcl <<EOF
source "assert.tcl"
DataCube dc
dc.nColAxes 2
dc.nRowAxes 1
dc.separator ","
dc.loadFile input.csv
dc.ravel.handles.@elem 2

puts "sortByAxisLabel 2 1"
dc.ravel.handles(2).sliceLabels.order forward
dc.populateArray data
# output data array in canonical order
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}


puts "sortByAxisLabel 2 -1"
dc.ravel.handles(2).sliceLabels.order reverse
dc.populateArray data
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}

puts "sortBy 2 1 1"
dc.sortBy 2 1 1
dc.populateArray data
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}

puts "sortBy 2 1 -1"
dc.sortBy 2 1 -1
dc.populateArray data
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}

puts "unsortAxis 2"
dc.unsortAxis 2
dc.ravel.handles(2).sliceLabels.order none

dc.populateArray data
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}

EOF

cat >inputNum.tcl <<EOF
source "assert.tcl"
DataCube dc
dc.nColAxes 2
dc.nRowAxes 1
dc.separator ","
dc.loadFile inputNum.csv
dc.ravel.handles.@elem 2

puts "sortByAxisLabel 2 1"
dc.ravel.handles(2).sliceLabels.order forward
dc.populateArray data
# output data array in canonical order
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}


puts "sortByAxisLabel 2 -1"
dc.ravel.handles(2).sliceLabels.order reverse
dc.populateArray data
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}

puts "sortNumericallyByAxisLabel 2 1"
dc.ravel.handles(2).sliceLabels.order numForward
dc.populateArray data
# output data array in canonical order
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}


puts "sortNumericallyByAxisLabel 2 -1"
dc.ravel.handles(2).sliceLabels.order numReverse
dc.populateArray data
foreach key [lsort [array names data]] {
  puts "\$key \$data(\$key)"
}
EOF

cat >expected.dat<<EOF
sortByAxisLabel 2 1
0,1 a
0,2 b
0,3 c
0,4 d
1,0 a
1,1 1
1,2 3
1,3 4
1,4 2
2,0 b
2,1 3
2,2 4
2,3 6
2,4 5
sortByAxisLabel 2 -1
0,1 d
0,2 c
0,3 b
0,4 a
1,0 a
1,1 2
1,2 4
1,3 3
1,4 1
2,0 b
2,1 5
2,2 6
2,3 4
2,4 3
sortBy 2 1 1
0,1 a
0,2 b
0,3 d
0,4 c
1,0 a
1,1 1
1,2 3
1,3 2
1,4 4
2,0 b
2,1 3
2,2 4
2,3 5
2,4 6
sortBy 2 1 -1
0,1 c
0,2 d
0,3 b
0,4 a
1,0 a
1,1 4
1,2 2
1,3 3
1,4 1
2,0 b
2,1 6
2,2 5
2,3 4
2,4 3
unsortAxis 2
0,1 a
0,2 d
0,3 b
0,4 c
1,0 a
1,1 1
1,2 2
1,3 3
1,4 4
2,0 b
2,1 3
2,2 5
2,3 4
2,4 6
EOF

cat >expectedNum.dat<<EOF
sortByAxisLabel 2 1
0,1 100
0,2 1000
0,3 200
0,4 50
1,0 a
1,1 1
1,2 3
1,3 2
1,4 4
2,0 b
2,1 3
2,2 4
2,3 5
2,4 6
sortByAxisLabel 2 -1
0,1 50
0,2 200
0,3 1000
0,4 100
1,0 a
1,1 4
1,2 2
1,3 3
1,4 1
2,0 b
2,1 6
2,2 5
2,3 4
2,4 3
sortNumericallyByAxisLabel 2 1
0,1 50
0,2 100
0,3 200
0,4 1000
1,0 a
1,1 4
1,2 1
1,3 2
1,4 3
2,0 b
2,1 6
2,2 3
2,3 5
2,4 4
sortNumericallyByAxisLabel 2 -1
0,1 1000
0,2 200
0,3 100
0,4 50
1,0 a
1,1 3
1,2 2
1,3 1
1,4 4
2,0 b
2,1 4
2,2 5
2,3 3
2,4 6
EOF

$here/ravelTest input.tcl >expected1.dat
if test $? -ne 0; then fail; fi

diff expected.dat expected1.dat
if test $? -ne 0; then fail; fi

$here/ravelTest inputNum.tcl >expectedNum1.dat
if test $? -ne 0; then fail; fi

diff expectedNum.dat expectedNum1.dat
if test $? -ne 0; then fail; fi



pass
