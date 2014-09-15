#!/bin/bash

# importing Manual Automation Config
source config.sh

# set the VM name
HUNG_VM_NAME=$*

# Creates the hang directory if not exists
if [ ! -e "${HANG_DIR}/archive/" ]
	then
	mkdir -p "${HANG_DIR}/archive/"
fi

# Making sure that the screen is turned on for the screenshot
if [[ $DISPLAY == :0* ]]
	then
	runuser -l $(who | grep tty2 | awk -F " " '{print $1}') -c "source $AUTO_PATH/config.sh;gnome-screensaver-command -d;xset dpms force on"
	sleep 1
fi

# grabbing the screen shot using scrot
#   Screenshot will be overwritten if already exists by default
scrot "${HANG_DIR}"/${HUNG_VM_NAME}_${HANG_SCREENSHOT_NAME}

# archive a copy of the screenshot with the timestamp
cp "${HANG_DIR}"/${HUNG_VM_NAME}_${HANG_SCREENSHOT_NAME} "${HANG_DIR}"/archive/${HUNG_VM_NAME}_${HANG_STAMP}_${HANG_SCREENSHOT_NAME}

# Setting the Launch Flag
touch $LAUNCH_FLAG
