#!/bin/bash

cmake CMakeLists.txt
#cd ./cmake-build-debug || exit

test="/home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_"
systemctl restart chronyd || exit
for ((c = 1; c <=41; c++)); do
  trap - SIGINT
  date
  date +"%FORMAT"
  var=$(date)
  var=$(date)
  echo "Start test $c at $var"

  {
    ./SUB3 "$test$c.json"
  }&
  sleep 5
  #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
  #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json

  {
    ./PUB "$test$c.json"
  }&
  echo "##########################################################"
  if [ $(($c % 6)) -eq 0 ]; then

    echo "starting sleep 17 minutes fot the test"
    echo "##########################################################"
    sleep 17m
  elif [ $(($c % 6)) -eq 1 ]; then

    echo "starting sleep 4 minutes fot the test"
    echo "##########################################################"
    sleep 4m
  else
    echo "starting sleep 2 minutes fot the test"
    echo "##########################################################"
    sleep 2m
  fi

  echo "kill SUB and PUB"
  killall SUB3
  killall PUB
  sleep 10 #sleep 10 secs until next test

done
