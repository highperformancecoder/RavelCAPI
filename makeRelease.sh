echo "old version=`git describe`"
echo '#define RAVEL_VERSION "'$1'"' >src/ravelVersion.h
git commit -a -m "Release $1"
git tag -a -m "" $1

