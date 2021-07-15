#!/bin/bash

cmake CMakeLists.txt
#cd ./cmake-build-debug || exit

test="/home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_"
systemctl restart chronyd || exit
for ((c = 0; c <=41; c++)); do
  trap - SIGINT
  date
  date +"%FORMAT"
  var=$(date)
  echo "##########################################################"
  echo "Start test $c at $var"
  echo "##########################################################"

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

  echo "send SIGTERM TO SUB and PUB"
  start-stop-daemon --stop --oknodo --retry 15 -n SUB3
  start-stop-daemon --stop --oknodo --retry 15 -n PUB

  #sleep 5
  #echo "send SIGKILL TO SUB and PUB"
  #killall SUB3
  #killall PUB
  sleep 10 #sleep 10 secs until next test

done
