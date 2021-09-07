#!/bin/bash
args=("$@")
cmake CMakeLists.txt || echo "Error to load CMakeLists"
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
test_path=${args[2]}
argument=${args[1]}
echo "ARG 2: $test_path"
echo "ARG 1: $argument"



directory_path="./ResultsCsv_" # can ben set by the user by argv
if [[ "$OSTYPE" == "linux-gnu"* ]]; then sudo ntpdate -u 0.it.pool.ntp.org && echo "TEST ON LINUX" || exit
elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS" && sudo ntpdate -u 0.it.pool.ntp.orgn
fi

for ((i = 0; i<=10; i++)); do

  for ((c = 0; c <=11; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"

    if [[ "$argument" == "-s" ]]
    then
        sudo ./SUB2 "$test_path$c.json" "$directory_path$i/" "-v"
    elif [[ "$argument" -eq "-p" ]]
    then
      sudo ./PUB2 "$test_path$c.json" "-v"
      sleep 10
    else
      {
        if [[ "$OSTYPE" == "linux-gnu"* ]]
        then
          sudo ./SUB2 "$test_path$c.json" "$directory_path$i/" "-v"
          sudo chmod +rwx "./"$directory_path$i/
        elif [[ "$OSTYPE" == "darwin"* ]];
        then
          ./SUB "$test_path$c.json" "$directory_path$i/" "-v"
        fi
      }&

      sleep 5

      #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
      #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json

      # shellcheck disable=SC2046

      if [[ "$OSTYPE" == "linux-gnu"* ]]
      then
          sudo ./PUB2 "$test_path$c.json"
      elif [[ "$OSTYPE" == "darwin"* ]]
      then
          ./PUB2 "$test_path$c.json"
      fi
      succ=$?
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
      sleep 10

    fi
    if [[ "$argument" == "-s" ]]
    then
      if [[ "$OSTYPE" == "linux-gnu"* ]]
      then
        sudo start-stop-daemon --stop --oknodo --retry 15 -n SUB
        sleep 5
        killall SUB
      else
        killall SUB
      fi
    fi
    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"
   # start-stop-daemon --stop --oknodo --retry 15 -n PUB
    if [[ "$argument" == "-p" ]]
    then
      sleep 10 #sleep 10 secs until next test
    fi

  done
done