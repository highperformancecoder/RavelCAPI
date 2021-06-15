echo "old version=`git describe`"
echo '#define RAVEL_VERSION "'$1'"' >src/ravelVersion.h
echo '#define RAVEL_RELEASE_TIMESTAMP' `date +%s`LL >>src/ravelVersion.h
pushd RavelCAPI
echo 'const char* ravelRelease="'$1'";' >RavelCAPI/ravelRelease.h
git commit -a -m "Release $1"
git tag -a -m "" $1
git push
popd
pushd linux-distro
semver=`echo $1|tr '-' '~'`
sed -e "s/^Version:.*/Version: $semver/" -i ravel.spec
sed -e "s/^Version:.*/Version: $semver-1/" -i ravel.dsc
popd
git commit -a -m "Release $1"
git tag -a -m "" $1
