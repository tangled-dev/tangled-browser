cd src

echo "reset git changes"
git reset --hard

echo "clean git files"
git clean -fd

echo "checkout git code $1"
git fetch origin refs/tags/$1:refs/tags/$1
git checkout $1

echo "sync code dependencies"
gclient sync -D -f