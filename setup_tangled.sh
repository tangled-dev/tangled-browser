#!/bin/sh

git_apply() {
    echo "appling patch $1"

    patch_file=../applied_patches.log

    if [ -f $patch_file ]; then
        grep -Fxq "$1" $patch_file
        if [ $? -eq 0 ]; then
            echo "patch already applied"
            return
        fi
    fi

    git apply -v $1
    if [ $? -ne 0 ]; then
        echo "git apply failed"
        exit $?
    fi

    echo "$1" >> ../applied_patches.log 
}

cd src

echo "replace chrome: with tangled: in the following folders"
echo "android_webview ash base build chrome chromecast chromeos components content device docs extensions fuchsia_web gin google_apis gpu headless ios media mojo native_client_sdk net pdf ppapi sandbox services skia sql storage styleguide testing third_party/blink third_party/closure_compiler third_party/wpt_tools tools ui url weblayer \n"

for folder in android_webview ash base build chrome chromecast chromeos components content device docs extensions fuchsia_web gin google_apis gpu headless ios media mojo native_client_sdk net pdf ppapi sandbox services skia sql storage styleguide testing third_party/blink third_party/closure_compiler third_party/wpt_tools tools ui url weblayer; do
    echo "processing folder $folder"
    LC_ALL=C find $folder -not -path '*/.*' -type f -exec sed -i '' 's/"chrome:/"tangled:/gi' {} \; -exec sed -i '' 's/chrome:\/\//tangled:\/\//gi' {} \;
done

echo "\nactivate tangled: schema\n"

for folder in chrome chromeos components content ios; do
    echo "processing folder $folder"
    LC_ALL=C find $folder -not -path '*/.*' -type f \( -name "*.cc" -or -name "*.h" \) -exec sed -i '' 's/Scheme\[\] = "chrome"/Scheme\[\] = "tangled"/g' {} \;
done


echo "rebranding: rename app"

sed -i '' 's/PRODUCT_STRING "Chromium"/PRODUCT_STRING "Tangled"/g' chrome/common/chrome_constants.cc
sed -i '' "s/'Chromium/'Tangled/g" tools/mb/mb.py

echo "regranding: update texts"
for folder in chrome chromeos components content ios; do
    echo "processing folder $folder"
    LC_ALL=C find $folder -not -path '*/.*' -type f \( -name "*.grdp" -or -name "*.grd" -or -name "*.xtb" \) -exec sed -i '' "s/Chromium/Tangled/g" {} \;
done

echo "regranding: fix chrominum link"
sed -i '' 's/Tangled<ph name="END_LINK_CHROMIUM"/Chromium<ph name="END_LINK_CHROMIUM"/g' components/components_chromium_strings.grd
LC_ALL=C find components/strings -not -path '*/.*' -type f \( -name "*.grdp" -or -name "*.grd" -or -name "*.xtb" \) -exec sed -i '' "s/Chromium/Tangled/g" {} \;

echo "regranding: fix default data folder"
sed -i '' 's/"Chromium"/"Tangled"/g' chrome/browser/mac/initial_prefs.mm
sed -i '' 's/"Chromium"/"Tangled"/g' chrome/common/chrome_paths_mac.mm

echo "apply tangled patches"
LC_ALL=C find ../patches -not -path '*/.*' -type f -name "*.patch" | while read fpatch; do
    git_apply $fpatch
done

echo "apply tangled patches"
LC_ALL=C find ../patches -not -path '*/.*' -type f -name "*.patch" | while read fpatch; do
    git_apply $fpatch
done

cp -r ../chromium_resources/* .
cp -r ../tangled-millix-bar-ui/* chrome/browser/resources/millix/
cp -r ../millix-wallet-ui/build/* chrome/browser/resources/millix/app/

gn gen out/Default --args="cc_wrapper=\"ccache\" target_cpu = \"$2\" is_debug = false ffmpeg_branding = \"Chrome\" proprietary_codecs = true enable_widevine = true"

if [[ "$1" == "darwin" ]]; then
    echo "build tangled for macos"
    autoninja -C out/Default chrome
    MILLIX_NODE_FOLDER=out/Default/Tangled.app/Contents/Resources/millix_node
    rm -rf $MILLIX_NODE_FOLDER
    mkdir -p $MILLIX_NODE_FOLDER
elif [[ "$1" == "linux" ]]; then
    echo "build tangled for linux"
elif [[ "$1" == "win" ]]; then
    echo "build tangled for windows"
fi

echo "copy millix node to the tangled browser app"
cp -r ../millix-node/{scripts,package.json} $MILLIX_NODE_FOLDER
cp -r ../millix-node/dist/* $MILLIX_NODE_FOLDER
cp -r ../tangled-advertisement/lib/tangled-advertisement.js $MILLIX_NODE_FOLDER
cp -r ../nodejs $MILLIX_NODE_FOLDER