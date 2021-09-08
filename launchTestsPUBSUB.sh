#!/bin/bash
cmake CMakeLists.txt || echo "Error to load CMakeLists"
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
test_path=$2
argument=$1
verbose=$3
echo "ARG 2: $test_path"
echo "ARG 1: $argument"
echo "ARG 3: $verbose"



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
        ./SUB2 "$test_path$c.json" "$son__path$i/" "$verbose"
      }
      done
    elif [[ "$argument" -eq "-p" ]]
    then
      ./PUB2 "$test_path$c.json" "-v"
    else
      {
        if [[ "$OSTYPE" == "linux-gnu"* ]]
        then
          ./SUB2 "$test_path$c.json" "$directory_path$i/" "$verbose"
        elif [[ "$OSTYPE" == "darwin"* ]];
        then
          ./SUB2 "$test_path$c.json" "$directory_path$i/" "$verbose"
        fi
      }&

      sleep 5
      ./PUB2 "$test_path$c.json"

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

    if [[ "$OSTYPE" == "linux-gnu"* ]]
    then
      sudo start-stop-daemon --stop --oknodo --retry 15 -n SUB2
      sudo start-stop-daemon --stop --oknodo --retry 15 -n PUB2
    fi
    killall SUB2
    sleep 5
    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"

  done
done