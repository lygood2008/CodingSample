#!/bin/bash
source config.sh

# 	INTEL CONFIDENTIAL
#	
#	Copyright 2011 Intel Corporation All Rights Reserved. 
#
#	The source code contained or described herein and all documents related to the source code ("Material") are 
#	owned by Intel Corporation or its suppliers or licensors. Title to the Material remains with Intel Corporation 
#	or its suppliers and licensors. The Material contains trade secrets and proprietary and confidential 
#	information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright and 
#	trade secret laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, 
#	published, uploaded, posted, transmitted, distributed, or disclosed in any way without Intel�s prior express 
#	written permission.
#
#	No license under any patent, copyright, trade secret or other intellectual property right is granted to or 
#	conferred upon you by disclosure or delivery of the Materials, either expressly, by implication, 
#	inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and 
#	approved by Intel in writing.


# Set Up Environment Variables
export HAS_DISK='phy:/dev/vcg/3d_winB,hda,w' 
#export HAS_DISK='phy:/dev/vcg/win8-1-64bit,hda,w' 
echo $HAS_DISK
export HAS_SCRIPT=/has/scripts/start_bxt_pipegt.lua
echo $HAS_SCRIPT
export HAS_SYSTEM_MEM_SIZE=3072
echo $HAS_SYSTEM_MEM_SIZE
export HAS_STOLEN_MEM_SIZE=64
echo $HAS_STOLEN_MEM_SIZE
export HAS_NIC="type=ioemu,bridge=xenbr-p2p1,mac=$(ifconfig|grep xenbr|sed 's/.*HWaddr //'|od -An -N6 -tx1|sed -e 's/^  *//' -e 's/  */:/g' -e 's/:$//' -e 's/^\(.\)[13579bdf]/\10/')"
echo $HAS_NIC
export HAS_GT_TYPE=2
echo $HAS_GT_TYPE
export HAS_TMP_STRIDE=skl
echo $HAS_TMP_STRIDE
export HAS_ARCH=skl;
echo HAS_ARCH $HAS_ARCH
export HAS_PCH_DEVICE_ID=0x99EF;
echo HAS_PCH_DEVICE_ID $HAS_PCH_DEVICE_ID
export REGINI=/mnt/share/bxt_regini.xml
#export REGINI=/mnt/share/ww43c_regini.xml
echo REGINI $REGINI

# Sim fulsim settings
export HAS_FULSIM_PATH=/fulsim/lin/bxt

#QT124G2
#export TBX_SERVER_IP=10.80.203.49

#QT124G1
#export TBX_SERVER_IP=10.80.203.45

#QT156m_2
#export TBX_SERVER_IP=10.80.62.4

#QT152m_3
export TBX_SERVER_IP=10.80.62.182 # filt2074.fm.intel.com

#QT153m_3
#export TBX_SERVER_IP=filt2070.fm.intel.com

#filt2061
#export TBX_SERVER_IP=10.80.62.156

#QT159M_0
#export TBX_SERVER_IP=10.80.62.204


echo $TBX_SERVER_IP
export HAS_SERIAL='tcp::80,server,nodelay,nowait'
echo $HAS_SERIAL

#a0 settings
#export HAS_FULSIM_STEPPING=a0
#echo HAS Fulsim stepping $HAS_FULSIM_STEPPING
#export HAS_REV_ID=0x0
#export HAS_REVISION_ID=0x0
#echo RevID $HAS_REV_ID
#echo RevisionID $HAS_REVISION_ID

echo timeout settings
export HANG_TIMEOUT=60
echo $HANG_TIMEOUT
export HANG_CYCLES=10
echo $HANG_CYCLES
export HANG_POLL_MMIO='0x2030,0x7100,0xe164,0x206c'
echo $HANG_POLL_MMIO
export HANG_PATTERN_MMIO='0x2030,0x2034,0x2068,0x206c,0x2070,0x2140,0x7100,0xe160,0xe164,0x12030,0x12034,0x1a030,0x1a034,0x22030,0x22034'
echo $HANG_PATTERN_MMIO		
export HANG_APP_NAME='/mnt/share/__automation__/hung_vm.sh'
echo $HANG_APP_NAME
export IDLE_APP_NAME='/mnt/share/__automation__/hung_vm.sh'
echo $IDLE_APP_NAME
#export HANG_TCP_PORT
#echo $HANG_TCP_PORT

#VNC Mouse fix
export HAS_USB='tablet'

export INTR_EMUL_SUPP='1'
echo $INTR_EMUL_SUPP
export CNCRY_INTR_EMUL_SUPP='1'
echo $CNCRY_INTR_EMUL_SUPP
export RUN_LIST_WA='1'
echo $RUN_LIST_WA
export HAS_RUN_LIST_WA='1'
echo $HAS_RUN_LIST_WA

# Keep the HAS config file
export HAS_KEEP_TEMPCONFIG=1
echo HAS_KEEP_TEMPCONFIG $HAS_KEEP_TEMPCONFIG

# 9200 settings
#export FLIP_OFFSET='0x1802c4'
echo FLIP_OFFSET $FLIP_OFFSET
export CS_SERIAL='0'
echo CS_SERIAL $CS_SERIAL

# FLR Settings for BXT Models (WW48p2+ Models set to 0)
export FLR_ENABLE=0


# SKL/BXT Display plane change - Saumyajyoti Mukherjee.  Must add "HKLM\SYSTEM\CurrentControlSet\Services\Gfx_sim\HASDisplayConfig 0x80000042" into the registry also.
# The HAS flag is to switch between plane 1 and Plane 2 as primary. 
# We are now removing the plane 2 related hooks from driver so only plane 1 will be used as the primary plane.
export HAS_PRIMARY_PLANE1='skl'

# Start HAS 3
if [ "$HAS_SCRIPT" != "" ]
then
	/has/bin/genxfsim-dm -S $HAS_SCRIPT
else
	/has/bin/genxfsim-dm
fi

