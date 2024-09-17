#!/bin/bash

cd tangled-bot

NODEJS_DIST=$1
NODEJS_DIST_UNIX=("darwin" "linux")

rm -rf node_modules

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
  ../nodejs/bin/node ../nodejs/bin/npm install --scripts-prepend-node-path --build-from-source --target_arch=$2 --fallback-to-build
  ../nodejs/bin/node ../nodejs/bin/npm run build --scripts-prepend-node-path
else
  ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js install --scripts-prepend-node-path --build-from-source --target_arch=$2 --fallback-to-build
  ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js run build --scripts-prepend-node-path
fi