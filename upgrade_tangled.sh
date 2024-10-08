#!/bin/bash

if [[ $5 == "true" ]]; then
    echo "remove applied patch log"
    rm -rf applied_patches.log

    echo "working on chromium"
    bash setup_chromium.sh $1

    echo "working on millix"
    bash setup_nodejs.sh "$2-$3"
fi

echo "working on tangled advertisement"
bash setup_tangled_advertisement.sh $2 $3

echo "working on tangled bot"
bash setup_tangled_bot.sh $2 $3

echo "working on millix node"
bash setup_millix_node.sh $2 $3

echo "working on millix wallet ui"
bash setup_millix_wallet_ui.sh $2

echo "working on tangled"
bash setup_tangled.sh $2 $3 $4 $5