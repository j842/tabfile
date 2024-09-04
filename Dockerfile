FROM ubuntu:24.04
LABEL Description="Build environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN sed 's@archive.ubuntu.com@ubuntu.mirror.aarnet.edu.au@' -i /etc/apt/sources.list

RUN apt-get update

RUN apt-get -y --no-install-recommends install \
    build-essential \
    clang 

RUN apt-get -y --no-install-recommends install \
    zlib1g-dev libdeflate-dev cmake libssl-dev libasio-dev \
    clang build-essential libfmt-dev libxlsxwriter-dev

