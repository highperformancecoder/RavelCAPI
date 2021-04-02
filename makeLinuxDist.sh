version=`git describe`
name=ravel-$version
target=`pwd`/linux-distro/$name.tar

rm linux-distro/*.tar.gz
git archive --format=tar --prefix=$name/ HEAD -o $target

git submodule update --init --recursive
git submodule | cut -f3 -d' '| while read module; do
    pushd $module
    git archive --format=tar --prefix=$name/$module/ HEAD -o /tmp/$$.tar
    tar tvf /tmp/$$.tar
    tar Avf $target /tmp/$$.tar
    popd
done

gzip $target

cd linux-distro

for d in xUbuntu_18.04; do
    osc build $d
done
