#!/bin/bash

cmake CMakeLists.txt || echo "Error to load CMakeLists"
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
test_path="./fileJson/test_"


directory_path="./ResultsCsv_" # can ben set by the user by argv
if [[ "$OSTYPE" == "linux-gnu"* ]]; then systemctl restart chronyd && echo "TEST ON LINUX" || exit
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS"
fi
for ((i = 0; i<=10; i++)); do

  for ((c = 0; c <=11; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"

    {
      if [[ "$OSTYPE" == "linux-gnu"* ]]
      then
        sudo nice --19 ./SUB "$test_path$c.json" "$directory_path$i/"
        sudo chmod +rwx "./"$directory_path$i/
      elif [[ "$OSTYPE" == "darwin"* ]];
      then
        ./SUB "$test_path$c.json" "$directory_path$i/"
      fi
    }&
    sleep 5
    #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
    #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json

    # shellcheck disable=SC2046

    if [[ "$OSTYPE" == "linux-gnu"* ]]
    then
        sudo nice --19 ./PUB "$test_path$c.json"
        succ=$?
    elif [[ "$OSTYPE" == "darwin"* ]]
    then
        ./PUB "$test_path$c.json"
        succ=$?
    fi

    if [ $succ -eq 0 ]
    then
      echo
      echo "test succeeded..."
      sleep 5
      echo "##########################################################"
      echo "send SIGTERM and SIGKILL TO SUB"
    else
      # shellcheck disable=SC1072
      echo " test failed"
      echo "exit code: "$succ
    fi
    sleep 20
    if [[ "$OSTYPE" == "linux-gnu"* ]]
    then
      sudo start-stop-daemon --stop --oknodo --retry 15 -n SUB
      sleep 5
      killall SUB
    else
      killall SUB
    fi

    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"
   # start-stop-daemon --stop --oknodo --retry 15 -n PUB

    sleep 10 #sleep 10 secs until next test

  done
done