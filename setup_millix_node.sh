#!/bin/sh
cd millix-node
cp -r ../millix_node_resources/* .
../nodejs/bin/node ../nodejs/bin/npm install --scripts-prepend-node-path --build-from-source --target_arch=$1 --fallback-to-build
../nodejs/bin/node ../nodejs/bin/npm run dist --scripts-prepend-node-path