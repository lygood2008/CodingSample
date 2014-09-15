#!/bin/bash
VERSION="v2.2.0-BETA"
### Project Link: https://github.intel.com/avronskx/Manual-Automation

export PATH=${PATH}:/usr/sbin:/usr/kerberos/sbin:/usr/kerberos/bin:/usr/bin:/bin:/root/bin:/sbin:/usr/local/sbin:/usr/local/bin/usr/lib64/ccache:/home/genxfsim/.local/bin:/home/genxfsim/bin
export DISPLAY=":0.0"
/usr/bin/xhost + > /dev/null

###################################################################################
### SCRIPT VARS ###################################################################
###################################################################################

# Where is the scripts directory with all the VM_STARTUP scripts
AUTO_PATH="/mnt/share/__automation__"

# VM start up script name
VM_START="$AUTO_PATH/VM_STARTUP_3D_em_CNL_Win8_64.sh"

# Has Log Path
LOG_PATH="/has/log"

# TM Share Path (needs to be pre-mounted)
TM_SHARE_PATH="/mnt/TestMinion"

# ShareDrive Mount Path
LUCAS_SHARE_PATH="/mnt/share/ShareDrive"

# VM Mount Point
VM_IMAGE="/mnt/share/os"

# Location for recording hang info
HANG_DIR="/mnt/share/hang_sigs"
HANG_SIGS="hangs.txt"
HANG_SCREENSHOT_NAME="hang.jpg"

# Location for recording config info
CONFG_DIR="/mnt/share/CONFG_DIR"
CONFG_LOG="config_log.txt"

### Flags #############################################

# existence of this file causes script to purge the log folder of files before starting the VM
PURGE_LOG_FLAG="$AUTO_PATH/purge_logs.flag"

# existence of this file tells us if we're in monitoring mode
DO_FLAG="$AUTO_PATH/do_mon.flag"

# flag file for launch needed
LAUNCH_FLAG="$AUTO_PATH/launch.flag"

# flag file for idleness
IDLE_FLAG="$AUTO_PATH/idle.flag"
#######################################################

### Log and Temp file locations #######################
# The log file to be copied from the VM to the hang signature file
# This is legacy - still supported
WRAPPER_LOG="Users/Intel/dxlogs/awrap/awrap_console.log"

# Filt and GT type files needed for hang_sig.sh
FILT_IP_FILE="$AUTO_PATH/_FILT_IP_.txt"
GT_TYPE_FILE="$AUTO_PATH/_GT_TYPE_.txt"
PROXY_PORT_FILE="$AUTO_PATH/_PROXY_PORT_.txt"

### Cron_mon Settings #################################
# script's log file location and name
CRON_SCRIPT_LOG="$AUTO_PATH/cron_script.log"

# genxfsim-dm log file name
GENXFSIM_DM_LOG="/has/log/genxfsim-dm.log"

# genxfsim-dm.log error string
CONN_ERR="Trying to MMIO read without connection"

# script to start the VM
CRON_VM_START="$AUTO_PATH/start_vm.sh"
#######################################################

### Time Stamp and Temp Start file location ###########
# Grabbing a time stamp
STAMP=$(date --rfc-3339=seconds | sed -e 's/:/-/g' | sed -e 's/ /_/g')

# temp start file name
TEMP_VM_START=${STAMP}_start.sh
#######################################################

### Default Settings for start_vm.sh ##################
DEFAULT_LUA_FILE=/has/scripts/start_bxt_pipegt.lua
DEFAULT_TBX_SERVER_IP=127.0.0.1
DEFAULT_GT="2"
DEFAULT_PHY="/dev/vcg/3d_winB"
#DEFAULT_PHY1="/dev/vcg/SharedDrive"
DEFAULT_PHY1=
DEFAULT_REGINI_FILE=/has/config/include/units/skl_regini.xml
DEFAULT_FILTER_REMAP=/has/config/include/units/filter_remap.xml
DEFAULT_PROXY_PORT=4321
DEFAULT_HANG_TIMEOUT=60
DEFAULT_HANG_COUNT=10
DEFAULT_VM_NAME=has0
DEFAULT_IM_PART=3d_winB
DEFAULT_PARTITION=/dev/mapper/vcg-
DEFAULT_IM_PART1=SharedDrive
DEFAULT_PARTITION1=/dev/vcg/
DEFAULT_HOST_CONFIG_NAME=host_config
DEFAULT_SCRIPT_SLEEP_TIME=60
#######################################################

### Behavioral Settings ###############################
# Exit manual_automation when tbx_server dies. Set to TRUE or FLASE
REG_MON_TBX_SERVER_DEAD_KILL_SWITCH=TRUE

# Debug messages flag; set it nothing [blank] if you don't want them
DEBUG_FLAG=
#######################################################


###################################################################################
### End of - SCRIPT VARS ##########################################################
###################################################################################

###################################################################################
### Dumping all the Vars ##########################################################
###################################################################################

# Dumping all the scripts vars if debug mode is on
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: ###### Debug flag is on, dumping all states ######"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: AUTO_PATH: $AUTO_PATH"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: VM_START: $VM_START"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: LOG_PATH: $LOG_PATH"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: TM_SHARE_PATH: $TM_SHARE_PATH"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: LUCAS_SHARE_PATH: $LUCAS_SHARE_PATH"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: VM_IMAGE: $VM_IMAGE"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HANG_DIR: $HANG_DIR"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HANG_SIGS: $HANG_SIGS"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HANG_SCREENSHOT_NAME: $HANG_SCREENSHOT_NAME"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: CONFG_DIR: $CONFG_DIR"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: CONFG_LOG: $CONFG_LOG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PURGE_LOG_FLAG: $PURGE_LOG_FLAG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: DO_FLAG: $DO_FLAG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: LAUNCH_FLAG: $LAUNCH_FLAG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: IDLE_FLAG: $IDLE_FLAG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: WRAPPER_LOG: $WRAPPER_LOG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: FILT_IP_FILE: $FILT_IP_FILE"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: GT_TYPE_FILE: $GT_TYPE_FILE"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PROXY_PORT_FILE: $PROXY_PORT_FILE"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: CRON_SCRIPT_LOG: $CRON_SCRIPT_LOG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: GENXFSIM_DM_LOG: $GENXFSIM_DM_LOG"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: CONN_ERR: $CONN_ERR"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: VM_START: $VM_START"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: STAMP: $STAMP"; fi

###################################################################################
### End of - Dumping all the Vars #################################################
###################################################################################
