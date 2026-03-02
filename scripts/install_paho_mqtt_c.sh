#!/bin/bash
#
# Installs the matching version of Paho MQTT C library required by the C++ lib.
#

set -ex

git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.13

cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_BUILD_STATIC=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPAHO_ENABLE_TESTING=OFF
sudo env "PATH=$PATH" cmake --build build/ --target install
sudo ldconfig

exit 0


