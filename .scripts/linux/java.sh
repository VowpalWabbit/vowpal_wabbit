#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Run Java build and test
mvn clean test -f java/pom.xml

if [ ! -z "$ossrh_username" ]
then
	# template for username/password for sonatype repository server
	cp java/settings.xml ~/.m2/settings.xml

	# import signing key
	# this is how to export them
	# gpg --export-secret-keys 'Markus Cozowicz <marcozo@microsoft.com>' | base64 -w 0
	echo $ossrh_gpg | base64 -d | gpg --import -

	MAVEN_OPTS="-Dossrh.username=$ossrh_username -Dossrh.password=$ossrh_password"
	# mvn -f java/pom.xml release:prepare $MAVEN_OPTS
	mvn -f java/pom.xml verify gpg:sign deploy:deploy -Dmaven.test.skip=true $MAVEN_OPTS
fi