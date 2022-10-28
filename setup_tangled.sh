#!/bin/bash

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

replace_in_file() {
    cp ../replace_in_file.template options.js;
    npx replace-in-file "#FROM" "$1"  options.js
    npx replace-in-file "#TO" "$2"  options.js
    npx replace-in-file "#FILES" "$3"  options.js
    npx replace-in-file --configFile=options.js  --verbose
}

if [[ "$OSTYPE" == "darwin"* ]]; then
  SEDOPTION="-i ''"
else
  SEDOPTION="-i"
fi

cd src

echo "replace chrome: with tangled: in the following folders"
echo "android_webview ash base build chrome chromecast chromeos components content device docs extensions fuchsia_web gin google_apis gpu headless ios media mojo native_client_sdk net pdf ppapi sandbox services skia sql storage styleguide testing third_party/blink third_party/closure_compiler third_party/wpt_tools tools ui url weblayer "

for folder in android_webview ash base build chrome chromecast chromeos components content device docs extensions fuchsia_web gin google_apis gpu headless ios media mojo native_client_sdk net pdf ppapi sandbox services skia sql storage styleguide testing third_party/blink third_party/closure_compiler third_party/wpt_tools tools ui url weblayer; do
    echo "processing folder $folder"
    replace_in_file "/\"chrome:/gi" "'\"tangled:'" "'$folder/**'"
    replace_in_file "/chrome:\/\//gi" "'tangled://'" "'$folder/**'"
done

echo "activate tangled: schema"

for folder in chrome chromeos components content ios; do
    echo "processing folder $folder"
    replace_in_file "/Scheme\[\] = \"chrome\"/g" "'Scheme[] = \"tangled\"'" "['$folder/**/*.cc','$folder/**/*.h']"
done


echo "rebranding: rename app"

sed $SEDOPTION 's/PRODUCT_STRING "Chromium"/PRODUCT_STRING "Tangled"/g' chrome/common/chrome_constants.cc
sed $SEDOPTION "s/'Chromium/'Tangled/g" tools/mb/mb.py

echo "regranding: update texts"
for folder in chrome chromeos components content ios; do
    echo "processing folder $folder"
    replace_in_file "/[C]hromium/g" "'Tangled'" "['$folder/**/*.grdp','$folder/**/*.grd','$folder/**/*.xtb']" --verbose
done

echo "regranding: fix chrominum link"
sed $SEDOPTION 's/Tangled<ph name="END_LINK_CHROMIUM"/Chromium<ph name="END_LINK_CHROMIUM"/g' components/components_chromium_strings.grd
replace_in_file "/[C]hromium/g" "'Tangled'" "['$folder/**/*.grdp','$folder/**/*.grd','$folder/**/*.xtb']" --verbose

echo "regranding: fix default data folder"
sed $SEDOPTION 's/"Chromium"/"Tangled"/g' chrome/browser/mac/initial_prefs.mm
sed $SEDOPTION 's/"Chromium"/"Tangled"/g' chrome/common/chrome_paths_mac.mm

echo "apply tangled patches"
LC_ALL=C find ../patches -not -path '*/.*' -type f -name "*.patch" | while read fpatch; do
    git_apply $fpatch
done

cp -r ../chromium_resources/* .
cp -r ../tangled-millix-bar-ui/* chrome/browser/resources/millix/
cp -r ../millix-wallet-ui/build/* chrome/browser/resources/millix/app/

gn gen out/Default --args="cc_wrapper=\"ccache\" target_cpu = \"$2\" is_debug = false ffmpeg_branding = \"Chrome\" proprietary_codecs = true enable_widevine = true"

echo "copy millix node to the tangled browser app"
rm -rf millix_node
mkdir -p millix_node
cp -r ../millix-node/{scripts,package.json} millix_node
cp -r ../millix-node/dist/* millix_node
cp -r ../tangled-advertisement/lib/tangled-advertisement.js millix_node
cp -r ../nodejs millix_node

if [[ "$1" == "darwin" ]]; then
    echo "build tangled for macos"
    autoninja -C out/Default chrome
    cp -r millix_node out/Default/Tangled.app/Contents/Resources/millix_node
elif [[ "$1" == "linux" ]]; then
    echo "build tangled for linux"
    autoninja -C out/Default chrome
    autoninja -C out/Default installer
elif [[ "$1" == "win" ]]; then
    echo "build tangled for windows"
    autoninja -C out/Default chrome
    autoninja -C out/Default mini_installer
fi