#!/bin/bash

echo "\nworking on chromium\n"
#bash setup_chromium.sh $1

echo "\nworking on millix\n"
bash setup_nodejs.sh "$2-$3"

echo "\nworking on tangled advertisement\n"
#bash setup_tangled_advertisement.sh

echo "\nworking on millix node\n"
#bash setup_millix_node.sh $3

echo "\nworking on millix wallet ui\n"
#bash setup_millix_wallet_ui.sh

echo "\nworking on tangled\n"
#bash setup_tangled.sh $2 $3