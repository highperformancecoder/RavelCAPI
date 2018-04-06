r=0
for i in test/00/*; do
    echo $i
    if sh $i; then
        true
    else
        let $[r++];
    fi
done
exit $r
