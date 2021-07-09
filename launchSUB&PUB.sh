#!/bin/bash


cmake CMakeLists.txt
cd ./cmake-build-debug || exit

systemctl restart chronyd || exit
{
  ./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
}&
./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json




