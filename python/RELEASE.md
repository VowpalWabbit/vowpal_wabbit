# Publishing the Python package to PyPI

Publishing is automatic. Pushing a release tag (`9.11.4`, or a prerelease such as
`9.11.5-rc1`) runs `.github/workflows/python_wheels.yml`, which builds every wheel and the
source distribution, then publishes them from the `publish_pypi` job. There is no manual
download-and-upload step and no API token stored in this repository.

## Why this replaced the old process

The previous documented process was: download artifacts with a third-party tool, **build
the macOS ARM wheels by hand on a dedicated Apple M1 Mac**, check each wheel, upload to
test PyPI, verify, then upload for real.

The macOS ARM step has not been necessary since GitHub added Apple Silicon runners; the
`cibuildwheel.macos-14` job already builds those wheels on every push. The rest was manual
effort with no automation, which is how 9.11.3 came to be version-bumped and merged without
ever reaching PyPI -- leaving its security fixes unavailable to anyone installing with pip.

## What gets published

| Artifact | Built by |
|---|---|
| Linux x86_64 wheels | `cibuildwheel.ubuntu-latest` |
| Linux aarch64 wheels | `cibuildwheel.ubuntu-24.04-arm` |
| macOS arm64 wheels | `cibuildwheel.macos-14` |
| macOS x86_64 wheels | `cibuildwheel.macos-15-intel` |
| Windows wheels | `cibuildwheel.windows-2022` |
| Source distribution | `ubuntu-latest.amd64.py3.10.sdist-bundle` |

All of them publish together. The job refuses to proceed if the sdist is missing or if
noticeably fewer wheels than expected are present: a release missing a platform's wheels
does not fail loudly, it silently makes pip fall back to building from source on that
platform, which for this project means a long compile or an outright failure for the user.

## How authentication works

The workflow uses [PyPI trusted publishing](https://docs.pypi.org/trusted-publishers/)
rather than a stored API token. The job requests a GitHub OIDC token, PyPI validates it
against a trusted publisher registered for the project and returns a short-lived token used
for the upload. Nothing long-lived is stored, so there is nothing to rotate or leak.

The trusted publisher must be registered once, by a project **Owner**, at
<https://pypi.org/manage/project/vowpalwabbit/settings/publishing/>:

- **Owner**: `VowpalWabbit`
- **Repository name**: `vowpal_wabbit`
- **Workflow name**: `python_wheels.yml`
- **Environment**: leave empty (the job does not use a GitHub environment)

The publisher binds to the workflow *file name*, so renaming `python_wheels.yml` or moving
the publish job into another workflow breaks publishing until it is updated.

Once registered, nobody needs PyPI credentials to cut a release. That is the point: the
project has seven PyPI maintainers and still missed a release, because publishing depended
on a specific person finding time to do it by hand.

## Cutting a release

1. Follow the [release checklist](../.github/ISSUE_TEMPLATE/release_checklist.md).
2. Push the tag. That is the whole publishing step.
3. Watch the `Publish to PyPI` job in the tag's workflow run.

The job verifies the tag matches the built version, so a tag cannot publish a version
nobody intended, and afterwards installs the published package from PyPI into a clean
virtualenv and imports it. A release that uploads successfully but is not installable
fails the job.

## When it fails

**`invalid-publisher` or a 403 from the upload step** -- the trusted publisher is not
registered, or does not match this run. Check the owner, repository name and workflow file
name at the settings link above.

**Tag/version mismatch** -- the tag and the built artifacts disagree. Usually means the tag
was pushed before `version.txt` was updated, or at the wrong commit.

**Too few wheels** -- one of the `cibuildwheel` jobs failed, so the matrix is incomplete.
Publishing is refused rather than shipping a release that is missing platforms. Fix the
failing job and re-run the workflow for that tag.

**File already exists** -- that version was already published. PyPI does not allow
re-uploading a version, even after deletion; cut a new patch version instead.

**The tag built but nothing published** -- the publish job depends on every wheel job and
the sdist jobs. If any failed, publishing is skipped by design.
