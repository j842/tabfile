FROM ubuntu:24.04
LABEL Description="Build environment"

ENV HOME=/root

SHELL ["/bin/bash", "-c"]

RUN apt-get update && \
    apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    zlib1g-dev libdeflate-dev cmake libssl-dev libasio-dev \
    clang build-essential libfmt-dev libxlsxwriter-dev \
    wget libspdlog-dev \
    &&  apt-get clean

RUN wget --no-check-certificate https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz && tar zxvf make-4.4.1.tar.gz 
RUN cd make-4.4.1/ && ./configure && make \
    && make install && rm -f /usr/bin/make && ln -s /usr/local/bin/make /usr/bin/make
