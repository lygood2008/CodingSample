#!/bin/bash
# by ALEX VRONSKIY 04/11/2014

echo "====== HAS3 Manual Automation Upgrade Script  ======="



# Making sure this script is running as root
if [ `whoami` != root ]; then
	echo ERROR: This script MUST run as root or using sudo
	exit
fi



# importing Manual Automation Config
source ./config.sh



# Adding the AUTO_PATH into bashrc
BASH_CHECK=`cat ~/.bashrc | grep ${AUTO_PATH}`
if [ "" == "$BASH_CHECK" ]
then
	echo "INFO: Creating path: ${AUTO_PATH}"
	echo "if ! echo \$PATH | grep '${AUTO_PATH}' > /dev/null; then PATH=\${PATH}:${AUTO_PATH}; fi" >> ~/.bashrc
	echo "export PATH" >> ~/.bashrc
fi



# Installing needed packages for the HAS3 environment
echo "INFO: Installing/Updating needed packages"
/usr/bin/which screen || sudo yum install -y screen
/usr/bin/which scrot || sudo yum install -y scrot

# Summary of packages and why we need them
# screen - used to hide terminals in the background (used by reg_mon) so you can recall them.
# scrot - used to capture screen shots of hang conditions



# Adding the scripts needed for state pulling of the manual automation environment
# MUST HAVE FOR AUTOMATION
CRONCHECK=`crontab -u root -l | grep "SHELL=/bin/bash"`
if [ "" == "$CRONCHECK" ]
then
	echo "INFO: Adding go_mon.sh into the crontab"
	crontab -u root -l | grep -v "cron_mon" > ${AUTO_PATH}/crontab_l
	echo "SHELL=/bin/bash" >> ${AUTO_PATH}/crontab_l
	echo "PATH=${PATH}:${AUTO_PATH}" >> ${AUTO_PATH}/crontab_l
	echo "0,5,10,15,20,25,30,35,40,45,50,55 * * * *       ${AUTO_PATH}/cron_mon.sh >> /tmp/cront_log.txt 2>&1" >> ${AUTO_PATH}/crontab_l
	echo "1,6,11,16,21,26,31,36,41,46,51,56 * * * *       ${AUTO_PATH}/cron_mon_1.sh >> /tmp/cront_log.txt 2>&1" >> ${AUTO_PATH}/crontab_l
	sudo crontab ${AUTO_PATH}/crontab_l
	rm -f ${AUTO_PATH}/crontab_l
fi



# reloading the shell
bash

