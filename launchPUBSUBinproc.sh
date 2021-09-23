#!/bin/bash

sudo cmake ./CMakeLists.txt || echo "Error to load CMakeLists"

args=("$@")
echo $# arguments passed
echo ${args[0]} ${args[1]} ${args[2]}
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
test_path=$1
directory_path=$2
verbose=$3
#if [[ "$OSTYPE" == "linux-gnu"* ]]; then systemctl restart chronyd && echo "TEST ON LINUX" || exit
#elif [[ "$OSTYPE" == "darwin"* ]]; then
 # echo " TESTS ON MAC OS"
#fi
for ((i = 0; i<=2; i++)); do

  for ((c = 0; c <=11; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"
    sleep 5

    if [[ "$OSTYPE" == "linux-gnu"* ]]
    then
      {
        ./INPROCESS_TEST_M "$test_path$c.json" "$directory_path$i/" "$verbose"
      }&

    elif [[ "$OSTYPE" == "darwin"* ]]
    then
      {
        ./INPROCESS_TEST_M "$test_path$c.json" "$directory_path$i/" "$verbose"
        succ=$?
            if [ $succ -eq 0 ]
            then
              echo
              echo "test succeeded..."
              sleep 5
              echo "##########################################################"
              echo "send SIGTERM and SIGKILL TO INPROC TEST"
            else
              # shellcheck disable=SC1072
              echo " test failed"
              echo "exit code: "$succ
            fi
      }&

    fi
    sleep 65
    if [[ "$OSTYPE" == "linux-gnu"* ]]
    then
      sudo start-stop-daemon --stop --oknodo --retry 15 -n INPROCESS_TEST_M
      sleep 5
    fi
    killall INPROCESS_TEST_M

    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"
   # start-stop-daemon --stop --oknodo --retry 15 -n PUB
    sleep 10 #sleep 10 secs until next test

  done
done

