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

Publishing is automatic. Pushing a `wasm_v<version>` tag runs `.github/workflows/wasm.yml`,
which builds the WASM artifact, transpiles the TypeScript, verifies the tarball and
publishes to npm. There is no manual `npm publish` step and no npm token to store.

> **Blocked as of 9.11.7.** The automatic path needs a trusted publisher registered on
> npmjs.com, and registering one needs admin rights on the `@vowpalwabbit` scope, which
> nobody currently reachable has. See [publishing by hand](#publishing-by-hand-while-the-scope-is-blocked)
> below, and delete that section once the registration exists.

#### Why the guards exist

The tarball needs output from **two different builds**: `dist/vw-wasm.js` from emscripten
via cmake, and `dist/*.js` from `tsc`. 0.0.9 shipped with only the first, because the
TypeScript step silently stopped running at publish time, so
`require('@vowpalwabbit/vowpalwabbit')` failed for every Node consumer (#4921).

Three things now prevent a repeat, and none of them need a human:

- the `prepare` script runs `tsc` before both `npm pack` and `npm publish`. Do not replace
  it with `prepublish`, which modern npm ignores at publish time -- that is the exact
  regression that broke 0.0.9.
- `npm run verify-package` packs the tarball and fails if anything `package.json` points at
  is missing. It runs in CI on every pull request and again before publishing.
- after publishing, the workflow installs the published package from the registry in a
  clean directory and requires it. A publish that "succeeds" but produces a broken package
  fails the job.

#### Authentication

The workflow uses [npm trusted publishing](https://docs.npmjs.com/trusted-publishers).
npm detects the GitHub OIDC environment and exchanges a short-lived token; there is no
`NODE_AUTH_TOKEN`. Because the repository and package are both public, npm also generates
provenance attestations automatically.

The trusted publisher must be registered once, on npmjs.com under the package's Settings:

- **Organization or user**: `VowpalWabbit`
- **Repository**: `vowpal_wabbit`
- **Workflow filename**: `wasm.yml`
- **Environment**: leave empty (the job does not use a GitHub environment)

The publisher binds to the workflow *file name*, so renaming `wasm.yml` or moving the
publish job into another workflow breaks publishing until it is updated.

Trusted publishing requires npm >= 11.5.1 and Node >= 22.14.0. Node 22 ships an older npm,
so the workflow upgrades npm explicitly -- do not remove that step.

#### Cutting a release

1. Update `version` in `package.json`.
2. Run `npm run docs` and check in `documentation.md` if it changed.
3. Update version references in `README.md`, and add a row to the version table mapping the
   new npm version to the VW version and tag.
4. Commit to master.
5. Tag `wasm_v<version>` (for example `wasm_v0.0.10`) and push the tag. That is the whole
   publishing step.
6. Watch the `Publish to npm` job in the tag's workflow run.

The version in the tag must match `package.json`; the workflow checks this and fails
otherwise, so a tag cannot publish a version nobody intended.

#### Publishing by hand while the scope is blocked

Until the trusted publisher can be registered, releases go out with a local `npm publish`.
The risk this reintroduces is precisely the one that broke 0.0.9: publishing whatever
happens to be sitting in `dist/`. Do not skip step 3.

1. Check out the tag you intend to ship, clean. Not a dirty working tree, and not master
   if master has moved on:

   ```sh
   git checkout wasm_v<version>
   git status --porcelain   # must be empty
   ```

2. Build both halves from that commit, following the build instructions above. `dist/`
   must end up with `vw-wasm.js` from emscripten *and* the `tsc` output.

3. Verify the tarball before pushing anything anywhere:

   ```sh
   npm run verify-package
   ```

   This is the same check CI runs. It packs the tarball and fails if anything
   `package.json` points at is missing.

4. Publish:

   ```sh
   npm publish --access public --provenance=false
   ```

   Provenance attestation is off because it is generated from the CI OIDC environment and
   is not available locally.

5. Verify what the registry actually serves, from an empty directory:

   ```sh
   cd "$(mktemp -d)"
   npm install @vowpalwabbit/vowpalwabbit@<version>
   node -e "require('@vowpalwabbit/vowpalwabbit'); console.log('ok')"
   ```

   0.0.9 published successfully and was broken for every consumer. A green `npm publish`
   is not evidence that the package works.

Steps 3 and 5 are what the workflow does around the publish. Doing one without the other
is how the last manual release failed.

#### The version table is a claim about the WASM binary

The table in `README.md` maps each npm version to a VW version. That is only true if
`dist/vw-wasm.js` was built from a commit containing that VW code. The workflow builds it
from the tagged commit, so tag the commit you actually want to ship. Publishing from an
artifact built elsewhere can ship a stale binary under a version number claiming otherwise.

#### When it fails

**Tag/version mismatch** -- the tag says one version and `package.json` another. Fix
whichever is wrong; delete and re-push the tag if needed.

**`npm error need auth` or a 401/403 on publish** -- the trusted publisher is not
registered, or does not match this run. Check the organization, repository and workflow
filename on npmjs.com.

**`verify-package` reports missing files** -- the TypeScript build did not run, or
`dist/vw-wasm.js` is absent from the build artifact. Publishing is refused; this is the
guard working.

**The tag built but nothing published** -- the publish job depends on the build job. If the
build or tests failed, publishing is skipped by design.
