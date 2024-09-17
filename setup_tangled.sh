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

if [[ $4 == "true" ]]; then
    echo "replace chrome: with tangled: in the following folders"
    echo "base build chrome chromecast chromeos components content device docs extensions fuchsia_web gin google_apis gpu headless ios media mojo native_client_sdk net pdf ppapi sandbox services skia sql storage styleguide testing third_party/blink third_party/closure_compiler third_party/wpt_tools tools ui url weblayer "

    for folder in base build chrome components content extensions media mojo skia sandbox tools ui url; do
        echo "processing folder $folder"
        replace_in_file "/\"chrome:/gi" "'\"tangled:'" "'$folder/**'"
        replace_in_file "/chrome:\/\//gi" "'tangled://'" "'$folder/**'"
    done

    echo "activate tangled: schema"

    for folder in chrome components content; do
        echo "processing folder $folder"
        replace_in_file "/Scheme\[\] = \"chrome\"/g" "'Scheme[] = \"tangled\"'" "['$folder/**/*.cc','$folder/**/*.h']"
    done


    echo "rebranding: rename app"

    sed $SEDOPTION 's/PRODUCT_STRING "Chromium"/PRODUCT_STRING "Tangled"/g' chrome/common/chrome_constants.cc
    sed $SEDOPTION "s/'Chromium/'Tangled/g" tools/mb/mb.py

    echo "regranding: update texts"
    for folder in chrome components content; do
        echo "processing folder $folder"
        replace_in_file "/[C]hromium/g" "'Tangled'" "['$folder/**/*.grdp','$folder/**/*.grd','$folder/**/*.xtb']" --verbose
    done

    echo "regranding: fix chrominum link"
    sed $SEDOPTION 's/Tangled<ph name="END_LINK_CHROMIUM"/Chromium<ph name="END_LINK_CHROMIUM"/g' components/components_chromium_strings.grd
    replace_in_file "/[C]hromium/g" "'Tangled'" "['$folder/**/*.grdp','$folder/**/*.grd','$folder/**/*.xtb']" --verbose

    echo "regranding: fix default data folder"
    sed $SEDOPTION 's/"Chromium"/"Tangled"/g' chrome/browser/mac/initial_prefs.mm
    sed $SEDOPTION 's/"Chromium"/"Tangled"/g' chrome/common/chrome_paths_mac.mm
fi    

echo "apply tangled patches"
LC_ALL=C find ../patches -not -path '*/.*' -type f -name "*.patch" | while read fpatch; do
    git_apply $fpatch
done

cp -p -r ../chromium_resources/* .
cp -p -r ../tangled-millix-bar-ui/* chrome/browser/resources/millix/
cp -p -r ../millix-wallet-ui/build/* chrome/browser/resources/millix/app/

BUILD_FOLDER=$3
gn gen $BUILD_FOLDER --args="cc_wrapper=\"ccache\" target_cpu = \"$2\" is_debug = false dcheck_always_on = false is_component_build = false ffmpeg_branding = \"Chrome\" proprietary_codecs = true enable_widevine = true"

echo "copy millix node to the tangled browser app"
rm -rf millix_node
mkdir -p millix_node
cp -r ../millix-node/{scripts,package.json} millix_node
cp -r ../millix-node/dist/* millix_node
cp -r ../tangled-advertisement/lib/tangled-advertisement.js millix_node
cp -r ../tangled-bot/lib/tangled-bot.js millix_node
cp -r ../nodejs millix_node

if [[ "$1" == "darwin" ]]; then
    echo "build tangled for macos"
    autoninja -C $BUILD_FOLDER chrome
    rm -rf $BUILD_FOLDER/Tangled.app/Contents/Resources/millix_node
    cp -r millix_node $BUILD_FOLDER/Tangled.app/Contents/Resources/millix_node
elif [[ "$1" == "linux" ]]; then
    echo "build tangled for linux"
    autoninja -C $BUILD_FOLDER chrome
    autoninja -C $BUILD_FOLDER installer
elif [[ "$1" == "win" ]]; then
    echo "build tangled for windows"
    autoninja -C $BUILD_FOLDER chrome
    autoninja -C $BUILD_FOLDER mini_installer
fi