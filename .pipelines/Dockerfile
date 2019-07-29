FROM ubuntu:14.04 AS build

# Upgrade cmake to 3.2
RUN apt-get update && apt-get install -y software-properties-common python-software-properties debconf-utils
RUN add-apt-repository -y ppa:george-edison55/cmake-3.x
RUN apt-get update

# Add Open JDK repo (including license agreement), install Java
RUN echo "oracle-java11-installer shared/accepted-oracle-license-v1-2 select true" | \
  debconf-set-selections && \
  add-apt-repository -y ppa:openjdk-r/ppa && \
  apt-get update && \
  apt install -y openjdk-11-jdk

# Install build tools
RUN apt-get install -y \
    gcc g++ libboost-all-dev zlib1g-dev help2man make libssl-dev cmake doxygen graphviz \
    python-setuptools python-dev build-essential maven wget git && \
  rm -rf /var/lib/apt/lists/*

# ppa for g++ 4.9 (first version that supports complete c++11. i.e. <regex>)
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN apt-get install -y gcc-4.9 g++-4.9
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.9
RUN add-apt-repository -y --remove "ubuntu-toolchain-r-test"

# Install cpprestsdk
RUN git clone https://github.com/Microsoft/cpprestsdk.git casablanca && \
  cd casablanca/Release && \
  # Checkout 2.10.1 version of cpprestsdk
  git checkout e8dda215426172cd348e4d6d455141f40768bf47 && \
  mkdir build && \
  cd build && \
  cmake .. -DBUILD_TESTS=Off -DBUILD_SAMPLES=Off && \
  make -j 4 && \
  make install && \
  cd ../../../ && \
  rm -rf casablanca

# Install python tools
RUN easy_install pip && \
  pip install cpp-coveralls wheel virtualenv pytest readme_renderer pandas && \
  wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh && \
  bash miniconda.sh -b -p $HOME/miniconda && \
  hash -r && \
  $HOME/miniconda/bin/conda config --set always_yes yes --set changeps1 no && \
  $HOME/miniconda/bin/conda update -q conda && \
  $HOME/miniconda/bin/conda create -q -n test-python27 python=2.7 nomkl numpy scipy scikit-learn

# Download maven dependencies
RUN wget https://raw.githubusercontent.com/VowpalWabbit/vowpal_wabbit/master/java/pom.xml.in && \
  mvn dependency:resolve -f pom.xml.in && \
  rm pom.xml.in

# Cleanup
RUN apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/{apt,dpkg,cache,log}

# Set environment variables used by build
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"
ENV JAVA_HOME="/usr/lib/jvm/java-11-openjdk-amd64/"

# Download clang-format 7.0
RUN wget http://releases.llvm.org/7.0.1/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-14.04.tar.xz && \
	tar xvf clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-14.04.tar.xz && \
	mv clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-14.04/bin/clang-format /usr/local/bin && \
	rm -rf clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-14.04/
