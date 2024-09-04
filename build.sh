#!/bin/bash

SCRIPT_DIR="$(dirname "$(realpath "$0")")"

echo "Building docker image"
docker build -t tabbuild .

echo "Building tabfile"
docker run -v "$(pwd)":/root tabbuild /bin/bash -c "cd /root && make && chmod a+rwx build/tabfile && mv build/tabfile . && rm -rf build"

