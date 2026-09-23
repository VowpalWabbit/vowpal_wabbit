#!/usr/bin/env bash
#
# Post-release steps for the two destinations that are not driven by the release tag.
#
# Everything else publishes from CI when the tag is pushed: nuget.org, PyPI and Maven
# Central from the release tag, npm from its own wasm_v* tag, Homebrew and conda-forge from
# bots. vcpkg and Docker live in other repositories, so they cannot be driven by this
# repository's workflows without storing a cross-repository token -- which is exactly the
# kind of stored credential the trusted-publishing setup exists to avoid. Running them from
# a maintainer's machine uses that maintainer's own `gh` auth instead.
#
# Usage:
#   utl/post-release.sh <version>            # show what would happen, change nothing
#   utl/post-release.sh <version> --yes      # actually dispatch and open the PR
#
# Requires: gh (authenticated), git, curl, sha512sum (or shasum on macOS).

set -euo pipefail

VERSION="${1:-}"
APPLY="${2:-}"

if [[ -z "$VERSION" ]]; then
  echo "Usage: $0 <version> [--yes]" >&2
  echo "  e.g. $0 9.11.6 --yes" >&2
  exit 1
fi

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.-]+)?$ ]]; then
  echo "Refusing a version that is not semver-shaped: $VERSION" >&2
  exit 1
fi

DRY_RUN=1
[[ "$APPLY" == "--yes" ]] && DRY_RUN=0

UPSTREAM="VowpalWabbit/vowpal_wabbit"
DOCKER_REPO="VowpalWabbit/docker-images"
VCPKG_REPO="microsoft/vcpkg"

say()  { printf '\n\033[1m==> %s\033[0m\n' "$*"; }
note() { printf '    %s\n' "$*"; }
run()  {
  if [[ "$DRY_RUN" -eq 1 ]]; then
    printf '    [dry run] %s\n' "$*"
  else
    "$@"
  fi
}

sha512_of() {
  if command -v sha512sum >/dev/null 2>&1; then sha512sum "$1" | cut -d' ' -f1
  else shasum -a 512 "$1" | cut -d' ' -f1
  fi
}

# ---------------------------------------------------------------------------
# Preflight: the tag has to exist upstream, because both steps build from it.
# ---------------------------------------------------------------------------
say "Checking the release tag exists"
if ! git ls-remote --tags "https://github.com/${UPSTREAM}.git" "refs/tags/${VERSION}" \
     | grep -q "refs/tags/${VERSION}"; then
  echo "Tag ${VERSION} does not exist on ${UPSTREAM}." >&2
  echo "Push the release tag first; both steps below build from it." >&2
  exit 1
fi
note "tag ${VERSION} found"

if [[ "$DRY_RUN" -eq 1 ]]; then
  say "DRY RUN -- nothing will be dispatched, pushed or opened. Re-run with --yes to apply."
fi

# ---------------------------------------------------------------------------
# Docker: workflow dispatch in the docker-images repository.
# ---------------------------------------------------------------------------
say "Docker images"
note "dispatching build_deploy_release.yml in ${DOCKER_REPO} for tag ${VERSION}"
run gh workflow run build_deploy_release.yml \
      --repo "$DOCKER_REPO" \
      --field build_tag="$VERSION"
if [[ "$DRY_RUN" -eq 0 ]]; then
  note "watch: https://github.com/${DOCKER_REPO}/actions/workflows/build_deploy_release.yml"
fi
note "the non-release images are cut by creating a release in ${DOCKER_REPO}; that is a"
note "judgement call about which images to refresh, so it is deliberately left manual."

# ---------------------------------------------------------------------------
# vcpkg: a pull request against the port in microsoft/vcpkg.
# ---------------------------------------------------------------------------
say "vcpkg port"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

note "hashing the source archive vcpkg will download"
ARCHIVE="${WORK}/vw-${VERSION}.tar.gz"
curl -sSL --fail --max-time 300 \
  "https://github.com/${UPSTREAM}/archive/${VERSION}.tar.gz" -o "$ARCHIVE"
NEW_SHA512="$(sha512_of "$ARCHIVE")"
note "sha512: ${NEW_SHA512}"

# Clone upstream, not the fork. A fork that has not been synced for a while still carries
# the *old* port, and a branch cut from it produces a pull request that reverts whatever
# landed in between -- versions/baseline.json conflicts on the same lines, and the diff
# looks like it is redoing someone else's update. Upstream is the only safe base.
#
# The push to the fork then makes every upstream commit since the fork last synced newly
# reachable in the fork, and some of those touch .github/workflows. GitHub refuses that
# from an OAuth token without the `workflow` scope -- as a plain 422 on `gh repo sync`,
# and (less helpfully) as a bare 404 on ref creation. `gh auth status` shows the scopes.
note "checking gh has the scopes this needs"
if [[ "$DRY_RUN" -eq 0 ]]; then
  if ! gh auth status 2>&1 | grep -q "Token scopes:.*'workflow'"; then
    echo "" >&2
    echo "gh is authenticated without the 'workflow' scope." >&2
    echo "Pushing a branch to your vcpkg fork also pushes the upstream commits the fork is" >&2
    echo "missing, some of which touch .github/workflows, and GitHub rejects that." >&2
    echo "" >&2
    echo "  gh auth refresh -h github.com -s workflow" >&2
    echo "" >&2
    echo "then re-run this script." >&2
    exit 1
  fi
fi

note "forking and cloning ${VCPKG_REPO} (shallow)"
if [[ "$DRY_RUN" -eq 0 ]]; then
  gh repo fork "$VCPKG_REPO" --clone=false >/dev/null 2>&1 || true
  VCPKG_FORK="$(gh api /user --jq .login)/vcpkg"
  gh repo sync "$VCPKG_FORK" --source "$VCPKG_REPO" >/dev/null
  git clone -q --depth 1 "https://github.com/${VCPKG_REPO}.git" "${WORK}/vcpkg"
  cd "${WORK}/vcpkg"
  git remote add fork "https://github.com/${VCPKG_FORK}.git"
  git checkout -q -b "vowpal-wabbit-${VERSION}"
else
  note "[dry run] would sync the fork, clone ${VCPKG_REPO} and branch vowpal-wabbit-${VERSION}"
fi

PORT="ports/vowpal-wabbit"

if [[ "$DRY_RUN" -eq 0 ]]; then
  note "updating ${PORT}/vcpkg.json and portfile.cmake"
  python3 - "$VERSION" <<'PY'
import json, sys, collections
version = sys.argv[1]
p = "ports/vowpal-wabbit/vcpkg.json"
with open(p) as fh:
    d = json.load(fh, object_pairs_hook=collections.OrderedDict)
d["version"] = version
# port-version counts rebuilds of the *same* upstream version, so it resets on a bump.
d.pop("port-version", None)
with open(p, "w") as fh:
    json.dump(d, fh, indent=2)
    fh.write("\n")
PY
  # Replace only the SHA512 line, leaving REF alone: it is "${VERSION}" and follows vcpkg.json.
  sed -i -E "s|^(\s*SHA512 )[0-9a-f]+|\1${NEW_SHA512}|" "${PORT}/portfile.cmake"

  # The patches were written against an older release. A patch that no longer applies fails
  # the port build, and that is not something this script should paper over.
  #
  # The usual cause is that upstream has since made the same change, so the patch is
  # obsolete rather than wrong: fix-fmt-header.patch stopped applying at 9.11.6 because
  # 5f3aecba8 ("fix: compatibility with fmt 12.2.0") landed those exact edits in the source
  # tree. The fix in that case is to delete the patch file and drop it from the PATCHES list
  # in portfile.cmake -- but deciding that is a judgement call, so stop here.
  note "checking the port's patches still apply to ${VERSION}"
  SRC="${WORK}/src"; mkdir -p "$SRC"
  tar -xzf "$ARCHIVE" -C "$SRC" --strip-components=1
  patch_problem=0
  for patch in "${PORT}"/*.patch; do
    [[ -e "$patch" ]] || continue
    if git -C "$SRC" apply --check "$(cd "$(dirname "$patch")" && pwd)/$(basename "$patch")" 2>/dev/null; then
      note "  applies: $(basename "$patch")"
    else
      note "  DOES NOT APPLY: $(basename "$patch")"
      patch_problem=1
    fi
  done
  if [[ "$patch_problem" -ne 0 ]]; then
    echo "" >&2
    echo "One or more port patches no longer apply to ${VERSION}." >&2
    echo "The branch is prepared at ${WORK}/vcpkg but nothing has been pushed." >&2
    echo "Fix or drop the patches there, then run x-add-version and open the PR by hand." >&2
    trap - EXIT   # keep the working tree for inspection
    exit 1
  fi

  note "bootstrapping vcpkg so x-add-version can run (slow)"
  ./bootstrap-vcpkg.sh -disableMetrics >/dev/null
  git add -A "$PORT"
  git -c user.name="$(git -C "$OLDPWD" config user.name || echo VowpalWabbit)" \
      -c user.email="$(git -C "$OLDPWD" config user.email || echo noreply@github.com)" \
      commit -q -m "[vowpal-wabbit] update to ${VERSION}"
  ./vcpkg x-add-version vowpal-wabbit >/dev/null
  git add -A versions
  git -c user.name="$(git -C "$OLDPWD" config user.name || echo VowpalWabbit)" \
      -c user.email="$(git -C "$OLDPWD" config user.email || echo noreply@github.com)" \
      commit -q -m "[vowpal-wabbit] x-add-version"

  note "pushing and opening the pull request"
  git push -q fork "vowpal-wabbit-${VERSION}"
  gh pr create --repo "$VCPKG_REPO" \
    --base master --head "$(gh api /user --jq .login):vowpal-wabbit-${VERSION}" \
    --title "[vowpal-wabbit] Update to ${VERSION}" \
    --body "Updates the vowpal-wabbit port to ${VERSION}.

Source: https://github.com/${UPSTREAM}/releases/tag/${VERSION}

- version bumped, \`port-version\` reset
- SHA512 updated for the ${VERSION} source archive
- existing port patches confirmed to still apply against the ${VERSION} source
- \`x-add-version\` run in a separate commit"
else
  note "[dry run] would set version=${VERSION}, reset port-version, update SHA512,"
  note "[dry run] verify the port patches still apply, run x-add-version, and open a PR"
fi

say "Done"
if [[ "$DRY_RUN" -eq 1 ]]; then
  note "This was a dry run. Re-run with --yes to apply."
else
  note "Check the dispatched Docker build and the vcpkg pull request."
fi
