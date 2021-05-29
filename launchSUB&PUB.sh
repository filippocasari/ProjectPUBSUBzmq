#!/bin/bash


cmake CMakeLists.txt
cd ./cmake-build-debug || exit

systemctl restart chronyd || exit
{
  ./TEST1 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/parameters.json
}&
sleep 1
./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/parameters.json
nano ./;



