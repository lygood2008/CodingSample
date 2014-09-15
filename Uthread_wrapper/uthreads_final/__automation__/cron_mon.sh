#!/bin/bash

# importing Manual Automation Config
source config.sh

# Setting intital state to IDLE
IDLE="FALSE"
IDLE_COUNTER=0


# log the time into a clean file
echo "$(date)" > $CRON_SCRIPT_LOG



# MEM LEAK Fix
/usr/sbin/xm mem-set 0 4G
sleep 2
/usr/sbin/xm mem-set 0 4G



# Function Declaration
# Checking to see if genxfsim is running
function check_genxfsim {
  echo "Checking for existing genxfsim-d* processes" >> $CRON_SCRIPT_LOG
  if [ "x$(ps -ef | grep genxfsim-d | grep -v grep | awk '{print $2}')x" != "xx" ] ; then
    GENXFSIM_IDLE_CHECK="FALSE"
  else
    GENXFSIM_IDLE_CHECK="TRUE"
    let IDLE_COUNTER=${IDLE_COUNTER}+1
  fi
}



# Checking to see if reg_mon is running
function check_reg_mon {
  echo "Checking for existing reg_mon processes" >> $CRON_SCRIPT_LOG
  if [ "x$(ps -ef | grep reg_mon | grep -v grep | awk '{print $2}')x" != "xx" ] ; then
    REG_MON_IDLE_CHECK="FALSE"
  else
    REG_MON_IDLE_CHECK="TRUE"
    let IDLE_COUNTER=${IDLE_COUNTER}+1
  fi
}



# Checking to see if the qemu is running
function check_qemu {
  echo "Checking for existing qemu-dm* processes" >> $CRON_SCRIPT_LOG
  if [ "x$(ps -ef | grep qemu-dm | grep -v grep | awk '{print $2}')x" != "xx" ] ; then
    QEMU_IDLE_CHECK="FALSE"
  else
    QEMU_IDLE_CHECK="TRUE"
    let IDLE_COUNTER=${IDLE_COUNTER}+1
  fi
}



# Checking to make sure that there are no connection problems
function check_connection {
  echo "Check for connection errors" >> $CRON_SCRIPT_LOG
  if [ -e $GENXFSIM_DM_LOG ]
  then
    ERR_COUNT=$(tail -2 $GENXFSIM_DM_LOG | grep -c "$CONN_ERR")
  if [ "$ERR_COUNT" == "2" ]
  then
    touch $LAUNCH_FLAG
  fi
  fi
}



# If the manual-automation flag is set, determine the idle condition and start VM if needed
if [ -e $DO_FLAG ]
then
  echo "monitoring enabled" >> $CRON_SCRIPT_LOG
  # check for idleness
  check_genxfsim
  check_reg_mon
  check_qemu
  check_connection

  echo "GENXFSIM_IDLE_CHECK: $GENXFSIM_IDLE_CHECK" >> $CRON_SCRIPT_LOG
  echo "REG_MON_IDLE_CHECK: $REG_MON_IDLE_CHECK" >> $CRON_SCRIPT_LOG
  echo "QEMU_IDLE_CHECK: $QEMU_IDLE_CHECK" >> $CRON_SCRIPT_LOG
  echo "IDLE_COUNTER: $IDLE_COUNTER" >> $CRON_SCRIPT_LOG

  # If any idle condition returns true, set idle to true
  if [ "$IDLE_COUNTER" -ge "1" ]; then IDLE="TRUE"; fi

  if [ "$IDLE" == "TRUE" ]
  then
    touch $IDLE_FLAG
  fi
else
  echo "monitoring disabled" >> $CRON_SCRIPT_LOG
fi

