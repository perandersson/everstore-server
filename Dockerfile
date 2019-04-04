FROM ubuntu AS builder

##
## 1. Install basic ubuntu tools needed to build everstore
## 2. Download and install CMake 3.x
## 3. Download and build everstore
## 4. Remove unnecessary stuff (such as the compiler)
## 5. Copy files to appropriate directories
##

# Install Development Tools
RUN apt-get update
RUN apt-get install -y wget unzip
RUN apt-get install -y gcc g++ build-essential libidn11

# Install CMAKE
RUN wget https://cmake.org/files/v3.6/cmake-3.6.3-Linux-x86_64.tar.gz && \
    tar -C /opt -xf cmake-3.6.3-Linux-x86_64.tar.gz && \
    rm cmake-3.6.3-Linux-x86_64.tar.gz && \
    ln -s /opt/cmake-3.6.3-Linux-x86_64/bin/cmake /usr/bin/cmake && \
    ln -s /opt/cmake-3.6.3-Linux-x86_64/bin/ccmake /usr/bin/ccmake

# Copy the source code
COPY . /src

# Compile the code
WORKDIR /src
RUN cmake . && make && cd bin && ./everstore-tests

##
## Create an image containing only the server and workers
##

FROM ubuntu
COPY --from=builder /src/bin/everstore-server /
COPY --from=builder /src/bin/everstore-worker /
EXPOSE 6929
ENTRYPOINT ["./everstore-server"]