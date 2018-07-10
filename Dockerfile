FROM ubuntu:16.04 AS build

# install add-apt-repository
RUN apt-get update && apt-get install -y software-properties-common python-software-properties

# add Oracle JDK repo (including licence agreement)
RUN echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | debconf-set-selections && add-apt-repository -y ppa:webupd8team/java && apt-get update

# install build tools
RUN apt-get install -y \
	libboost-all-dev zlib1g-dev libgtest-dev google-mock \
    g++ make libssl-dev cmake libcpprest-dev rapidjson-dev doxygen-latex graphviz \
	python-setuptools python-dev build-essential \
	maven oracle-java8-installer \
	wget git vim netcat pkg-config && \
	rm -rf /var/lib/apt/lists/* && \
  	rm -rf /var/cache/oracle-jdk8-installer

# install python tools
RUN easy_install pip && \
    pip install cpp-coveralls wheel virtualenv pytest readme_renderer && \
    wget https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh -O miniconda.sh && \
    bash miniconda.sh -b -p $HOME/miniconda && \
    hash -r && \
    $HOME/miniconda/bin/conda config --set always_yes yes --set changeps1 no && \
    $HOME/miniconda/bin/conda update -q conda && \
    $HOME/miniconda/bin/conda create -q -n test-python27 python=2.7 nomkl numpy scipy scikit-learn pytest

# download maven dependencies
RUN wget https://raw.githubusercontent.com/JohnLangford/vowpal_wabbit/master/java/pom.xml && mvn dependency:resolve -f pom.xml && rm pom.xml

RUN cd /usr/src/gtest && cmake CMakeLists.txt && make && cp *.a /usr/lib && \
    cd /usr/src/gmock && cmake CMakeLists.txt && make && cp *.a /usr/lib

# cleanup
RUN apt-get clean autoclean && \
    apt-get autoremove -y && \
    rm -rf /var/lib/{apt,dpkg,cache,log}
