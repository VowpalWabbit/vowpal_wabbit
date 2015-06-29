#!/usr/bin/env bash

# =============================================================================
#  Constants:
# =============================================================================
__not_darwin=1
__brew_not_installed=2

make_base="cd /vowpal_wabbit;
make clean;
make;"

ubuntu_base="apt-get update -qq;
apt-get install -qq software-properties-common g++ make libboost-all-dev default-jdk;"

ubuntu_12="$ubuntu_base
export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64;
$make_base
mv java/target/vw_jni.lib java/target/vw_jni.Ubuntu.12.amd64.lib"

ubuntu_14_32="$ubuntu_base
export JAVA_HOME=/usr/lib/jvm/java-7-openjdk-i386;
$make_base
mv java/target/vw_jni.lib java/target/vw_jni.Ubuntu.14.i386.lib"

ubuntu_14="$ubuntu_base
export JAVA_HOME=/usr/lib/jvm/java-7-openjdk-amd64;
$make_base
mv java/target/vw_jni.lib java/target/vw_jni.Ubuntu.14.amd64.lib"

red_hat_6="yum update -q -y;
yum install -q -y wget which boost-devel zlib-devel java-1.7.0-openjdk-devel perl;
cd /etc/yum.repos.d;
wget http://people.centos.org/tru/devtools-2/devtools-2.repo;
yum clean all;
yum install -q -y devtoolset-2-gcc devtoolset-2-gcc-c++ devtoolset-2-binutils;
ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/;
export JAVA_HOME=/usr/lib/jvm/java-1.7.0-openjdk.x86_64;
cd /vowpal_wabbit;
make clean;
cat Makefile | sed 's/-fPIC/-fpermissive -fPIC/g' > Makefile.permissive
make -f Makefile.permissive;
rm -f Makefile.permissive
mv java/target/vw_jni.lib java/target/vw_jni.Red_Hat.6.amd64.lib"

red_hat_7="yum install -q -y gcc-c++ make boost-devel zlib-devel java-1.7.0-openjdk-devel perl;
export JAVA_HOME=/usr/lib/jvm/java-1.7.0-openjdk;
$make_base
mv java/target/vw_jni.lib java/target/vw_jni.Red_Hat.7.amd64.lib"

# =============================================================================
#  Function Definitions:
# =============================================================================

# -----------------------------------------------------------------------------
#  Print red text to stderr.
# -----------------------------------------------------------------------------
red() {
  # https://linuxtidbits.wordpress.com/2008/08/11/output-color-on-bash-scripts/
  echo >&2 "$(tput setaf 1)${1}$(tput sgr0)"
}

# -----------------------------------------------------------------------------
#  Print yellow text to stderr.
# -----------------------------------------------------------------------------
yellow() {
  echo >&2 "$(tput setaf 3)${1}$(tput sgr0)"
}

die() { red $2; exit $1; }

# -----------------------------------------------------------------------------
#  Check that the OS is OS X.  If not, die.  If so, check that brew is 
#  installed.  If brew is not installed, ask the user if they want to install.  
#  If so, attempt to install.  After attempting install, check for existence.  
#  If it still doesn't exist, fail.
# -----------------------------------------------------------------------------
check_brew_installed() {
  local os=$(uname)
  if [[ "$os" != "Darwin" ]]; then
    die $__not_darwin "Build script only supported on OS X.  OS=${os}.  Aborting ..."
  else
    if ! brew help 1>/dev/null 2>/dev/null; then 
      red "brew not installed.  To install: Y or N?"
      read should_install
      if [[ "Y" == "${should_install^^}" ]]; then
        ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
      fi
      if ! brew help 1>/dev/null 2>/dev/null; then 
        die $__brew_not_installed "brew not installed.  Aborting ..."
      fi
    fi
  fi
}

install_brew_cask() {
  if ! brew cask 1>/dev/null 2>/dev/null; then 
    yellow "Installing brew-cask..."
    brew install caskroom/cask/brew-cask
  fi
}

install_brew_app() {
  local app=$1
  if ! brew list | grep $app 1>/dev/null; then
    yellow "installing brew app: $app"
    brew install $app
  fi
}

install_cask_app() {
  local app=$1
  if ! brew cask list | grep $app 1>/dev/null; then
    yellow "installing brew cask app: $app"
    brew cask install $app
  fi
}

run_docker() {
  local machine=$1
  local script=$2
  docker run -v $(pwd):/vowpal_wabbit $machine /bin/bash -c "$script"
}

# =============================================================================
#  Main
# =============================================================================

check_brew_installed
install_brew_cask
install_cask_app "virtualbox"
install_brew_app "boot2docker"
install_brew_app "docker"
boot2docker delete
boot2docker init
boot2docker up

# After running boot2docker up this is printed out and it should be the same for everyone
export DOCKER_HOST=tcp://192.168.59.103:2376
export DOCKER_CERT_PATH=~/.boot2docker/certs/boot2docker-vm
export DOCKER_TLS_VERIFY=1

run_docker "ubuntu:12.04" "$ubuntu_12"
run_docker "32bit/ubuntu:14.04" "$ubuntu_14_32"
run_docker "ubuntu:14.04" "$ubuntu_14"
run_docker "centos:6" "$red_hat_6"
run_docker "centos:7" "$red_hat_7"

make clean
make
mv java/target/vw_jni.lib java/target/vw_jni.$(uname -s).$(uname -m).lib
