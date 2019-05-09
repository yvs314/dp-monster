FROM ubuntu:18.04
MAINTAINER Andrey Sheka <andrey.sheka@gmail.com>

RUN apt-get update && \
    apt-get install -y \
    cmake \
    clang \
    python3 && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*


VOLUME /work
WORKDIR /work


