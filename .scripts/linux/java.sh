#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Run Java build and test
mvn clean test -f java/pom.xml

# publish snapshot jar to staging repository
if [ "$ossrh_username" = "\$(ossrh_username)" ] || [ -z "$ossrh_username" ]
then
	echo "Skipping package publishing"
else
	# template for username/password for sonatype repository server
	cp java/settings.xml ~/.m2/settings.xml

	# import signing key
	# this is how to export them
	# gpg --export-secret-keys 'Markus Cozowicz <marcozo@microsoft.com>' | base64 -w 0
	echo $ossrh_gpg | base64 -d | gpg --import -

	MAVEN_OPTS="-Dossrh.username=$ossrh_username -Dossrh.password=$ossrh_password"

	# to use the snapshot from oss.sonatype.org 
	# * add http://oss.sonatype.org/content/repositories/snapshots 
	# * reference com.github.vowpalwabbit:vw-jni:8.7.0-SNAPSHORT
	# 
	# more details at https://stackoverflow.com/questions/7715321/how-to-download-snapshot-version-from-maven-snapshot-repository

	# For a proper release:
	# * remove -SNAPSHOT in pom.xml
	# * visit https://oss.sonatype.org/#stagingRepositories and "close & release" the staged .jar
	# * see https://oss.sonatype.org/#stagingRepositories
	mvn -f java/pom.xml verify gpg:sign deploy:deploy -Dmaven.test.skip=true $MAVEN_OPTS
fi