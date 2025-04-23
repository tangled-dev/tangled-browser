#!/bin/bash

cd millix-node
cp -r ../millix_node_resources/* .

NODEJS_DIST=$1
NODEJS_DIST_UNIX=("darwin" "linux")

rm -rf node_modules

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
    ../nodejs/bin/node ../nodejs/bin/npm install --scripts-prepend-node-path --build-from-source --arch=$2 --target_arch=$2 --fallback-to-build
    BABEL_ENV=production ../nodejs/bin/npx webpack --config webpack.prod.config.js
else
    ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js install --scripts-prepend-node-path --build-from-source --arch=$2 --target_arch=$2 --fallback-to-build
    export BABEL_ENV=production && ../nodejs/npx webpack --config webpack.prod.config.js
fi