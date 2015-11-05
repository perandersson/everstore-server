FROM ubuntu

##
## 1. Install basic ubuntu tools needed to build everstore
## 2. Download and install CMake 3.x
## 3. Download and build everstore
## 4. Remove unnecessary stuff (such as the compiler)
## 5. Copy files to appropriate directories
##

RUN apt-get update && \
    apt-get install -y wget && \
    apt-get install -y unzip && \
    apt-get install -y gcc g++ build-essential && \
    \
    wget https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.tar.gz && \
    tar -C /opt -xf cmake-3.3.2-Linux-x86_64.tar.gz && \
    rm cmake-3.3.2-Linux-x86_64.tar.gz && \
    ln -s /opt/cmake-3.3.2-Linux-x86_64/bin/cmake /usr/bin/cmake && \
    ln -s /opt/cmake-3.3.2-Linux-x86_64/bin/ccmake /usr/bin/ccmake && \
    \
    wget https://github.com/perandersson/everstore/archive/master.zip && \
    unzip master.zip -d everstore && \
    rm master.zip && \
    cd /everstore/everstore-master && \
    cmake . && \
    make && \
    \
    apt-get remove -y wget build-essential unzip g++ && \
    apt-get autoremove -y && \
    apt-get clean && \
    \
    cd /everstore/everstore-master/bin && \
    cp everstore-server /everstore && \
    cp everstore-worker /everstore && \
    cd /everstore && \
    rm -rf everstore-master && \
    ln -s /everstore/everstore-server /usr/bin/everstore-server && \
    ln -s /everstore/everstore-worker /usr/bin/everstore-worker

##
## Start everstore
##

WORKDIR /everstore
ENTRYPOINT ["./everstore-server"]