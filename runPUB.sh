#!/bin/bash
cmake CMakeLists.txt || echo "Error to load CMakeLists"
#cd ./cmake-build-debug || exit
#test_date=$(date +"%H:%M")
json_path=$2
argument=$1
verbose=$3
echo "ARG 2: $json_path"
echo "ARG 1: $argument"
echo "ARG 3: $verbose"


directory_path="./ResultsCsv_" # can ben set by the user by argv
if [[ "$OSTYPE" == "linux-gnu"* ]]; then sudo ntpdate -u 0.it.pool.ntp.org || sudo ntpdate -u 1.it.pool.ntp.org

elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS" && sudo ntpdate -u 0.it.pool.ntp.org || sudo sntp -sS time.apple.com
fi

for ((i = 0; i<=10; i++)); do
  directory_path="./ResultsCsv_$i\_"
  for ((c = 0; c <15; c++)); do
    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"


    bash ./runSUBS.sh  "-s" "$json_path$c.json" "$verbose" "$directory_path"
    if [[ "$argument" -eq "-p" ]]
    then
      ./PUB2 "$json_path$c.json" "-v"
    fi
    sleep 10
    bash ./stopSUBS.sh
    sleep 5
  done
done