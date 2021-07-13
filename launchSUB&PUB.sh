#!/bin/bash


cmake CMakeLists.txt
#cd ./cmake-build-debug || exit

test="/home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_"
systemctl restart chronyd || exit
for (( c=0; c<=42; c++ ))
do
   trap - SIGINT
   echo "Start test $c "

  {
    ./SUB3 "$test$c.json"
    #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
  }&
  #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
  ./PUB "$test$c.json"
done



