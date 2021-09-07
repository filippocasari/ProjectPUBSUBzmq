#!/bin/bash
args=("$@")
cmake CMakeLists.txt || echo "Error to load CMakeLists"
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
test_path=${args[2]}
argument=${args[1]}
echo "ARG 2: $test_path"
echo "ARG 1: $argument"

nc -l 2389 > $out

directory_path="./ResultsCsv_" # can ben set by the user by argv
if [[ "$OSTYPE" == "linux-gnu"* ]]; then sudo ntpdate -u 0.it.pool.ntp.org || sudo ntpdate -u 1.it.pool.ntp.org

elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS" && sudo ntpdate -u 0.it.pool.ntp.org || sudo sntp -sS time.apple.com
fi

for ((i = 0; i<=10; i++)); do

  for ((c = 0; c <12; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"

    if [[ "$argument" == "-s" ]]
    then
      for (( j = 0 ; j < 7 ; j++));do
      {
        son_path="_${j}"
        son__path="$directory_path$son_path"
        ./SUB2 "$test_path$c.json" "$son__path$i/" "-nv"
      }&
      done
      if [[ out -eq "TERMINATE" ]];then sleep 10 && killall SUB2


    elif [[ "$argument" -eq "-p" ]]
    then
      ./PUB2 "$test_path$c.json" "-v"
      sleep 10
    else
      {
        if [[ "$OSTYPE" == "linux-gnu"* ]]
        then
          ./SUB2 "$test_path$c.json" "$directory_path$i/" "-v"
          sudo chmod +rwx "./"$directory_path$i/
        elif [[ "$OSTYPE" == "darwin"* ]];
        then
          ./SUB2 "$test_path$c.json" "$directory_path$i/" "-v"
        fi
      }&

      sleep 5

      #./SUB3 /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json
      #./PUB /home/filippocasari/CLionProjects/ProjectPUBSUBzmq/fileJson/test_1.json

      # shellcheck disable=SC2046

      if [[ "$OSTYPE" == "linux-gnu"* ]]
      then
          ./PUB2 "$test_path$c.json"
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
        sudo start-stop-daemon --stop --oknodo --retry 15 -n SUB2
        sleep 5
        killall SUB2
      else
        killall SUB2
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