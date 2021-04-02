version=`git describe`
name=ravel-$version

git archive --format=tar.gz --prefix=$name/ HEAD -o linux-distro/$name.tar.gz
