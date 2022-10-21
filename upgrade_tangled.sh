echo "\nworking on chromium\n"
sh setup_chromium.sh $1

echo "\nworking on millix\n"
sh setup_nodejs.sh "$2-$3"

echo "\nworking on tangled advertisement\n"
sh setup_tangled_advertisement.sh

echo "\nworking on millix node\n"
sh setup_millix_node.sh $3

echo "\nworking on millix wallet ui\n"
sh setup_millix_wallet_ui.sh

echo "\nworking on tangled\n"
sh setup_tangled.sh $2 $3