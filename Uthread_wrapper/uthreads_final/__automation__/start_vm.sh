#!/bin/bash

# importing Manual Automation Config (by running the file)
source config.sh


###################################################################################
### test code #####################################################################
###################################################################################

# Creating a temp VM launch script
#   Filters out the "genxfsim-dm" so that the VM doesn't launch when we import the file.
if [ ! -e ${AUTO_PATH}/temp ]; then mkdir -p ${AUTO_PATH}/temp; fi
cat ${VM_START} | sed 's|\/has\/bin\/genxfsim-dm|echo \>\/dev\/null |g' > ${AUTO_PATH}/temp/${TEMP_VM_START}
chmod +x ${AUTO_PATH}/temp/${TEMP_VM_START}



# Runs the temp VM start file to import all the vars
source ${AUTO_PATH}/temp/${TEMP_VM_START}


# Removing the temp VM start file
rm -f ${AUTO_PATH}/temp/${TEMP_VM_START}



# parse the VM_START for the start lua file
#   If not defined, set a default lua file
#   Pull out the lua file name as well
LUA_FILE=$(grep .lua $VM_START | grep -v '^#' | awk -F "#" '{print $1}' | tail -1 | sed -e 's/.*=//g' | sed -e 's|\/has\/bin\/genxfsim-dm -S ||g' | sed -e 's/;//g')
if [ "" == "$LUA_FILE" ]
  then
  LUA_FILE=$DEFAULT_LUA_FILE
fi
LUA_FILE_NAME=$(echo $LUA_FILE | sed -e 's/.*\///g') 
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: LUA_FILE: $LUA_FILE"; fi


###################################################################################
### End of - Test code ############################################################
###################################################################################

###################################################################################
### WA Section ####################################################################
###################################################################################

# WA - Changing dom0 balloning to "no" to make sure for removal of Genxfsim/HAS timing contention
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Changing enable-dom0-ballooning to no"; fi
sed -i 's|enable-dom0-ballooning yes|enable-dom0-ballooning no|g' /etc/xen/xend-config.sxp
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Status: $(cat /etc/xen/xend-config.sxp | grep dom0-ball | tail -1)"; fi
export HAS_DISABLE_SET_DOM0_MEM=1



# Adding export var to dump the temp_conf_ file
export HAS_KEEP_TEMPCONFIG=1
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HAS_KEEP_TEMPCONFIG: $HAS_KEEP_TEMPCONFIG"; fi



# Need to cd into AUTO_PATH for the screen regmon loggin to work correctly
cd $AUTO_PATH

###################################################################################
### End of - WA Section ###########################################################
###################################################################################

###################################################################################
### Functions #####################################################################
###################################################################################

# This kills all instances of genxfsim thats currently running (will kill all vms)
function kill_genxfsim {
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Checking for left over genxfsim-d* processes and killing them"; fi
  for i in $(ps -ef | grep genxfsim-dm | grep -v "grep" | awk -F " " '{print $2}')
  do 
    kill -9 $i
  done
}



# This kills any VM that's passed into this function
kill_vm() {
  if xm list | grep "$1" >/dev/null
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Using XEN to destroy $1"; fi
    xm dest "$1"
  fi
}



# Checks to see if the genxfsim is running (in general)
function check_genxfsim {
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Checking to see if the VM has started"; fi
  if [ "x$(ps -ef | grep genxfsim-d | grep -v grep | awk '{print $2}')x" != "xx" ] ; then
     STARTED="TRUE"
  fi
}

###################################################################################
### End of - Functions ############################################################
###################################################################################

###################################################################################
######### Start of Script # Parsing the VM_STARTUP Script for Values ##############
###################################################################################

# Setting the STARTED State to "FASLE" indicating that the VM needs to be started.
STARTED="FALSE"



# Making sure TBX_SERVER_IP is set; needed for reg_mon
if [ "" == "$TBX_SERVER_IP" ]
  then
  TBX_SERVER_IP=$DEFAULT_TBX_SERVER_IP
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: TBX_SERVER_IP: $TBX_SERVER_IP"; fi



# Grabs both filt and filt IP address in a very confusing manner
FILT_IP=`nslookup $TBX_SERVER_IP | grep Address: | tail -1 | awk -F ": " '{print $2}'`
if [ "" == "$FILT_IP" ]
  then
  FILT_IP=$TBX_SERVER_IP
  FILT_NAME=`nslookup $TBX_SERVER_IP | grep name | awk -F "= " '{print $2}' | sed -e 's|.$||'`
else 
  FILT_NAME=$TBX_SERVER_IP
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: FILT_IP: $FILT_IP"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: FILT_NAME: $FILT_NAME"; fi



# Making sure "GT" is defined
if [ "" == "$GT" ]
  then
  GT=$DEFAULT_GT
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: GT: $GT"; fi



# Making sure that PHY var is set from HAS_DISK
PHY=${HAS_DISK}
PHY=$(echo ${PHY} | sed -e 's/^.*phy://g' | sed -e 's/,.*$//g')
if [ "" == "$PHY" ]
  then
  PHY="$DEFAULT_PHY"
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PHY: $PHY"; fi



# Making sure that the PHY1 var is set from HAS_DISK1
PHY1=${HAS_DISK1}
PHY1=$(echo $PHY1 | sed -e 's/^.*phy://g' | sed -e 's/,.*$//g')
if [ "" == "$PHY1" ]
  then
  PHY1=$DEFAULT_PHY1
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PHY1: $PHY1"; fi



# Setting FULSIM_PATH
#   Uncertain how to pull the FULSIM_PATH from config files yet; therefore cannot set default value
FULSIM_PATH=$HAS_FULSIM_PATH
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: FULSIM_PATH: $FULSIM_PATH"; fi



# Getting the Regini file
REGINI_FILE=$REGINI
if [ "" == "$REGINI_FILE" ]
  then
  REGINI_FILE=$(grep REGINI $LUA_FILE | grep -v '^#' | awk -F "#" '{print $1}' | grep .xml | tail -1 | sed -e 's|[ a-zA-Z0-9\.\(]*"REGINI", "||g' | sed -e 's|")||g')
fi
if [ "" == "$REGINI_FILE" ]
  then
  REGINI_FILE=$DEFAULT_REGINI_FILE
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: REGINI_FILE: $REGINI_FILE"; fi



# setting the filter remap file
if [ -e $DEFAULT_FILTER_REMAP ]
  then
  FILTER_REMAP=$DEFAULT_FILTER_REMAP
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: FILTER_REMAP: $FILTER_REMAP"; fi



# setting the proxy port
PROXY_PORT=${HAS_PROXY_PORT}
if [ "" == "$PROXY_PORT" ]
  then
  PROXY_PORT=$(grep HAS_PROXY_PORT $LUA_FILE | grep -v '^#' | awk -F "#" '{print $1}' | tail -1 | sed -e 's|[ a-zA-Z0-9\.\(]*"HAS_PROXY_PORT", "||g' | sed -e 's|")||g')
fi

if [ "" == "$HAS_PROXY_PORT" ]
  then
  PROXY_PORT=$DEFAULT_PROXY_PORT
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PROXY_PORT: $PROXY_PORT"; fi



# setting the hang timeout settings
HANG_TIMEOUT=$HANG_TIMEOUT
if [ "" == "$HANG_TIMEOUT" ]
  then
  HANG_TIMEOUT=$(grep "HangTimeout = " $LUA_FILE | grep -v '^#' | awk -F "#" '{print $1}' | tail -1 | sed -e 's| *HangTimeout = ||' | sed -e 's|"||g')
fi
if [ "" == "$HANG_TIMEOUT" ]
  then
  HANG_TIMEOUT=$DEFAULT_HANG_TIMEOUT
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HANG_TIMEOUT: $HANG_TIMEOUT"; fi



# setting the hang count settings
HANG_COUNT=$HANG_CYCLES
if [ "" == "$HANG_COUNT" ]
  then
  HANG_COUNT=$(grep "HangCycles = " $LUA_FILE | grep -v '^#' | awk -F "#" '{print $1}' | tail -1 | sed -e 's| *HangCycles = ||' | sed -e 's|"||g')
fi
if [ "" == "$HANG_COUNT" ]
  then
  HANG_COUNT=$DEFAULT_HANG_COUNT
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HANG_COUNT: $HANG_COUNT"; fi



# Setting the VM_NAME
VM_NAME=$HAS_DOMAIN_NAME
if [ "" == "$VM_NAME" ]
  then
  VM_NAME=$(grep "DomainName = " $LUA_FILE | grep -v '^#' | awk -F "#" '{print $1}' | tail -1 | sed -e 's| *DomainName = ||g' | sed -e 's|"||g')
fi

if [ "" == "$VM_NAME" ]
  then
  VM_NAME=$DEFAULT_VM_NAME
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: VM_NAME: $VM_NAME"; fi


###################################################################################
### End of - Parsing ##############################################################
###################################################################################

###################################################################################
### Environment Gathering #########################################################
###################################################################################

# grabbing network information
x=$(/sbin/ifconfig) && x=${x#*xenbr-} && y=${x%% *} && z=${x#*HWaddr } && z=${z%% *} && x=${x#*inet addr:} && x=${x%% *}
HOST_NIC=xenbr-$y
HOST_IP_ADDR=$x
HOST_MAC=$z
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HOST_NIC: $HOST_NIC"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HOST_IP_ADDR: $HOST_IP_ADDR"; fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: HOST_MAC: $HOST_MAC"; fi



# grabbing genxfsim package info
GENXFSIM_VER=$(yum info genxfsim | grep -m 1 -e Version | sed -e 's/Version *: //g')
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: GENXFSIM_VER: $GENXFSIM_VER"; fi



# grabbing the xen package info
XEN_VER=$(yum info xen | grep -m 1 -e Release | sed -e 's/Release *: //g')
XEN_VER=$(yum info xen | grep -m 1 -e Version | sed -e 's/Version *: //g')-$XEN_VER
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: XEN_VER: $XEN_VER"; fi



# Getting Fulsim Version
if [ ! "" == "$FULSIM_PATH" ]
  then
  if [ -e "$FULSIM_PATH/AubLoad.exe" ]
    then
    FULSIM=`wine $FULSIM_PATH/AubLoad.exe -v | tail -1 ` && FULSIM=${FULSIM#*Version} && FULSIM=`(echo $FULSIM | sed -e 's/-[A-Z][a-z0-9A-Z: ]*//g')`
  fi

  if [ -e "$FULSIM_PATH/AubLoad" ]
    then
    FULSIM=`$FULSIM_PATH/AubLoad -v | tail -1 ` && FULSIM=${FULSIM#*Version} && FULSIM=`(echo $FULSIM | sed -e 's/-[A-Z][a-z0-9A-Z: ]*//g' | sed -e 's/ [A-Z][a-z0-9A-Z: ]*//g')`
  fi
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: FULSIM: $FULSIM"; fi
fi



# Grabbing the Loaded Model information
#   If there's more than one "runtimeserver" + "debug_qt", script willl pick the last one
#   Legacy interactive mode
if [ "$TBX_SERVER_IP" != "192.168.1.101" ]
  then
  if [ "$TBX_SERVER_IP" != "127.0.0.1" ]
    then
    MODEL_X=`ssh -o LogLevel=ERROR -o StrictHostKeyChecking=no $FILT_IP -F $AUTO_PATH/quicktur/ssh2_config "ps -ef" | grep runtimeserver | grep debug_qt | tail -1`
    #   Netbatch interactive mode
    if [ "" == "$MODEL_X" ]
      then
      MODEL_X=`ssh -o LogLevel=ERROR -o StrictHostKeyChecking=no $FILT_IP -F $AUTO_PATH/quicktur/ssh2_config "ps -ef" | grep runtimeserver | tail -1` && MODEL_X=${MODEL_X#*session} && MODEL_X=`echo $MODEL_X | sed -e 's/_[0-9a-z]*_filt[0-9a-zA-Z/.]*//g' | sed -e 's/_[0-9a-z]*_fmev[0-9a-zA-Z/.]*//g'`
    fi
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: MODEL_X: $MODEL_X"; fi

    # this works as long as the model string appears right before "/debug_qt"
    # remove that and the rest of the string
    MODEL=$(echo $MODEL_X | sed -e 's/\/debug_qt.*//g')

    # As long as we find "/", remove it and any preceeding string
    while [ "" != "$(echo $MODEL | grep '/')" ] 
    do
      MODEL=$(echo $MODEL | sed -e 's/.*\///g') 
    done
  fi
fi
# the model string (actually the folder name) should be what's left
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: MODEL: $MODEL"; fi



# Grabbing the Emulator Name
if [ "" != "$MODEL" ]
  then
  FILT_EMULATOR_NAME=$(echo $FILT_NAME | awk -F "." '{print $1}')
  EMULATOR_NAME=`ssh -o LogLevel=ERROR -o StrictHostKeyChecking=no $FILT_IP -F $AUTO_PATH/quicktur/ssh2_config "emustatus emus | grep $FILT_EMULATOR_NAME" | awk -F " " '{print $1}'`
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: EMULATOR_NAME: $EMULATOR_NAME"; fi
fi



# Grabbing TBX Server Version
if [ "" != "$MODEL" ]
  then
  TBX_SERVER_VERSION=`ssh -o LogLevel=ERROR -o StrictHostKeyChecking=no $FILT_IP -F $AUTO_PATH/quicktur/ssh2_config "/nfs/fm/disks/fm_cse_n22500/sle53/sle.arch/TSERVER/bdw/gt/tcp_app/latest/test/tcp_app.sh /nfs/fm/disks/fm_cse_n22500/sle53/sle.arch/TSERVER/bdw/gt/tcp_app/latest/test/tbxversion.py -ip $FILT_IP" | grep "inner_var" | awk -F "=" '{print $2}'`
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: TBX_SERVER_VERSION: $TBX_SERVER_VERSION"; fi
fi

###################################################################################
####End of - Gathering of Environment #############################################
###################################################################################

###################################################################################
### Terminating Previous Sessions #################################################
###################################################################################

# kill prior runs if any using 'xm dest' 
kill_vm $VM_NAME

# killing the left over processes of genxfsim
kill_genxfsim

###################################################################################
### End of - Terminating Previous Sessions ########################################
###################################################################################

###################################################################################
### Mouting OS Partition - PHY VAR ################################################
###################################################################################

# This is hard drive image location from Fedora pov
#   As long as we find "/", remove it and any preceeding string
IM_PART="$PHY"
while [ "" != "$(echo $IM_PART | grep '/')" ] 
do
  IM_PART=$(echo $IM_PART | sed -e 's/.*\///g') 
done
  # Use default if there's nothing left
if [ "" == "$IM_PART" ]
  then
  IM_PART=$DEFAULT_IM_PART
fi
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: IM_PART: $IM_PART"; fi



# Assume PHY of /dev/vcg/3d_winB means a PARTITION of /dev/mapper/vcg-3d_winB2 for example
#   Adding extra '-' if needed for the mapper to work properly
PARTITION="${DEFAULT_PARTITION}${IM_PART//-/--}"
#   If the Partition ends with a number, will add the required "p" into the Partition Name
#   Windows OS Partition is usually 2
case ${PARTITION} in
  *[0-9]) PARTITION="${PARTITION}p2";;
  *) PARTITION="${PARTITION}2";;
esac
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PARTITION: $PARTITION"; fi



# Smashing VM_IMAGE and VM_NAME to create the mount point per VM
VM_IMAGE=${VM_IMAGE}_${VM_NAME}



# Creating the image folder if it doesn't exist
if [ ! -d $VM_IMAGE ]
  then
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Creating VM_IMAGE folder"; fi
  mkdir $VM_IMAGE
  chmod 0777 $VM_IMAGE
fi



# Un-mounting the image folder if it's mounted
#    just to make sure that it's mounted to the proper location (specified by this script)
if mount | grep $VM_IMAGE > /dev/null
  then
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Un-mounting of VM_IMAGE folder"; fi
  umount $VM_IMAGE
fi
if mount | grep $VM_IMAGE > /dev/null
  then
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Forcing Un-mountinf of VM_IMAGE folder"; fi
  umount -f $VM_IMAGE
fi



# Breaking the partition mapping, if mapped
if [ -e "${PARTITION}" ]
  then
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Breaking mapping IM_PART"; fi
  /sbin/kpartx -d $PHY
  sleep 2
fi



# kparting the partition (creating partition map)
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: kparting the partition"; fi
/sbin/kpartx -a $PHY > /dev/null



# mounting the image
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Mounting PARTITION VM_IMAGE"; fi
mount $PARTITION $VM_IMAGE -o force

###################################################################################
### End of - Mouting OS Partition - PHY VAR #######################################
###################################################################################

###################################################################################
### Mouting SharedDrive - PHY1 VAR ################################################
###################################################################################

# This is the SharedDrive Partition shared with the VM
#    As long as 'PHY1' is set, the script will continue the mounting process
#    As long as we find "/", remove it and any preceeding string
if [ ! "" == "$PHY1" ]
  then
  IM_PART1="$PHY1"
  while [ "" != "$(echo $IM_PART1 | grep '/')" ] 
  do
    IM_PART1=$(echo $IM_PART1 | sed -e 's/.*\///g') 
  done
    # Use default if there's nothing left
  if [ "" == "$IM_PART1" ]
    then
    IM_PART1="$DEFAULT_IM_PART1"
  fi
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: IM_PART1: $IM_PART1"; fi
fi



# Defining the SharedDrive PARTITION1 VAR
if [ ! "" == "$PHY1" ]
  then
  PARTITION1="${DEFAULT_PARTITION1}${IM_PART1}"
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: PARTITION1: $PARTITION1"; fi
fi



# Defining ShareDrive Mount Point from the pre-defined var
if [ ! "" == "$PHY1" ]
  then
  VM_IMAGE1="${LUCAS_SHARE_PATH}"
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: VM_IMAGE: $VM_IMAGE1"; fi
fi



# Creating the image1 folder if it doesn't exist
if [ ! "" == "$PHY1" ]
  then
  if [ ! -d $VM_IMAGE1 ]
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Creating VM_IMAGE1 folder"; fi
    mkdir -p $VM_IMAGE1
    chmod 0777 $VM_IMAGE1
  fi
fi



# Un-mounting the image1 folder if it's mounted
if [ ! "" == "$PHY1" ]
  then
  if mount | grep $VM_IMAGE1 > /dev/null
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Un-mounting of VM_IMAGE1 folder"; fi
    umount $VM_IMAGE1
  fi
  if mount | grep $VM_IMAGE1 > /dev/null
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Forcing Un-mounting of VM_IMAGE1 folder"; fi
    umount -f $VM_IMAGE1
  fi
fi



# mounting the SharedDrive image
if [ ! "" == "$PHY1" ]
  then
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Mounting PARTITION1 VM_IMAGE1"; fi
  mount $PARTITION1 $VM_IMAGE1 -o force
fi

###################################################################################
### End of Mouting SharedDrive - PHY1 VAR #########################################
###################################################################################

###################################################################################
### Dumping Config info - local ###################################################
###################################################################################

# Creating the local Config Folder
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Dumping Config information to ${CONFG_DIR}"; fi



# If the 'CONFIG_DIR/$STAMP' doesn't exists, script creates both directories.
if [ ! -e $CONFG_DIR/$STAMP ]
  then
  mkdir -p "${CONFG_DIR}/${STAMP}"
fi



# Dumping Config information to the 'CONFIG_DIR'
echo $(date) >> $CONFG_DIR/$STAMP/$CONFG_LOG
echo $MODEL >> $CONFG_DIR/$STAMP/$CONFG_LOG
echo $(yum list genxfsim xen) >> $CONFG_DIR/$STAMP/$CONFG_LOG
cp $VM_START $CONFG_DIR/$STAMP
echo $HOSTNAME > $CONFG_DIR/$STAMP/hostname.txt
echo $HOST_NIC > $CONFG_DIR/$STAMP/vm_nic.txt
echo $HOST_IP_ADDR > $CONFG_DIR/$STAMP/host_ip.txt
echo $HOST_MAC > $CONFG_DIR/$STAMP/host_mac.txt
echo $FILT_NAME > $CONFG_DIR/$STAMP/filt_name.txt
echo $FILT_IP > $CONFG_DIR/$STAMP/filt_ip.txt
echo $EMULATOR_NAME > $CONFG_DIR/$STAMP/emulator_name.txt
echo $MODEL > $CONFG_DIR/$STAMP/model.txt
echo $(date) > $CONFG_DIR/$STAMP/date.txt
echo $FULSIM > $CONFG_DIR/$STAMP/fulsim.txt
echo $FULSIM_PATH > $CONFG_DIR/$STAMP/fulsim_path.txt
echo $GT > $CONFG_DIR/$STAMP/gt_type.txt
echo $LUA_FILE > $CONFG_DIR/$STAMP/start_script.txt
echo $PHY > $CONFG_DIR/$STAMP/phy.txt
echo $VM_NAME > $CONFG_DIR/$STAMP/vm_name.txt
echo $GENXFSIM_VER > $CONFG_DIR/$STAMP/genxfsim.txt
echo $XEN_VER > $CONFG_DIR/$STAMP/xen.txt
echo $TBX_SERVER_VERSION > $CONFG_DIR/$STAMP/tbx_server_version.txt
echo $HANG_TIMEOUT > $CONFG_DIR/$STAMP/hang_timeout.txt
echo $HANG_COUNT > $CONFG_DIR/$STAMP/hang_count.txt
set -o posix ; set > $CONFG_DIR/$STAMP/env_dump.txt
cat $REGINI_FILE > $CONFG_DIR/$STAMP/regini.xml
cat $FILTER_REMAP > $CONFG_DIR/$STAMP/filter_remap.xml
echo \#\!\/bin\/bash > $CONFG_DIR/$STAMP/vm_start.sh
cat $VM_START | grep -v '^#' >> $CONFG_DIR/$STAMP/vm_start.sh

###################################################################################
### End of - Dumping Config info - local ###########################################
###################################################################################

###################################################################################
### Dumping Config info - VM ######################################################
###################################################################################

# Creating the fedora config directory in the VM
#   Deletes any previous host config file
if [ -e $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME ]
  then
  rm -Rf $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME
fi

if [ ! -e $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME ]
  then
  mkdir $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME
fi



# Dumping the fedora config into the vm partition
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Injecting config info into ${VM_IMAGE}/$DEFAULT_HOST_CONFIG_NAME"; fi
if [ -e $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME ]
  then
  echo $HOSTNAME > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/hostname.txt
  echo $HOST_NIC > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/vm_nic.txt
  echo $HOST_IP_ADDR > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/host_ip.txt
  echo $HOST_MAC > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/host_mac.txt
  echo $FILT_NAME > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/filt_name.txt
  echo $FILT_IP > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/filt_ip.txt
  echo $EMULATOR_NAME > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/emulator_name.txt
  echo $MODEL > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/model.txt
  echo $(date) > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/date.txt
  echo $FULSIM > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/fulsim.txt
  echo $FULSIM_PATH > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/fulsim_path.txt
  echo $GT > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/gt_type.txt
  echo $LUA_FILE > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/start_script.txt
  echo $PHY > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/phy.txt
  echo $VM_NAME > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/vm_name.txt
  echo $GENXFSIM_VER > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/genxfsim.txt
  echo $XEN_VER > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/xen.txt
  echo $TBX_SERVER_VERSION > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/tbx_server_version.txt
  echo $HANG_TIMEOUT > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/hang_timeout.txt
  echo $HANG_COUNT > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/hang_count.txt
  set -o posix ; set > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/env_dump.txt
  cat $REGINI_FILE > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/regini.xml
  cat $FILTER_REMAP > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/filter_remap.xml
  echo \#\!\/bin\/bash > $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/vm_start.sh
  cat $VM_START | grep -v '^#' >> $VM_IMAGE/$DEFAULT_HOST_CONFIG_NAME/vm_start.sh
fi

###################################################################################
### Hang Signature Collection #####################################################
###################################################################################

# grabbing hang_sigs if detected and dropping them into TestMinion Folder if exists
#   Looks like TM is deleting the hangs folder in the C:\TestMinion\active\ dir; need to find another way to sunc this
#   For now, storing the hang sigs on the fedora and C:\TestMinion\hangs
#   Looks like the VM is being corrupted when it's being modded like so.
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Processing Hangs"; fi
if [ -e "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" ]
  then
  
  # TM: Pulling out variables out of json objects to get the current status
  if [ -e "$VM_IMAGE/TestMinion/running.json" ]
    then
    TM_CURRENT_LINE=$(grep \"curr_line\" $VM_IMAGE/TestMinion/running.json | awk -F ": " '{print $2}' | sed -e 's/...$//')
    TM_CURRENT_CMD=$(grep \"curr_line_text\" $VM_IMAGE/TestMinion/running.json | awk -F ": " '{print $2}' | sed -e 's/...$//')
    TM_JOB_NAME=$(grep \"name\" $VM_IMAGE/TestMinion/running.json | awk -F ": " '{print $2}' | sed -e 's/...$//' | sed -e 's/"//g')
    TM_CLIENT_NAME=$(grep \"host_name\" $VM_IMAGE/TestMinion/sysinfo.json | awk -F ": " '{print $2}' | sed -e 's/...$//' | sed -e 's/"//g')
    
    # TM: If there is a current test running, creates a job folder locally in the hang_dir
    if [ ! "" == "$TM_CURRENT_LINE" ]
      then
      
      # TM: Creating the Job Folder if it does exists
      if [ ! -e "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/" ]
        then
        mkdir -p "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/"
      fi
      
      # TM: Grabbing Screenshot if exists
      if [ -e "${HANG_DIR}"/${VM_NAME}_${HANG_SCREENSHOT_NAME} ]
        then
        cp -f "${HANG_DIR}"/${VM_NAME}_${HANG_SCREENSHOT_NAME} "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/${TM_CURRENT_LINE}_${STAMP}.jpg"
      fi
      
      # TM: Collecting hang signature into the local hang_dir
      echo $TM_CURRENT_CMD >> "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/${TM_CURRENT_LINE}.log"
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/${TM_CURRENT_LINE}.log"
      unix2dos "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/${TM_CURRENT_LINE}.log"
      cp -f "${HANG_DIR}/${VM_NAME}_${HANG_SCREENSHOT_NAME}" "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/${TM_CURRENT_LINE}_${STAMP}.jpg"
      if [ -e $AUTO_PATH/screenlog.* ]
        then
        cat $AUTO_PATH/screenlog.* >> "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/${TM_CURRENT_LINE}_regmon_output.log"
      fi
      
      # TM: Archiving the Hangs to the rolling Hang Directory
      echo $TM_CURRENT_CMD >> "$HANG_DIR/$HANG_SIGS"
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> $HANG_DIR/$HANG_SIGS
      echo $TM_CURRENT_CMD >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.archive"
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.archive"
      unix2dos "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.archive"
      if [ -e $AUTO_PATH/screenlog.* ]
        then
        cat $AUTO_PATH/screenlog.* >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}_regmon_output.archive"
      fi
      
      # TM: fixing permissions for pulling the logs off remotely
      chmod 0777 -R "/${HANG_DIR}/${VM_NAME}/${TM_JOB_NAME}/"
      chmod 0777 -R "/${HANG_DIR}/${VM_NAME}/"

      # TM: Collecting the hang signature data into the TM share (if exists)
      if [ -e "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}" ]
        then

        # TM: Creating the hang directory in TM if it doesn't exist
        if [ ! -e "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs" ]
          then
          mkdir -p "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs"
        fi

        # TM: Collecting the the hang signature and hang image file
        echo $TM_CURRENT_CMD >> "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs/${TM_CURRENT_LINE}.log"
        cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs/${TM_CURRENT_LINE}.log"
        cp -f "${HANG_DIR}/${VM_NAME}_${HANG_SCREENSHOT_NAME}" "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs/${TM_CURRENT_LINE}_${STAMP}.jpg"
        unix2dos "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs/${TM_CURRENT_LINE}.log"
        if [ -e $AUTO_PATH/screenlog.* ]
          then 
          cat $AUTO_PATH/screenlog.* >> "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs/${TM_CURRENT_LINE}_regmon_output.log"
          unix2dos "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/inbox/${TM_JOB_NAME}/hangs/${TM_CURRENT_LINE}_regmon_output.log"
        fi
      fi
    fi
  else
    # awrap: Archiving the Hangs to the rolling Hang Directory
    if [ -e $VM_IMAGE/$WRAPPER_LOG ]
      then
      cat $VM_IMAGE/$WRAPPER_LOG | grep "# running" | tail -1 |  >> $HANG_DIR/$HANG_SIGS
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> $HANG_DIR/$HANG_SIGS

      # awrap processing (legacy), this data is only needed if TM is not being used.
      cat $VM_IMAGE/$WRAPPER_LOG | grep "# running" | tail -1 |  >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.awrap"
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.awrap"
      unix2dos "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.awrap"
    fi
  fi

  # LUCAS: Collecting Hangs for Lucas environment, ONLY if the SharedDrive is Mapped. (This script pulls that information from HAS_DISK1)
  if [ ! "" == "$PHY1" ]
    then

    # LUCAS: Getting the Lucas Status file
    LUCAS_STATUS_FILE=
    LUCAS_STATUS_FILE=$(find ${VM_IMAGE1} -name '*.status')

    if [ ! "" == "$LUCAS_STATUS_FILE" ]
      then
      # LUCAS: Pulling Test and Directory of the running test from the status file
      LUCAS_CURRENT_DIR=
      LUCAS_CURRENT_DIR=$(dirname ${LUCAS_STATUS_FILE})
      LUCAS_CURRENT_TEST=
      LUCAS_CURRENT_TEST=$(cat ${LUCAS_STATUS_FILE})

      # LUCAS: Dumping the Hang Signature into file
      echo $LUCAS_CURRENT_TEST >> "${LUCAS_CURRENT_DIR}/${LUCAS_CURRENT_TEST}_hang_${STAMP}.log"
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> "${LUCAS_CURRENT_DIR}/${LUCAS_CURRENT_TEST}_hang_${STAMP}.log"
      unix2dos "${LUCAS_CURRENT_DIR}/${LUCAS_CURRENT_TEST}_hang_${STAMP}.log"

      # LUCAS: Archiving the Hangs to the rolling Hang Directory
      echo $LUCAS_CURRENT_TEST >> $HANG_DIR/$HANG_SIGS
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> $HANG_DIR/$HANG_SIGS
      echo $LUCAS_CURRENT_TEST >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.archive"
      cat "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" >> "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.archive"
      unix2dos "$HANG_DIR/${VM_NAME}_${HANG_SIGS}.archive"

      # LUCAS: Copying the hang screenshot if generated
      if [ -e "${HANG_DIR}"/${VM_NAME}_${HANG_SCREENSHOT_NAME} ]
        then
        cp -f "${HANG_DIR}"/${VM_NAME}_${HANG_SCREENSHOT_NAME} "${LUCAS_CURRENT_DIR}/${LUCAS_CURRENT_TEST}_hang_${STAMP}.jpg"
      fi
      
      # LUCAS: Grabbing the remon_output.log 
      if [ -e $AUTO_PATH/screenlog.* ]
        then
        cat $AUTO_PATH/screenlog.* >> "${LUCAS_CURRENT_DIR}/${LUCAS_CURRENT_TEST}_regmon_output_${STAMP}.log"
        unix2dos "${LUCAS_CURRENT_DIR}/${LUCAS_CURRENT_TEST}_regmon_output_${STAMP}.log"
      fi
    fi
  fi
fi



# archiving hang collection data
#    This purges the hang signature to make sure that the new hang signatures are correctly created.
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Purging stale hang collection data"; fi
if [ -e "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" ]
  then
  rm -f "$HANG_DIR/${VM_NAME}_${HANG_SIGS}"
  rm -f "${HANG_DIR}"/${VM_NAME}_${HANG_SCREENSHOT_NAME}
  if [ -e $AUTO_PATH/screenlog.* ]
   then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: $AUTO_PATH/screenlog"; fi
    rm -f $AUTO_PATH/screenlog.*
  fi
fi

###################################################################################
### End of - Hang Signature Collection#############################################
###################################################################################

###################################################################################
### Final Pre-Launch Processes ####################################################
###################################################################################

# Creating FILT and GT_TYPE files needed by hang_sig.sh
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Creating ${FILT_IP_FILE}, ${GT_TYPE_FILE}, ${PROXY_PORT_FILE}"; fi
echo "${FILT_IP}" > ${FILT_IP_FILE}
echo "${GT}" > ${GT_TYPE_FILE}
echo "${PROXY_PORT}" > ${PROXY_PORT_FILE}



# Un-mounting VM_IMAGE
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Un-mounting $VM_IMAGE"; fi
umount $VM_IMAGE



# Breaking down partition mapping for $VM_IMAGE
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Breaking down the partition mapping for $VM_IMAGE"; fi
sleep 1
  /sbin/kpartx -d $PHY
sleep 1



# Un-mounting the $VM_IMAGE1
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Un-mounting $VM_IMAGE1"; fi
if [ ! "" == "$VM_IMAGE1" ] 
  then 
  umount -f $VM_IMAGE1
fi



# delete log files if flag set
#   This MAY take a while if the log files are huge
if [ -e $PURGE_LOG_FLAG ]
  then
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Purging logs $PURGE_LOG_FLAG"; fi
  rm -f $LOG_PATH/*.log
  sleep 5
fi



# deleting any previous configs in the /has/config dir
#    This is to prevent the mixup of future created temp_conf files
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Deleting /has/config/temp_conf_*"; fi
if ls /has/config/temp_conf_* > /dev/null
  then
  rm -f /has/config/temp_conf_*
fi

# Deleting them from the $AUTO_PATH directory
if [ -e $AUTO_PATH/temp_conf_$VM_NAME.txt ]
  then
  rm -f $AUTO_PATH/temp_conf_$VM_NAME.txt
fi


# killing any previous have detection that was running for that VM session
REG_MON_PID=$(ps -ef | grep $VM_NAME | grep SCREEN | grep reg_mon | awk -F " " '{print $2}')
if [ ! "" == "$REG_MON_PID" ]
  then
  echo "Killing Previous Hang Detection for ${VM_NAME} PID:${REG_MON_PID}"
  kill -9 ${REG_MON_PID}
fi



# Killing and tail process of the regmon_output.log
for i in $( ps -ef | grep "tail -f $AUTO_PATH/screenlog." | awk -F " " '{print $2}')
 do 
 kill -9 $i
done



# removing any launch flag that could be set by cron_mon (we don't need this since we're already launching)
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Removing launch.flag"; fi
if [ -e "$AUTO_PATH/launch.flag" ]
  then
  echo "INFO: Removing previous launch flag"
  rm -f "$LAUNCH_FLAG"
fi

###################################################################################
### End of - Final Pre-Launch Processes ###########################################
###################################################################################

###################################################################################
### Starting the VM (finally) #####################################################
###################################################################################

# trying to start the VM
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Starting VM"; fi
while [ "$STARTED" == "FALSE" ]
do
  # start the VM
  $VM_START &
  export VM_START_PID=$!
  if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: VM_START_PID: $VM_START_PID"; fi

  # only do this loop in monitoring mode (script can be used directly to start VM)
  if [ -e $DO_FLAG ]
    then
    sleep $DEFAULT_SCRIPT_SLEEP_TIME
    check_genxfsim
  else
    STARTED="TRUE"
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: WHOOP! The VM is now Started :)"; fi
  fi  

  # launching the hang detection if monitoring is enabled
  if [ -e $DO_FLAG ]
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Launching Hang Detection"; fi
    if [ -e $AUTO_PATH/reg_mon ]
      then
        if [ "$REG_MON_TBX_SERVER_DEAD_KILL_SWITCH" == "TRUE" ]
          then
          screen -d -S $VM_NAME -L -m $AUTO_PATH/reg_mon -i $FILT_IP -g $GT -p $PROXY_PORT -v $HANG_TIMEOUT -c $HANG_COUNT -k 45 -x "$AUTO_PATH/hung_vm.sh $VM_NAME" -s "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" -b "$AUTO_PATH/stop_mon.sh $TBX_SERVER_IP" -m
        else
          screen -d -S $VM_NAME -L -m $AUTO_PATH/reg_mon -i $FILT_IP -g $GT -p $PROXY_PORT -v $HANG_TIMEOUT -c $HANG_COUNT -k 45 -x "$AUTO_PATH/hung_vm.sh $VM_NAME" -s "$HANG_DIR/${VM_NAME}_${HANG_SIGS}" -m
        fi
      SCREEN_ID=$(ps -ef | grep "reg_mon" | grep "SCREEN" | grep "$VM_NAME" | awk -F " " '{print $2}')
      # tailing the screen process
      if [ -e $AUTO_PATH/screenlog.* ]
        then
        if [ -e "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}" ]
          then 
          tail -f $AUTO_PATH/screenlog.* >> "$TM_SHARE_PATH/clients/${TM_CLIENT_NAME}/regmon_output.log" &
        fi
        tail -f $AUTO_PATH/screenlog.* >> $CONFG_DIR/$STAMP/regmon_output.log &
      fi
      if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: reg_mon screen info is: ${SCREEN_ID}"; fi
    fi
  fi

  # Cleaning up any previous dead screen sessions
  if screen -ls | grep Dead > /dev/null
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Cleaning dead screens"; fi
    screen -wipe 
  fi

  # grabbing any temp_conf files that are generated
  #   Placing one into the CONFIG_DIR and another into the AUTO_PATH
  if ls /has/config/temp_conf_* &> /dev/null
    then
    if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: Grabbing temp_conf files"; fi
    cp /has/config/temp_conf_* $AUTO_PATH/temp_config_$VM_NAME.txt
    cp /has/config/temp_conf_* $CONFG_DIR/$STAMP/temp_config_$VM_NAME.txt
  fi

done
if [ ! "" == "$DEBUG_FLAG" ] ; then echo "DEBUG: ########### Script Completed, Exiting ############"; fi

###################################################################################
### End of Starting the VM (finally) ##############################################
###################################################################################
