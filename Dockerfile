FROM ubuntu:16.04 AS build

# Install add-apt-repository
RUN apt-get update && \
  apt-get install -y software-properties-common python-software-properties

# Add Oracle JDK repo (including license agreement)
RUN echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | \
  debconf-set-selections && \
  add-apt-repository -y ppa:webupd8team/java && \
  apt-get update

# Install build tools
RUN apt-get install -y \
	gcc g++ libboost-all-dev zlib1g-dev help2man make libssl-dev cmake doxygen graphviz python-setuptools python-dev build-essential \
	maven oracle-java8-installer \
	wget git vim netcat pkg-config && \
	rm -rf /var/lib/apt/lists/* && \
  rm -rf /var/cache/oracle-jdk8-installer

# Install cpprestsdk
RUN git clone https://github.com/Microsoft/cpprestsdk.git casablanca && \
  cd casablanca/Release && \
  # Checkout 2.10.1 version of cpprestsdk
  git checkout e8dda215426172cd348e4d6d455141f40768bf47 && \
  mkdir build && \
  cd build && \
  cmake .. && \
  make && \
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
RUN wget https://raw.githubusercontent.com/JohnLangford/vowpal_wabbit/master/java/pom.xml && \
  mvn dependency:resolve -f pom.xml && \
  rm pom.xml

# Cleanup
RUN apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/{apt,dpkg,cache,log}

# Set environment variables used by build
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/local/lib"
ENV PATH="${HOME}/miniconda/bin:${PATH}"
ENV JAVA_HOME="/usr/lib/jvm/java-8-oracle"
