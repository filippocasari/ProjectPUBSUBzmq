#!/bin/bash

spawn ssh pi@172.20.10.3
expect "password"
send "Miguelito99!\r"
interact
cd ProjectPUBSUBzmq || echo "no project 'zmq' found" && exit
if [[ "$OSTYPE" == "linux-gnu"* ]]
    then
      start-stop-daemon --stop --oknodo --retry 15 -n SUB2
      start-stop-daemon --stop --oknodo --retry 15 -n PUB2
    fi
    killall SUB2
    sleep 5
    var="$date"
    echo "##########################################################"
    echo "End test at $var #########"
    echo "##########################################################"
exit
echo "STOP SHH CONNECTION"