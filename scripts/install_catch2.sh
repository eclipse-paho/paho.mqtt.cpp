#!/bin/bash
#
# install_catch2.sh
#
# Travis CI build/test helper script for the Eclipse Paho C++ library.
# This installs Catch2 into the VM.
#

set -ex

VERSION=2.9.1

# Install Catch2 from sources
wget https://github.com/catchorg/Catch2/archive/v${VERSION}.tar.gz
tar -xf v${VERSION}.tar.gz
cd Catch2-${VERSION}/
cmake -Bbuild -H. -DBUILD_TESTING=OFF

# CMake bin is installed in a strange place where
# sudo can not find by default.
sudo env "PATH=$PATH" cmake --build build/ --target install

