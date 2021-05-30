#!/bin/bash


cmake CMakeLists.txt
cd ./cmake-build-debug || exit

systemctl restart chronyd || exit
{
  ./SUB2 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/parameters.json
}&
./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/parameters.json




