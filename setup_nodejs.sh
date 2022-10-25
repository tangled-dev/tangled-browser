#!/bin/bash

if [ -d "./nodejs" ]; then
    echo "using cached nodejs"
    exit
fi

NODEJS_DIST=$1
NODEJS_DIST_UNIX=("darwin-x64" "darwin-arm64" "linux-x64")

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
    NODEJS_FILE="node-v16.18.0-$1.tar.gz"
else
    NODEJS_FILE="node-v16.18.0-$1.zip"
fi

NODE_URL="https://nodejs.org/dist/v16.18.0/$NODEJS_FILE"
echo "downloading node v16.18.0 for $NODE_URL"
curl -o $NODEJS_FILE $NODE_URL

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
    tar -xvf $NODEJS_FILE
else
    unzip $NODEJS_FILE
fi

mv "node-v16.18.0-$1" nodejs
rm -rf $NODEJS_FILE

if [[ " ${NODEJS_DIST_UNIX[@]} " =~ " ${NODEJS_DIST} " ]]; then
    chmod +x ./nodejs/bin/node
fi