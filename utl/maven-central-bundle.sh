#!/bin/bash
# Build a Maven Central bundle from GitHub Actions artifacts for manual upload
# to the Sonatype Central Portal (https://central.sonatype.com/).
#
# Prerequisites:
#   - gh CLI authenticated with access to VowpalWabbit/vowpal_wabbit
#   - GPG key for signing (publish public key to keyserver.ubuntu.com)
#   - Sonatype Central Portal account with com.github.vowpalwabbit namespace
#
# Usage:
#   ./maven-central-bundle.sh <github-run-id> <gpg-key-id>
#
# Example:
#   ./maven-central-bundle.sh 22606152692 D1C2EB4D5076861A
#
# After running, upload bundle.zip via Central Portal -> Publish Component.

set -euo pipefail

if [ $# -ne 2 ]; then
    echo "Usage: $0 <github-run-id> <gpg-key-id>"
    echo "  github-run-id: ID of the 'Java Publish' workflow run"
    echo "  gpg-key-id:    GPG key ID for signing artifacts"
    exit 1
fi

RUN_ID="$1"
GPG_KEY="$2"
REPO="VowpalWabbit/vowpal_wabbit"

WORK_DIR=$(mktemp -d)
trap 'rm -rf "$WORK_DIR"' EXIT
cd "$WORK_DIR"

echo "==> Finding JARs artifact from run $RUN_ID..."
ARTIFACT_ID=$(gh api "repos/$REPO/actions/runs/$RUN_ID/artifacts" \
    --jq '.artifacts[] | select(.name == "vw-jni-jars") | .id')

if [ -z "$ARTIFACT_ID" ]; then
    echo "ERROR: No vw-jni-jars artifact found in run $RUN_ID"
    exit 1
fi

echo "==> Downloading artifact $ARTIFACT_ID..."
gh api "repos/$REPO/actions/artifacts/$ARTIFACT_ID/zip" > jars.zip
unzip -q jars.zip
rm jars.zip

# Detect version from JAR filename
VERSION=$(ls vw-jni-*.jar | grep -v sources | grep -v javadoc | sed 's/vw-jni-\(.*\)\.jar/\1/')
echo "==> Detected version: $VERSION"

# Extract POM from JAR
echo "==> Extracting POM..."
unzip -q -o "vw-jni-$VERSION.jar" META-INF/maven/com.github.vowpalwabbit/vw-jni/pom.xml
cp META-INF/maven/com.github.vowpalwabbit/vw-jni/pom.xml "vw-jni-$VERSION.pom"
rm -rf META-INF

# Verify all required files exist
for f in "vw-jni-$VERSION.pom" "vw-jni-$VERSION.jar" "vw-jni-$VERSION-javadoc.jar" "vw-jni-$VERSION-sources.jar"; do
    if [ ! -f "$f" ]; then
        echo "ERROR: Missing required file: $f"
        exit 1
    fi
done

# Generate GPG signatures
echo "==> Signing artifacts with key $GPG_KEY..."
for f in "vw-jni-$VERSION.pom" "vw-jni-$VERSION.jar" "vw-jni-$VERSION-javadoc.jar" "vw-jni-$VERSION-sources.jar"; do
    gpg --batch --yes --pinentry-mode loopback --passphrase '' \
        --default-key "$GPG_KEY" --armor --detach-sign "$f"
done

# Generate MD5 and SHA1 checksums
echo "==> Generating checksums..."
for f in "vw-jni-$VERSION.pom" "vw-jni-$VERSION.jar" "vw-jni-$VERSION-javadoc.jar" "vw-jni-$VERSION-sources.jar"; do
    md5sum "$f" | awk '{print $1}' > "$f.md5"
    sha1sum "$f" | awk '{print $1}' > "$f.sha1"
done

# Build bundle ZIP with Maven directory layout
echo "==> Building bundle..."
MAVEN_DIR="com/github/vowpalwabbit/vw-jni/$VERSION"
mkdir -p "bundle/$MAVEN_DIR"
cp vw-jni-"$VERSION".* "bundle/$MAVEN_DIR/"
cd bundle
zip -r ../bundle.zip .
cd ..

# Copy bundle to caller's directory
cp bundle.zip "$OLDPWD/bundle.zip"

echo ""
echo "==> Bundle created: bundle.zip"
echo "    Contents:"
unzip -l bundle.zip | grep -v "^Archive\|^  Length\|^-\|^$"
echo ""
echo "Upload via: Central Portal -> Publish Component -> select bundle.zip"
