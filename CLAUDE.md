# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

Tangled is a rebranded Chromium fork that bundles a [millix](https://github.com/millix/millix-node) crypto node and wallet UI. This repo does **not** contain Chromium itself. It holds the overlay that turns a stock Chromium checkout into Tangled:

- `patches/`: `git apply` diffs, mirroring Chromium's source tree (`patches/chrome/browser/...` patches `src/chrome/browser/...`).
- `chromium_resources/`: whole new or replacement files copied over `src/` (branding icons, the millix WebUI C++ controllers in `chrome/browser/ui/webui/millix/`, the millix bar view, the custom new-tab page).
- `millix_node_resources/` and `millix_wallet_ui_resources/`: files copied over the matching submodules before they are built (Tangled-specific `index.js`, environment/config).
- Git submodules (JS projects): `millix-node`, `millix-wallet-ui` (React), `tangled-millix-bar-ui` (static HTML/JS for the toolbar bar), `tangled-advertisement`, `tangled-bot`. Most commits here just bump a submodule pointer.

`src/` (the Chromium checkout, managed by `gclient` through `.gclient`), `nodejs/`, `.cipd/` and `applied_patches.log` are gitignored.

## Build

```sh
git submodule init && git submodule update
fetch --nohooks --no-history chromium     # once; creates src/ (needs depot_tools)
sh upgrade_tangled.sh <chromium-tag> <win|linux|darwin> <x64|arm64> <build-dir> <full-upgrade:true|false>
# e.g. sh upgrade_tangled.sh 110.0.5481.38 linux x64 out/Default true
```

`upgrade_tangled.sh` runs these steps in order. Each `setup_*.sh` can also be run alone from the repo root:

1. Only when `full-upgrade=true`: delete `applied_patches.log`, then run `setup_chromium.sh <tag>`. **This runs `git reset --hard` and `git clean -fd` in `src/`**, checks out the tag and runs `gclient sync -D -f`. After that, `setup_nodejs.sh <os>-<arch>` downloads Node v16.18.0 into `nodejs/`, unless it is already there.
2. `setup_tangled_advertisement.sh` and `setup_tangled_bot.sh` run `npm install` and `npm run build`. The output is `lib/*.js`.
3. `setup_millix_node.sh` copies the overlay, runs `npm install` and a webpack prod build into `dist/`.
4. `setup_millix_wallet_ui.sh` copies the overlay and runs `react-app-rewired build` into `build/`.
5. `setup_tangled.sh <os> <arch> <build-dir> <full>` does the following inside `src/`:
   - Only when `full=true`: a global regex rebrand using the `replace-in-file` npm package and `replace_in_file.template`. It changes `chrome://` to `tangled://`, the `"chrome"` URL scheme to `"tangled"`, and "Chromium" to "Tangled" in `.grd`/`.grdp`/`.xtb` strings, plus a few `sed` edits.
   - Applies every `patches/**/*.patch`. Each applied patch is recorded in `applied_patches.log` and skipped on re-runs, so delete the log (or its entry) to re-apply one. A patch missing from the log but already present in `src/` (`git apply --check -R` succeeds) is recorded and skipped instead of failing. The script stops at the first patch that fails.
   - Copies `chromium_resources/*`, the millix bar UI and the wallet UI build into `src/`.
   - Runs `gn gen` (release, ccache, proprietary codecs, widevine), then copies the millix node bundle, the bot and advertisement bundles, and the Node runtime into `src/millix_node/`.
   - Runs `autoninja -C <build-dir> chrome`, plus `installer` (linux) or `mini_installer` (win). On macOS it copies `millix_node` into `Tangled.app/Contents/Resources/`.

To limit the number of parallel compile jobs, set `TANGLED_BUILD_JOBS`, for example `TANGLED_BUILD_JOBS=4 sh upgrade_tangled.sh …`. It is passed to `autoninja` as `-j`. On an 8 GB machine, autoninja's default of 8 jobs fills swap and stalls the build.

All JS builds use the bundled `nodejs/` binary, not the system Node. There are no tests or linters in this repo (`npm test` is a stub). `tangled.iss` is an Inno Setup script for an alternative Windows installer.

## Runtime architecture

- **Node process launch**: `patches/content/browser/browser_main_loop.cc.patch` adds `BrowserMainLoop::CreateMillixNode()` as a startup task. It launches `millix_node/nodejs/bin/node index.dist.js --pid-file ./millix-node.pid` from the executable's directory (`../Resources/millix_node` on macOS), and the matching shutdown code kills it.
- **Child processes**: `millix_node_resources/index.js` (the Tangled entry point for millix-node) spawns `tangled-advertisement.js` and `tangled-bot.js` as child Node processes.
- **Browser UI**:
  - `chromium_resources/chrome/browser/ui/views/millix/millix_bar_view.*` adds a bar to the browser window. The `browser_view*` and `browser_view_layout*` patches wire it in, and it hosts `tangled://millix-bar` (from `tangled-millix-bar-ui`).
  - `tangled://millix` hosts the wallet (`millix-wallet-ui` build, copied to `chrome/browser/resources/millix/app/`).
  - `chrome-untrusted://millix` and `chrome-untrusted://millix-ws` are untrusted WebUIs for the app and the websocket bridge.
  - The URL constants are in the `webui_url_constants.*` patches. Registration is done by the `chrome_web_ui_controller_factory.cc` and `chrome_untrusted_web_ui_configs_desktop.cc` patches.
- **Other patches**: these cover branding and installer metadata, the user agent, extension permission tweaks, disabled update/crash/cleaner pieces, and the polymer tooling changes needed for the custom resources.

## Working on patches

- Patches are diffs against the **rebranded** tree, not against stock Chromium. To regenerate them for a new Chromium version:
  1. Run the rebrand block of `setup_tangled.sh`.
  2. Stage the result with `git add -u` in `src/`. This makes the index the baseline.
  3. Port or fix the patches, then run `git diff -- <path> > ../patches/<path>.patch` from inside `src/`.
  4. Check every patch applies against the baseline: `git checkout -- .`, then `git apply --check` on each.

  Keep one patch per file, at the mirrored path. One exception: `patches/chrome/browser/ui/views/BUILD.gn.patch` targets `ui/views/BUILD.gn`.
- The rebrand only rewrites `"chrome:` (with a double quote) and `chrome://`. A bare single-quoted `'chrome:'` survives it. Two places where that broke the WebUI TypeScript build have explicit patches: the mojom generators (`_CHROME_SCHEME_PREFIX`) and `tools/typescript/path_mappings.py`. If TS builds fail with unresolved `tangled://resources/...` imports, look for more of these.
- The millix bar is laid out by the tabbed-window layout (`frame/layout/browser_view_tabbed_layout_impl.cc`, `views().millix_bar`). It is not laid out in app or popup windows.
- Trusted millix pages are registered as `WebUIConfig`s in `chrome_web_ui_configs.cc`. Untrusted ones are registered in `chrome_untrusted_web_ui_configs.cc`. The configs themselves are defined in `chromium_resources/.../webui/millix/`.
- Tangled replaces `new_tab_page/app.html` (in `chromium_resources/`) with a trimmed copy of Chromium's Lit template. When upgrading, regenerate it from the new upstream `app.html`. Keep every element `app.ts` references through `this.$` (logo, customize buttons, undo toast, OneGoogleBar clip path). Hide those elements rather than deleting them.
- Branding resources in `chromium_resources/` only take effect if their paths match the current Chromium tree. After an upgrade, check for overlay files with no counterpart in stock `src/`. Known locations in 146:
  - **macOS app icon:** comes from the precompiled `chrome/app/theme/chromium/mac/Assets.car` (`CFBundleIconName` is `AppIcon`). `app.icns` is only the fallback. Tangled ships its own `Assets.car`, compiled from `mac/Assets.xcassets/AppIcon.appiconset`. To regenerate it: `xcrun actool Assets.xcassets --compile <out> --platform macosx --minimum-deployment-target 12.0 --app-icon AppIcon --output-partial-info-plist <out>/p.plist`. That command also produces a multi-size `AppIcon.icns` to use as `app.icns`.
  - **Product vector icons:** `components/vector_icons/chromium/product.icon` and `product_refresh.icon`. They used to be in `chrome/app/vector_icons/`.
  - **High-DPI images:** images in `default_200_percent`/`default_300_percent` must be exactly 2× or 3× the 100% size. A 1×-sized file there makes Chromium rescale it, which crashed the browser on 1× displays (`image_skia_rep_default.cc` CHECK).
- The millix bar only becomes visible once `millix_bar.js` receives a session from the node API at `https://localhost:5500`, which uses a self-signed certificate. `chrome_feature_list_creator.cc.patch` appends `--allow-insecure-localhost` directly, because the about:flags entry expired in M130.
- macOS keychain names ("Tangled Safe Storage" / "Tangled") must not change. If they did, existing users would lose their encrypted passwords and cookies.
- To check C++ changes quickly without a full build, use `gn outputs out/<dir> <file.cc>` to get the object file, then run `autoninja -C out/<dir> <obj>`.
- To bump the release version, change `NODE_MILLIX_VERSION` in `millix_node_resources/core/config/environment.js` (e.g. `1.25.6.0-tangled`). Also change the `Tangled/<version>` user-agent suffix in `patches/components/embedder_support/user_agent_utils.cc.patch`. Past commits did not bump `package.json`. `get_versions.sh` lists the release tags.
