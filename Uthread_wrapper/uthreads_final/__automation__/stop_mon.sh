#!/bin/bash

# importing Manual Automation Config
source config.sh



# Removing all Flags for clean up.
rm -f $DO_FLAG
rm -f $IDLE_FLAG
rm -f $LAUNCH_FLAG



# stopping all monitoring of reg_mon
for i in $( ps -ef | grep reg_mon | grep -v "grep" | awk -F " " '{print $2}')
	do
	kill -9 $i
done



# stopping all tail of the screen process
for i in $( ps -ef | grep "tail -f $AUTO_PATH/screenlog." | grep -v "grep" | awk -F " " '{print $2}')
	do
	kill -9 $i
done



# If called by reg_mon as abort, trying to exit the tbx_server just incase
if [ "$*" != "" ]
	then
	if [ "$REG_MON_TBX_SERVER_DEAD_KILL_SWITCH" == "TRUE" ]
		then
		$AUTO_PATH/reg_mon -i $* -Q
	fi
fi



# clearing any stopped screen processes
if screen -ls | grep Dead > /dev/null
	then
	screen -wipe 
fi