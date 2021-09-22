#!/bin/bash
cmake CMakeLists.txt || echo "Error to load CMakeLists"
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
json_path=$2
argument=$1
verbose=$3
echo "ARG 1: $argument"
echo "ARG 2: $json_path"
echo "ARG 3: $verbose"


directory_path="./ResultsLAN_CH_" # can ben set by the user by argv
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo " TEST ON LINUX"  #0.it.pool.ntp.org

elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS" #0.it.pool.ntp.org #|| sudo sntp -sS time.apple.com
fi
sudo ntpdate -s 0.ch.pool.ntp.org
ntp_success=$?
if [[ ntp_success -eq 0 ]]; then
  echo "Test NTP SUCCESS"
else
  echo "Test NTP FAILED"
fi
sleep 3
for ((i = 0; i<=10; i++)); do

  for ((c = 0; c <15; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"

    if [[ "$argument" == "-s" ]]
    then
      echo "#################START ONLY SUBSCRIBERS"
      for (( j = 0 ; j < 7; j++));do

        son_path="_${j}"
        son__path="$directory_path$i$son_path"
        {
          ./SUB2 "$json_path$c.json" "$son__path/" "$verbose"
        }&

      done
      sleep 100
      echo "SUBS WILL BE STOPPED"
      killall SUB2

    elif [[ "$argument" -eq "-p" ]]
    then
      echo "#################START ONLY PUBLISHER"
      ./PUB2 "$json_path$c.json" "-v"
      sleep 5
    else
      {
        if [[ "$OSTYPE" == "linux-gnu"* ]]
        then
          ./SUB2 "$json_path$c.json" "$directory_path$i/" "$verbose"
        elif [[ "$OSTYPE" == "darwin"* ]];
        then
          ./SUB2 "$json_path$c.json" "$directory_path$i/" "$verbose"
        fi
      }&

      sleep 5
      ./PUB2 "$json_path$c.json"

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
      if [[ "$argument" == "-s" ]];then
      sudo start-stop-daemon --stop --oknodo --retry 15 -n SUB2
      elif [[ "$argument" == "-p" ]];then
        sudo start-stop-daemon --stop --oknodo --retry 15 -n PUB2
      fi
    fi
    killall SUB2
    sleep 5
    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"

  done
done