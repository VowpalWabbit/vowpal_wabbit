# Publishing vw-jni to Maven Central

Publishing is automatic. Pushing a release tag runs `.github/workflows/java-publish.yml`,
which builds the JNI library for all five platforms, assembles the multi-platform JAR, and
deploys it to Maven Central through the Sonatype Central Portal.

## Why this is the one pipeline with stored secrets

NuGet, npm and PyPI all publish with OIDC trusted publishing: no credentials are stored
anywhere. **Maven Central has no equivalent.** Publishing needs a Portal user token and a
GPG signing key, both held as repository secrets.

That is a real difference in risk. Anyone able to change a workflow on `master` can sign
artifacts as this project. Use a **dedicated release-only signing key**, not a personal one,
so it can be revoked without affecting anything else.

The alternative is not "publish manually" — it is "do not publish". The JAR bundles native
libraries for five platforms, which only CI builds, so a manual deployment means downloading
five artifacts, placing them by hand, and running a signed `mvn deploy` locally. That is
what the process degenerated into, and Maven Central sat at 9.9.0 as a result.

## Required secrets

| Secret | What |
|---|---|
| `CENTRAL_TOKEN_USERNAME` | username half of a Central Portal user token |
| `CENTRAL_TOKEN_PASSWORD` | password half of the same token |
| `GPG_PRIVATE_KEY` | ASCII-armoured private key used to sign artifacts |
| `GPG_PASSPHRASE` | passphrase for that key |

The job fails with an explicit list of what is missing rather than skipping, so a tag cannot
quietly fail to publish.

### Portal token

Generate at <https://central.sonatype.com/usertoken> and copy both halves immediately; the
token cannot be retrieved after the dialog closes. It has a configurable expiry — whatever
you choose becomes a date on which publishing starts failing, so record it.

### Signing key

Generate a key that is used for nothing else:

```sh
gpg --quick-generate-key "Vowpal Wabbit Release Signing <jl@hunch.net>" rsa4096 sign 2y
gpg --list-secret-keys --keyid-format=long      # note the key id

# Central verifies signatures against a public keyserver, so the public half must be there:
gpg --keyserver keyserver.ubuntu.com --send-keys <KEY_ID>

# The value for the GPG_PRIVATE_KEY secret:
gpg --armor --export-secret-keys <KEY_ID>
```

Key continuity does not matter: Central does not require the same key across releases, so
whoever held the pre-2025 OSSRH signing key is irrelevant. A fresh key is fine.

## Cutting a release

1. Ensure the version in `version.txt` is not a `-SNAPSHOT`; the job refuses to publish one,
   since Central would otherwise silently take it as a snapshot deployment.
2. Push the tag. That is the whole publishing step.
3. Watch the `Publish to Maven Central` job.
4. Artifacts appear at
   <https://central.sonatype.com/search?smo=true&q=vw-jni>. Central validates asynchronously,
   so allow a few minutes.

## When it fails

**`Maven Central publishing is not configured`** — one or more secrets are unset; the error
names which.

**401 or 403 from the deploy** — the Portal token is wrong or expired. Note the credentials
are the *user token*, not the Sonatype account login.

**`gpg: signing failed: No secret key`** — `GPG_PRIVATE_KEY` is not the ASCII-armoured
private key, or `GPG_PASSPHRASE` does not match it.

**Central rejects the deployment as unsigned** — the `release-sign-artifacts` profile did not
activate. It is keyed on `-DperformRelease=true`, which the deploy step passes; if that flag
is dropped the build succeeds locally and is rejected on upload.

**Missing native library for `<platform>`** — one of the `build-native` matrix jobs failed,
so the JAR would ship without that platform. Publishing is refused rather than shipping a
JAR that fails at runtime on the missing platform.

**Tag does not match `version.txt`** — the tag was pushed at the wrong commit, or before the
version bump merged.

## History

Before 9.11.6 this pointed at OSSRH (`oss.sonatype.org`) via `nexus-staging-maven-plugin`.
OSSRH reached end of life on 2025-06-30 and was shut down, so that path stopped working
entirely — not merely stopped being automated. #4958 migrated the pom to the Central Portal;
this workflow makes the deployment happen on a tag.
