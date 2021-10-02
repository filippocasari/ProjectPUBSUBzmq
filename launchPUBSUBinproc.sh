#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]
then
    cmake ./CMakeLists.txt || echo "Error to load CMakeLists"
else
    sudo cmake ./CMakeLists.txt || echo "Error to load CMakeLists"
fi
args=("$@")
echo $# arguments passed
echo ${args[0]} ${args[1]} ${args[2]}

test_path=$1
directory_path=$2
verbose=$3
#if [[ "$OSTYPE" == "linux-gnu"* ]]; then systemctl restart chronyd && echo "TEST ON LINUX" || exit
#elif [[ "$OSTYPE" == "darwin"* ]]; then
 # echo " TESTS ON MAC OS"
#fi
for ((i = 0; i<=9; i++)); do
  echo
  echo "####################### MASTER TEST $i ##########################"
  echo
  mkdir "$directory_path$i/" || echo "KEEP GOING..."
  for ((c = 0; c <=14; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"
    sleep 5
    {
      ./INPROC_TEST_M "$test_path$c.json" "$directory_path$i/" "$verbose"
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
    if [ $c -eq 0 ] || [ $c -eq 5 ] || [ $c -eq 10 ]; then echo "sleep of 110 secs" && sleep 110
    else echo "sleep of 90 secs" && sleep 90; fi
    # 100 secs of sleep to get the results
    killall INPROC_TEST_M
    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"
   # start-stop-daemon --stop --oknodo --retry 15 -n PUB
    sleep 3 #sleep 10 secs until next test

  done
done

