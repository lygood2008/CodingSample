#!/bin/bash
# by ALEX VRONSKIY 04/10/2014

echo "====== HAS3 Manual Automation Installation Script  ======="



# Making sure this script is running as root
if [ `whoami` != root ]; then
	echo ERROR: This script MUST run as root or using sudo
	exit
fi



# importing Manual Automation Config
source ./config.sh



# Defining AUTO_PATH 
#echo "INFO: Defining the Install Directory"
#read -p "PROMPT: Please enter installation dir (empty for default):"
#AUTO_PATH=$REPLY



# Setting default AUTO_PATH if not defined
if [ "" = "${AUTO_PATH}" ]
then
	echo "INFO: Defaulting installation path to /mnt/share/__automation__"
	AUTO_PATH="/mnt/share/__automation__"
fi



# Copying Files to the AUTO_PATH directory
if [ ! -e "${AUTO_PATH}" ]
then
	echo "INFO: Creating path: ${AUTO_PATH}"
	mkdir -p "${AUTO_PATH}"
fi
cp -r * "${AUTO_PATH}"



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



# Patching startup scripts with the correct VARs
echo "INFO: Changing the MAC and xenbr in the scripts to match the system"
x=$(ifconfig) && x=${x#*xenbr-} && y=${x%% *} && z=${x#*HWaddr } && z=${z%% *} && x=${x#*inet addr:} && x=${x%% *}
NEW_NIC=xenbr-$y
BRIDGE_NUMBER=$y
NEW_MAC=`(date; cat /proc/interrupts) | md5sum | sed 's/[13579bdf]//g' | sed -r 's/^(.{12}).*$/\1/; s/([0-9a-f]{2})/\1:/g; s/:$//;'`
for file in ${AUTO_PATH}/*
do
	if [ -f "$file" ]
	then
		sed -i "s|xenbr-p2p1|${NEW_NIC}|g" $file
		sed -i "s|00:BA:CA:12:42:FE|${NEW_MAC}|g" $file
	fi
done



# Adding the scripts needed for state pulling of the manual automation environment
# MUST HAVE FOR AUTOMATION
CRONCHECK=`crontab -u root -l | grep ${AUTO_PATH}`
if [ "" == "$CRONCHECK" ]
then
	echo "INFO: Adding go_mon.sh into the crontab"
	crontab -u root -l > ${AUTO_PATH}/crontab_l
	echo "SHELL=/bin/bash" >> ${AUTO_PATH}/crontab_l
	echo "PATH=${PATH}:${AUTO_PATH}" >> ${AUTO_PATH}/crontab_l
	echo "0,5,10,15,20,25,30,35,40,45,50,55 * * * *       ${AUTO_PATH}/cron_mon.sh >> /tmp/cront_log.txt 2>&1" >> ${AUTO_PATH}/crontab_l
	echo "1,6,11,16,21,26,31,36,41,46,51,56 * * * *       ${AUTO_PATH}/cron_mon_1.sh >> /tmp/cront_log.txt 2>&1" >> ${AUTO_PATH}/crontab_l
	sudo crontab ${AUTO_PATH}/crontab_l
	rm -f ${AUTO_PATH}/crontab_l
fi



# Adding the SSH pubkey to be able to authicate with the filt\fmev machine
# MUST for hang detection; reg_mon needs to be able to hook into the machine to read the registers.
if [ ! -e /root/.ssh ] 
then
	echo "INFO: Setting SSH pubkey"
	chown root ${AUTO_PATH}/quicktur/id_rsa*
	chgrp root ${AUTO_PATH}/quicktur/id_rsa*
	chmod 0777 ${AUTO_PATH}/quicktur/id_rsa
	mkdir /root/.ssh
	cp ${AUTO_PATH}/quicktur/id_rsa /root/.ssh/
	chmod 600 /root/.ssh/id_rsa
fi
# Setting execute permissions on the scripts
chmod 0777 ${AUTO_PATH}/*



# reloading the shell
bash
