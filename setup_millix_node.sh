#!/bin/bash

cd millix-node
cp -r ../millix_node_resources/* .

NODEJS_DIST=$1
NODEJS_DIST_UNIX=("darwin-x64" "darwin-arm64" "linux-x64")

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
    ../nodejs/bin/node ../nodejs/bin/npm install --scripts-prepend-node-path --build-from-source --target_arch=$1 --fallback-to-build
    ../nodejs/bin/node ../nodejs/bin/npm run dist --scripts-prepend-node-path
else
    ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js install --scripts-prepend-node-path --build-from-source --target_arch=$1 --fallback-to-build
    ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js run dist --scripts-prepend-node-path
fi