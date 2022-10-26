# getting started

follow this instructions to build the [tangled browser](https://tangled.com) or download it from [tangled.com](https://tangled.com/browser/download.php)

## get the source code

clone the repository and it's submodules

````
    git clone https://github.com/tangled-dev/tangled-browser.git
    git submodule init
    git submodule update
````

## checking out [chromium](https://www.chromium.org/Home/)

follow the instructions to get the chromium source code and configurations for your operating system.

- [build on windows](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/docs/windows_build_instructions.md) 
- [build on linux](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/docs/linux/build_instructions.md) 
- [build on mac](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/docs/mac_build_instructions.md) 

after installing the tools and dependencies to build chromium, run the following command to get the source code

````
    fetch --nohooks --no-history chromium
````

the source code is very large and the download may take 30 minutes in a fast connection, and many hours on slower ones.

### building the tangled browser

the process will build the dependencies

- [millix-node](https://github.com/millix/millix-node)
- [millix-wallet-ui](https://github.com/millix/millix-wallet-ui)
- [tangled-advertisement](https://github.com/tangled-dev/tangled-advertisement)
- [tangled-millix-bar-ui](https://github.com/tangled-dev/tangled-millix-bar-ui)

and apply the patches required to build the tangled browser.

to start the process, you use the script `upgrade_tangled.sh <version> < win | linux | darwin > < x64 | arm64 >`

for example:

````
    sh upgrade_tangled.sh 106.0.5249.119 darwin arm64
````
