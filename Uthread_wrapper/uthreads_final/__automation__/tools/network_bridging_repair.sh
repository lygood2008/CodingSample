#!/bin/bash
# By: ALEX VRONSKIY | avronskx@intel.com | 06/09/2014 | VER 0.0.1

# Making sure this script is running as root
if [ `whoami` != root ]
then
	echo ERROR: This script MUST run as root or using sudo
	exit
fi



# Pulling the MAC Address
CURRENT_MAC_ADDR=$(dmesg -t | grep eth | tac | tail -n 1 | awk -F ") " '{print $2}')
echo "INFO: CURRENT_MAC_ADDR: $CURRENT_MAC_ADDR"



# Pulling the xenbr adapter name
CURRENT_XENBR_NAME=$(dmesg -t | grep eth | grep renamed | awk -F " " '{print $7}')
echo "INFO: CURRENT_XENBR_NAME: $CURRENT_XENBR_NAME"



# backing up the previous config files in-case something goes wrong (if they exist)
if ls /etc/sysconfig/network-scripts/ifcfg-* > /dev/null
then 
	echo "INFO: Backing up previous ifcfg files"
	for file in $(find /etc/sysconfig/network-scripts/ -name 'ifcfg-*' | grep -v "ifcfg-lo")
	do
		echo "INFO: Renaming \"$file\" to \"backup_$file.backup\""
		mv -f "$file" "$file.backup"
	done
fi



# Creating new files
echo "INFO: Creating a new ifcfg-$CURRENT_XENBR_NAME file"
echo "IPV6INIT=\"yes\"
DHCP_HOSTNAME=\"$HOSTNAME\"
HWADDR=\"$CURRENT_MAC_ADDR\"
BOOTPROTO=\"dhcp\"
DEVICE=\"$CURRENT_XENBR_NAME\"
ONBOOT=\"yes\"
BRIDGE=xenbr-$CURRENT_XENBR_NAME
NM_CONTROLLED=no" > /etc/sysconfig/network-scripts/ifcfg-$CURRENT_XENBR_NAME



# although creating this file is redundant, we would still like to create it
echo "INFO: Creating a new ifcfg-xen-$CURRENT_XENBR_NAME file"
echo "
DEVICE=xenbr-$CURRENT_XENBR_NAME
TYPE=Bridge
BOOTPROTO=dhcp
ONBOOT=yes
DELAY=0
NM_CONTROLLED=NO
PERSISTENT_DHCLIENT=yes
" > /etc/sysconfig/network-scripts/ifcfg-xen-$CURRENT_XENBR_NAME



# restarting the network.  Reboot is recommended, but not required
service network restart


# reboot prompt
read -p "PROMPT: Would you like to reboot? " -n 1 -r
echo 
if [[ $REPLY =~ ^[Yy]$ ]]
then
	echo "INFO: Rebooting the system"
	/sbin/reboot
fi
