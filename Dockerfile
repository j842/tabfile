FROM phusion/baseimage:noble-1.0.0
LABEL Description="Build environment"

ENV HOME /root

SHELL ["/bin/bash", "-c"]

RUN apt-get update && \
    apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    zlib1g-dev libdeflate-dev cmake libssl-dev libasio-dev \
    clang build-essential libfmt-dev libxlsxwriter-dev \
    &&  apt-get clean

