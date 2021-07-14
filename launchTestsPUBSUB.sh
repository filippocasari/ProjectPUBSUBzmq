#!/bin/bash


cmake CMakeLists.txt
#cd ./cmake-build-debug || exit

test="/home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_"
systemctl restart chronyd || exit
for (( c=0; c<=41; c++ ))
do
   trap - SIGINT
   echo "Start test $c "

  {
    ./SUB3 "$test$c.json"
    #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
  }&
  #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
  {
     ./PUB "$test$c.json"
  }&

  if [ $(($c % 6)) -eq 0 ]
  then
    echo "starting sleep 17 minutes fot the test"
    sleep 17m
  else
    echo "starting sleep 4 minutes fot the test"
    sleep 4m
  fi
  killall SUB3
  killall PUB
  sleep 10

done



