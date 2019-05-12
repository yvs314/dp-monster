FROM ubuntu:18.04
MAINTAINER Andrey Sheka <andrey.sheka@gmail.com>

RUN apt-get update && \
    apt-get install -y \
    cmake \
    clang \
    python3 \ 
    python3-pip && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives/*

RUN pip3 install pandas openpyxl

VOLUME /work
WORKDIR /work


