#!/bin/bash

cmake CMakeLists.txt
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
test_path="fileJson/test_"
systemctl restart chronyd || exit
for ((c = 0; c <=35; c++)); do
  date +"%FORMAT"
  var=$(date)
  echo "##########################################################"
  echo "Start test $c at $var ##################"
  echo "##########################################################"

  {
    ./SUB3 "$test_path$c.json"
  }&
  sleep 5
  #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
  #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json

  # shellcheck disable=SC2046
  ./PUB "$test_path$c.json"
  succ=$?
  if [ $succ -eq 0 ]; then
    echo "test succeeded..."
    sleep 5
    echo "##########################################################"
    echo "send SIGTERM and SIGKILL TO SUB"
  else
    echo " test failed"
  fi
  start-stop-daemon --stop --oknodo --retry 15 -n SUB3
  echo "##########################################################"
  echo "End test $c at $var ###################"
  echo "##########################################################"
 # start-stop-daemon --stop --oknodo --retry 15 -n PUB

  #sleep 5
  #echo "send SIGKILL TO SUB and PUB"
  #killall SUB3
  #killall PUB
  sleep 10 #sleep 10 secs until next test

done
