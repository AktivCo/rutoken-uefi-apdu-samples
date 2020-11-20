#!/bin/bash

set -e

pushd edk2
make -C BaseTools
popd

export WORKSPACE=$(pwd)
export PACKAGES_PATH=$(pwd)/edk2:$(pwd)

. edk2/edksetup.sh

build -t GCC5 -a X64 -b RELEASE -p ./RutokenSamplesPkg/RutokenSamplesPkg.dsc
