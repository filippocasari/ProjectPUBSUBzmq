#!/bin/bash


cmake CMakeLists.txt
cd ./cmake-build-debug || exit


./TEST1

systemctl restart chronyd || exit
sleep 2
./PUB
