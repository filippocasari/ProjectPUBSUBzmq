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


directory_path="./MACM1/TEST_2_RESTORED/1SUB_TCP_NET_3_" # can ben set by the user by argv
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo " TEST ON LINUX"  #0.it.pool.ntp.org

elif [[ "$OSTYPE" == "darwin"* ]]; then
  echo " TESTS ON MAC OS" #0.it.pool.ntp.org #|| sudo sntp -sS time.apple.com
fi
#sudo ntpdate -u time.apple.com
#ntp_success=$?
#    if [[ ntp_success -eq 0 ]]; then
 #     echo "Test NTP SUCCESS"
#    else
 #     echo "Test NTP FAILED"
 #   fi
#sleep 3
#sudo ntpdate -u time.apple.com
    #ntp_success=$?
    #    if [[ ntp_success -eq 0 ]]; then
   #       echo "Test NTP SUCCESS"
    #    else
    #      echo "Test NTP FAILED"
     #   fi
for ((i = 0; i<10; i++)); do

  mkdir $directory_path"$i"
  for ((c = 0; c <15; c++)); do

    date +"%FORMAT"
    var=$(date)
    echo "##########################################################"
    echo "Start test $c at $var #########"
    echo "##########################################################"

    if [[ "$argument" == "-s" ]]
    then
      echo "#################START ONLY SUBSCRIBERS"
      for (( j = 0 ; j < 1; j++));do

        son_path="/${j}"
        son__path="$directory_path$i$son_path"
        {
          ./SUB2 "$json_path$c.json" "$son__path" "$verbose"
        }&

      done
      if [ $c -eq 0 ] || [ $c -eq 5 ] || [ $c -eq 10 ]; then echo "sleep of 70 secs" && sleep 60
                else echo "sleep of 60 secs" && sleep 35; fi
      echo "SUBS WILL BE STOPPED"
      killall SUB2
    fi
    if [[ "$argument" == "-p" ]]
    then
      echo "################# START ONLY PUBLISHER ###############"
      ./PUB2 "$json_path$c.json" "-v"
      sleep 5

    elif [[ "$argument" == "-sp" ]]
      then
      echo "MODE: PUB SUB BOTH"
      {
        ./PUB2 "$json_path$c.json" "-v"
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
      }&
      sleep 5
      for (( j = 0 ; j < 7; j++));do
        {
           echo starting sub "$j"
           son_path="/${j}"
           son__path="$directory_path$i$son_path"
          ./SUB2 "$json_path$c.json" "$son__path" "$verbose"
        }&
      done

    if [ $c -eq 0 ] || [ $c -eq 5 ] || [ $c -eq 10 ]; then echo "sleep of 70 secs" && sleep 55
          else echo "sleep of 60 secs" && sleep 35; fi
    fi
    killall SUB2
    killall PUB2
    sleep 5

    echo "##########################################################"
    echo "End test $c at $var #########"
    echo "##########################################################"

  done
done