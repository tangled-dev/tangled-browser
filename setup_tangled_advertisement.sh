#!/bin/bash

cd tangled-advertisement

NODEJS_DIST=$1
NODEJS_DIST_UNIX=("darwin" "linux")

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
  ../nodejs/bin/node ../nodejs/bin/npm install --scripts-prepend-node-path
  ../nodejs/bin/node ../nodejs/bin/npm run build --scripts-prepend-node-path
else
  ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js install --scripts-prepend-node-path
  ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js run build --scripts-prepend-node-path
fi