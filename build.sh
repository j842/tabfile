#!/bin/bash

SCRIPT_DIR="$(dirname "$(realpath "$0")")"

docker build -t tabbuild .

docker run "$(pwd)":/root tabbuild make


