#!/usr/bin/env node
// Verifies that the tarball `npm publish` would upload actually contains every file
// package.json points consumers at.
//
// 0.0.9 shipped without dist/vw.js, dist/vwnode.js and dist/vwbrowser.js, so
// `require('@vowpalwabbit/vowpalwabbit')` failed with MODULE_NOT_FOUND for every Node
// consumer (issue #4921). The transpile step had silently stopped running at publish
// time, and nothing checked the result, so the breakage was only discovered downstream.
//
// This runs `npm pack --dry-run --json`, which executes the same prepare/prepack
// lifecycle as a real publish, and fails if any declared entry point is absent from the
// file list.

const { execFileSync } = require('child_process');
const path = require('path');

const pkgRoot = path.resolve(__dirname, '..');
const pkg = require(path.join(pkgRoot, 'package.json'));

// Collect every path package.json advertises: main, types, and each leaf of exports.
function collectDeclared(pkgJson) {
  const declared = new Set();
  const add = (v) => {
    if (typeof v === 'string' && v.startsWith('.')) {
      declared.add(path.posix.normalize(v.replace(/^\.\//, '')));
    }
  };
  add(pkgJson.main ? './' + pkgJson.main : undefined);
  add(pkgJson.types ? './' + pkgJson.types : undefined);

  const walk = (node) => {
    if (typeof node === 'string') { add(node); return; }
    if (node && typeof node === 'object') { Object.values(node).forEach(walk); }
  };
  walk(pkgJson.exports);

  // package.json itself is always present; not worth asserting.
  declared.delete('package.json');
  return [...declared];
}

const declared = collectDeclared(pkg);
if (declared.length === 0) {
  console.error('verify-package: package.json declares no entry points -- nothing to verify.');
  process.exit(1);
}

let packed;
try {
  const out = execFileSync('npm', ['pack', '--dry-run', '--json'], {
    cwd: pkgRoot, encoding: 'utf8', stdio: ['ignore', 'pipe', 'inherit'],
  });
  packed = JSON.parse(out);
} catch (err) {
  console.error('verify-package: `npm pack --dry-run --json` failed.');
  process.exit(1);
}

const entry = Array.isArray(packed) ? packed[0] : packed;
const shipped = new Set((entry.files || []).map((f) => path.posix.normalize(f.path)));

const missing = declared.filter((f) => !shipped.has(f));

if (missing.length > 0) {
  console.error('verify-package: the package tarball is missing files that package.json points at:');
  missing.forEach((f) => console.error('  missing: ' + f));
  console.error('');
  console.error('Publishing this would break consumers at require/import time (see issue #4921).');
  console.error('Usually this means the TypeScript build did not run -- check that the "prepare"');
  console.error('script is intact, and that dist/vw-wasm.js was produced by the cmake/emscripten');
  console.error('build and copied into wasm/ before packing.');
  process.exit(1);
}

console.log(
  'verify-package: OK -- all ' + declared.length + ' declared entry point(s) present in the tarball:');
declared.sort().forEach((f) => console.log('  ' + f));
