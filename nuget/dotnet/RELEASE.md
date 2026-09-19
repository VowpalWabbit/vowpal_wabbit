# Publishing the .NET packages to nuget.org

Publishing is automatic. Pushing a release tag (`9.11.4`, or a prerelease such as
`9.11.5-rc1`) runs `.github/workflows/dotnet_nugets.yml`, which builds the packages,
tests them, and then publishes them from the `publish_nuget` job. No manual upload step
and no long-lived API key.

This document exists because the previous process was "manually upload the artifacts",
which is how the packages ended up stranded at 9.3.0 while the project released many
versions past it.

## What gets published

Five packages, all from this one workflow:

| Package | Notes |
|---|---|
| `VowpalWabbit` | the combined package, built on Windows |
| `VowpalWabbit.runtime.win-x64` | platform-specific native runtime |
| `VowpalWabbit.runtime.linux-x64` | " |
| `VowpalWabbit.runtime.osx-x64` | " |
| `VowpalWabbit.runtime.osx-arm64` | " |

The `VowpalWabbit` package depends on the matching `runtime` packages, so all five must
publish together. The workflow refuses to start if it cannot find all five, and fails the
job if any single push fails, precisely to avoid leaving a `VowpalWabbit` on nuget.org
whose dependencies do not exist.

Two other packages, `VowpalWabbit.JSON` and `VowpalWabbit.Parallel`, exist on nuget.org
but are **not** built by this workflow. They are still at 9.3.0 and this automation does
not change that. If they should be released again, that is separate work.

## How authentication works

The workflow uses [NuGet trusted publishing](https://learn.microsoft.com/en-us/nuget/nuget-org/trusted-publishing)
rather than a stored API key. The job requests a GitHub OIDC token, `NuGet/login` exchanges
it with nuget.org for an API key valid for **one hour and a single use**, and the push uses
that. Nothing long-lived is stored, so there is nothing to rotate or leak.

nuget.org validates the token against a trusted publishing policy, which must exist:

- **Package owner**: the nuget.org account that owns the `VowpalWabbit*` packages
- **Repository owner**: `VowpalWabbit`
- **Repository**: `vowpal_wabbit`
- **Workflow file**: `dotnet_nugets.yml` (file name only, no `.github/workflows/` prefix)
- **Environment**: empty (the job does not use a GitHub environment)
- **Scopes**: Push, *"Push new packages and package versions"*
- **Glob**: `VowpalWabbit*`

The scope must allow **new packages**, not only new versions: the four
`VowpalWabbit.runtime.*` IDs have never been published, so their first push creates them.
With the narrower "push only new package versions" scope those four fail with an
authorization error while the combined package succeeds -- the exact partial-publish this
setup is built to prevent.

Manage the policy at nuget.org under your username, Trusted Publishing.

The one repository secret is `NUGET_USER`: the nuget.org **profile name** that owns the
policy, not an email address. It is not a credential -- the profile name is public on every
package page -- but it is stored as a secret to match NuGet's documented example. The job
fails with an explanatory error if it is unset, rather than silently skipping the publish.

## Cutting a release

1. Follow the [release checklist](../../.github/ISSUE_TEMPLATE/release_checklist.md).
2. Push the tag. That is the whole publishing step.
3. Watch the `Publish to nuget.org` job in the tag's workflow run.
4. Confirm the packages are live. The job's last step checks this, but nuget.org indexes
   asynchronously so it reports rather than enforces:
   ```sh
   curl -s https://api.nuget.org/v3/registration5-semver1/vowpalwabbit/index.json | jq -r '.items[-1].upper'
   ```

## When it fails

**`NUGET_USER is not set`** -- add the repository secret. Until then no tag publishes.

**403 / "The specified API key is invalid"** from `NuGet/login`** -- the trusted publishing
policy does not match this run. Check the repository owner, repository name and workflow
file name on the policy. The policy binds to the workflow *file name*, so renaming
`dotnet_nugets.yml`, or moving the publish job into a different workflow file, breaks it
until the policy is updated.

**403 on some packages only** -- almost certainly the scope is "push only new package
versions" while a new package ID is being created. Widen the scope, or publish that ID
once by hand.

**Policy shows inactive** -- policies for private repositories start temporarily active for
7 days and lapse if nothing publishes. `vowpal_wabbit` is public, so its policy captured the
repository and owner IDs at creation and is permanently active; if this ever changes, check
that the policy owner is still an active member of any organization that owns it.

**The tag built but nothing published** -- the publish job depends on the build and both
test jobs. If any of those failed, publishing is skipped by design. Fix the failure and
re-run the workflow for that tag; `--skip-duplicate` makes re-running safe for packages
that already published.
