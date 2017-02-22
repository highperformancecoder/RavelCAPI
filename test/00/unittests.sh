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
ax1,ax2,a,d,b,c
a,a,1,2,3,4
a,b,2 ,5,4,3
b,a,3, 5,4,6
b,b,4,5,6,7
EOF

$here/test/unittests
if test $? -ne 0; then fail; fi

# there should be some PNG files generated
ls *.png&>/dev/null
if test $? -ne 0; then fail; fi

for i in *.png; do
    diff $i $here/test/renderImages/$i
    if test $? -ne 0; then 
        echo "$i differs from reference"
        fail
    fi
    done

pass
