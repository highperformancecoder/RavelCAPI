echo "old version=`git describe`"
echo '#define RAVEL_VERSION "'$1'"' >src/ravelVersion.h
pushd RavelCAPI
git tag -a -m "" $1
git push
popd
git commit -a -m "Release $1"
git tag -a -m "" $1
