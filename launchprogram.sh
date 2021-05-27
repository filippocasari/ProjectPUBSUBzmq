#!/bin/bash

cmake CMakeLists.txt
cd ./cmake-build-debug || exit
./PUB
./SUB
