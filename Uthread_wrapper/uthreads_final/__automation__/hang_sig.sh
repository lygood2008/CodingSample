#!/bin/bash

# importing Manual Automation Config
source config.sh


# if GT value is blank, prompt for value
GT=`cat "${GT_TYPE_FILE}"`
if [ "" == "$GT" ]
  then
  echo "INFO: Unable to locate the GT file OR GT File is empty"
  read -p "PROMPT: Please Enter the GT Value:"
  GT=$REPLY
fi

# if GT value is blank, set GT value to 2 as default
if [ "" == "$GT" ]
  then
  GT="2"
fi

# grabbing the PROXY Port info
PROXY_PORT=`cat "${PROXY_PORT_FILE}"`
if [ "" == "$PROXY_PORT" ]
  then
  echo "INFO: PROXY PORT not defined"
  read -p "PROMPT: Please Enter the Proxy Port:"
  PROXY_PORT=$REPLY
fi

# if PROXY_PORT value is blank, set PROXY_PORT value to 4321 as default
if [ "" == "$PROXY_PORT" ]
  then
  PROXY_PORT="4321"
fi

# grabbing the FILT IP address
FILT_IP=`cat "${FILT_IP_FILE}"`
if [ "" == "$FILT_IP" ]
  then
  echo "INFO: Unable to locate the Filt ip file OR the file is empty"
  read -p "PROMPT: Please Enter the Filt IP Address:"
  FILT_IP=$REPLY
fi

# If FILT_IP value is set, running the hang sig collection
if [ "" == "$FILT_IP" ]
  then
  echo "ERROR: No Filt IP spceified, exiting..."
else
  if [ -e $AUTO_PATH/reg_mon ]
    then
    $AUTO_PATH/reg_mon -i $FILT_IP -g $GT -p $PROXY_PORT -v 0 -c 0 -s "${HANG_DIR}/${HANG_SIGS}" -m
  else
    echo "ERROR: Cannot locate ${AUTO_PATH}/reg_mon"
  fi
fi