#!/bin/bash
# importing Manual Automation Config
source config.sh


# this script is intended to run a minute after cron_mon.sh
# to allow for any race conditions to be accounted for

# Setting Default State to IDLE and NOT_STARTED
IDLE="FALSE"
START_UP="FALSE"
IDLE_COUNTER=0

# log the time into a clean file
echo "$(date)" >> $CRON_SCRIPT_LOG



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



# If the manual-automation flag is set, determine the idle condition and start VM if needed
if [ -e $DO_FLAG ]
then
  echo "monitoring enabled" >> $CRON_SCRIPT_LOG
  # check for idle flag from other script
  if [ -e $IDLE_FLAG ]
  then
    rm -f $IDLE_FLAG
    check_genxfsim
    check_qemu
    check_reg_mon

    echo "GENXFSIM_IDLE_CHECK: $GENXFSIM_IDLE_CHECK" >> $CRON_SCRIPT_LOG
    echo "REG_MON_IDLE_CHECK: $REG_MON_IDLE_CHECK" >> $CRON_SCRIPT_LOG
    echo "QEMU_IDLE_CHECK: $QEMU_IDLE_CHECK" >> $CRON_SCRIPT_LOG
    echo "IDLE_COUNTER: $IDLE_COUNTER" >> $CRON_SCRIPT_LOG

    # If any idle condition returns true, set idle to true
    if [ "$IDLE_COUNTER" -ge "1" ]; then IDLE="TRUE"; fi

    # If IDLE is true, set the start flag
    if [ "$IDLE" == "TRUE" ]
    then
      START_UP="TRUE"
    fi
  fi



  # If the Launch flag exists (usually created by hung_vm.sh), Set start flag to "TRUE"
  if [ -e $LAUNCH_FLAG ]
  then
    rm -f $LAUNCH_FLAG
    START_UP="TRUE"
  fi



  # If Start flag is "TRUE", Start VM
  if [ "$START_UP" == "TRUE" ]
  then
      bash $CRON_VM_START
  fi
else
  # Log status
  echo "monitoring disabled" >> $CRON_SCRIPT_LOG
fi

