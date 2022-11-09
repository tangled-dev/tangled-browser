#!/bin/bash

cd millix-wallet-ui
cp -r ../millix_wallet_ui_resources/* .

NODEJS_DIST=$1
NODEJS_DIST_UNIX=("darwin" "linux")

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
  ../nodejs/bin/node ../nodejs/bin/npm install --scripts-prepend-node-path
  CI=false ../nodejs/bin/npx react-app-rewired build
else
  ../nodejs/node ../nodejs/node_modules/npm/bin/npm-cli.js install --scripts-prepend-node-path
  export CI=false && ../nodejs/npx react-app-rewired build
fi