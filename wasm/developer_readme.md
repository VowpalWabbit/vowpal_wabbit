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

The published tarball must contain the transpiled JavaScript under `dist/` **and** the
emscripten output `dist/vw-wasm.js`. These come from two different builds, which is how
0.0.9 shipped broken: the TypeScript step silently stopped running at publish time and
only `vw-wasm.js` made it into the tarball, so `require('@vowpalwabbit/vowpalwabbit')`
failed for every Node consumer (#4921).

Two guards now make that failure loud instead of silent:

- the `prepare` script runs `tsc` before both `npm pack` and `npm publish`. Do not
  replace it with `prepublish`, which modern npm ignores at publish time -- that is the
  exact regression that broke 0.0.9.
- `npm run verify-package` packs the tarball and fails if anything `package.json` points
  at is missing from it. It runs in CI on every pull request and again from
  `prepublishOnly`, so a publish cannot proceed without it passing.

Steps:

1. Update the version in `package.json`.
2. Build the WASM artifact so `dist/vw-wasm.js` exists and is current:
   ```sh
   emcmake cmake --preset wasm -G Ninja -DCMAKE_BUILD_TYPE=MinSizeRel
   cmake --build build --target vw-wasm
   ```
   Skipping this publishes a stale `vw-wasm.js` -- the version table below will claim a
   VW version the bundle does not actually contain.
3. `npm install` (this runs `prepare`, transpiling `src/*.ts` into `dist/`).
4. Run `npm run docs` and check in the new `documentation.md` if it has changed.
5. Change all version references in `README.md`. Relative links stay broken until the
   change is merged to master and the tag is cut.
6. Add a row to the version table in `README.md` mapping the new npm version to the VW
   version and tag.
7. Verify the tarball before going any further:
   ```sh
   npm run verify-package
   npm pack --dry-run          # eyeball the file list
   ```
   Confirm `dist/vw.js`, `dist/vwnode.js`, `dist/vwbrowser.js` and `dist/vw-wasm.js` are
   all listed. If any are missing, stop -- publishing would break consumers.
8. Commit the changes to master.
9. Tag the release as `wasm_v<major>.<minor>.<patch>` (for example `wasm_v0.0.10`) and
   push the tag.
10. Publish: `npm publish --access public`. You need to be signed in to npm and have
    access to the `vowpalwabbit` organisation.
11. Smoke-test the published package from a clean directory, which is what actually
    catches a bad tarball:
    ```sh
    cd $(mktemp -d) && npm init -y >/dev/null
    npm install @vowpalwabbit/vowpalwabbit@<version>
    node -e "require('@vowpalwabbit/vowpalwabbit'); console.log('ok')"
    ```
