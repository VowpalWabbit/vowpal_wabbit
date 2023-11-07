# VW - WASM

This module is currently very experimental. It is not currently built standalone and still requires the glue JS to function. Ideally it will be standalone WASM module eventually.

## Use docker container
### Build
```sh
# Run in VW root directory
docker run --rm -v $(pwd):/src -it emscripten/emsdk emcmake cmake -G "Unix Makefiles" --preset wasm -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_TOOLCHAIN_FILE=/src/ext_libs/vcpkg/scripts/buildsystems/vcpkg.cmake
docker run --rm -v $(pwd):/src -it emscripten/emsdk emcmake cmake --build /src/build --target vw-wasm -j $(nproc)
```

Artifacts are placed in `wasm/out`

### Test
Assumes required build artifacts are in `wasm/out`

```sh
# Run in VW root directory
docker run --workdir="/src/wasm" --rm -v $(pwd):/src -t node:16-alpine npm install
docker run --workdir="/src/wasm" --rm -v $(pwd):/src -t node:16-alpine npm test
```

## Or, setup local environment to build

### Install Emscripten and activate environment
Instructions here: https://emscripten.org/docs/getting_started/downloads.html
```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build
Make sure Emscripten is activated.
```sh
emcmake cmake --preset wasm -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_TOOLCHAIN_FILE=$(pwd)/ext_libs/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --target vw-wasm
npm run build
```

### Test
```sh
npm test
```

### Build docs
``` sh
npm run docs
```

### Release on npm

1. Update the version in package.json
2. Run `npm run docs` and check in the new `documentation.md` if it has changed
3. Change all version references in README.md (relative links will be broken until merged to master and the tag is cut)
4. Update the table in README.md to point to latest VW version and tag
5. Commit changes to master
6. Tag the release as `wasm_v.major.minor.patch`
7. Run `cmake` to pick up any new `vw-wasm.js` changes
8. Publish to npm `npm publish --access public` (you need to sign into your npm account first and have access to the vowpalwabbit organisation)