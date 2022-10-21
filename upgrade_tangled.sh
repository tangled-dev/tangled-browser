#!/bin/bash

echo "working on chromium"
bash setup_chromium.sh $1

echo "working on millix"
bash setup_nodejs.sh "$2-$3"

echo "working on tangled advertisement"
bash setup_tangled_advertisement.sh

echo "working on millix node"
bash setup_millix_node.sh $3

echo "working on millix wallet ui"
bash setup_millix_wallet_ui.sh

echo "working on tangled"
bash setup_tangled.sh $2 $3