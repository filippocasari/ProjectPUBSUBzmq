#!/bin/bash

argument=$1
json_path=$2
verbose=$3
which_test=$4
echo "ARG 2: $json_path"
echo "ARG 1: $argument"
echo "ARG 2: $verbose"
echo " STARTING TO COMMUNICATE VIA SSH"

/usr/bin/expect << EOF
spawn ssh pi@172.20.10.3
expect "password"
send "Miguelito99!\r"
interact
xterm
echo "STARTING COMMUNICATION... "
directory_path=$3 # can ben set by the user by argv


if [[ "$OSTYPE" == "linux-gnu"* ]]; then sudo ntpdate -u 0.it.pool.ntp.org || sudo ntpdate -u 1.it.pool.ntp.org

elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS" && sudo ntpdate -u 0.it.pool.ntp.org || sudo sntp -sS time.apple.com
fi
cd ProjectPUBSUBzmq || echo "no project 'zmq' found" && exit
if [[ "$argument" == "-s" ]]
then
  for ((j = 0; j <7; j++)); do
      {

          son_path="_${j}"
          son__path="$directory_path$which_test$son_path"
          ./SUB2 "$json_path$j.json" "$son__path$/" "$verbose"
      }&
  done
  sleep 30
fi
exit
EOF
echo "FINISHHHHH"