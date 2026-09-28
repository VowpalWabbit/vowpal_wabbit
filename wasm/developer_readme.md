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
This was done for 0.0.10; what follows is what actually worked, not what seemed like it
should.

Do not build locally unless you already have emscripten set up. The WASM half needs it,
and the documented Docker route needs a running daemon. **Take CI's build instead**, which
is also what makes the version table honest: the binary then comes from the commit CI
built, not from whatever your machine produced.

1. Find a green `wasm.yml` run for the commit you are shipping and download its artifact:

   ```sh
   gh run list --workflow=wasm.yml --branch master --limit 3
   gh run download <run-id> --name wasm-npm-package --dir ./wasmpkg
   ```

2. **That artifact is not publishable as it stands.** It is uploaded before the package is
   built, so it carries `dist/vw-wasm.js` from emscripten and *none* of the TypeScript
   output -- no `dist/vwnode.js`, which `package.json` names as `main`. Publishing it with
   `--ignore-scripts` reproduces the 0.0.9 bug exactly. The `publish_npm` job regenerates
   that half with its own "Install and build" step, and so must you:

   ```sh
   cd wasmpkg
   npm install          # runs `prepare`, which runs tsc
   ```

   Node 20 or newer. CI builds on 20 and publishes on 22; the `tsc` and dependency set do
   not work on older runtimes.

3. Verify, and do not skip this:

   ```sh
   npm run verify-package
   ```

   It should list every declared entry point. `dist/vwnode.js` missing here is the 0.0.9
   failure caught before publishing rather than after.

4. Prove the tarball works before it is public, not after:

   ```sh
   npm pack
   cd "$(mktemp -d)" && npm init -y
   npm install /path/to/vowpalwabbit-vowpalwabbit-<version>.tgz
   node -e "require('@vowpalwabbit/vowpalwabbit').then(vw => {
     const m = new vw.Workspace({args_str: '--quiet'});
     m.learn(m.parse('1 | a:.5')); console.log(m.predict(m.parse('| a:.5'))); m.delete();
   })"
   ```

   The package resolves to a promise; the exports are empty until the WASM finishes
   loading, so a bare `Object.keys(require(...))` tells you nothing.

5. Publish. The registry requires 2FA for this package, so a plain `npm publish` after
   `npm login` fails with:

   ```
   403 ... Two-factor authentication or granular access token with bypass 2fa
   enabled is required to publish packages
   ```

   Either pass a one-time code, or use a granular access token created on npmjs.com with
   **read and write** on `@vowpalwabbit/vowpalwabbit` and **bypass 2FA** ticked:

   ```sh
   npm publish --access public --otp=<code>
   # or, with a token, keeping it out of ~/.npmrc and out of the tarball:
   umask 077
   printf '//registry.npmjs.org/:_authToken=%s\n' "$TOKEN" > /tmp/npmrc-publish
   npm publish --access public --userconfig /tmp/npmrc-publish
   shred -u /tmp/npmrc-publish
   ```

   A token that authenticates but lacks the bypass fails the same way as no token. One
   that is expired or revoked gives `401` from `npm whoami`, so check that first rather
   than reading a publish failure as a permissions problem.

6. Confirm against the registry, not against your tarball. Allow a few minutes for the
   metadata to update, then repeat step 4 installing `@vowpalwabbit/vowpalwabbit@<version>`
   from npm. 0.0.9 published successfully and was broken for every consumer; a green
   publish is not evidence of anything.

7. Tag `wasm_v<version>` at the commit CI built, so the repository records what shipped.
   The tag's `publish_npm` job will fail while the trusted publisher is unregistered. That
   failure is accurate and worth leaving visible.

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
