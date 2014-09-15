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
#	published, uploaded, posted, transmitted, distributed, or disclosed in any way without Intel’s prior express 
#	written permission.
#
#	No license under any patent, copyright, trade secret or other intellectual property right is granted to or 
#	conferred upon you by disclosure or delivery of the Materials, either expressly, by implication, 
#	inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and 
#	approved by Intel in writing.

export HAS_DISK='phy:/dev/vcg/3d_winB,hda,w' 
export HAS_SCRIPT=/has/scripts/start_cnl_pipegt.lua
export HAS_NIC="type=ioemu,bridge=xenbr-p2p1,mac=$(ifconfig|grep xenbr|sed 's/.*HWaddr //'|od -An -N6 -tx1|sed -e 's/^  *//' -e 's/  */:/g' -e 's/:$//' -e 's/^\(.\)[13579bdf]/\10/')"
#export HAS_PCH_DEVICE_ID='0x99EF'
#export HAS_PRIMARY_PLANE1='skl'

################# WW05.2 ##################
#export LEGACY_SKL_RESET='0'
#export STOLEN_MEM_BASE_EMU="1"
#export USE_MODEL_MEM_MAP="1"
###########################################
#export MGGC_STOLEN_SIZE_BIT_POS="8"
#export HAS_SYSTEM_MEM_SIZE=2048
#export GTT_BASE="0x7F800000"
#export GTT_SIZE="8"
#export HAS_STOLEN_MEM_SIZE=32
#export HAS_STOLEN_MEM_BASE="0x80000000"
#export STOLEN_MEM_BASE_EMU="1"
###########################################

#export HAS_GT_TYPE=1
export HAS_GT_TYPE=2
#export HAS_GT_TYPE=3

#a0
#export HAS_REV_ID=0x0
#export HAS_REVISION_ID=0

#b0
#export HAS_REV_ID=0x1
#export HAS_REVISION_ID=1

#c0
#export HAS_REV_ID=0x2
#export HAS_REVISION_ID=2


#export HAS_FULSIM_PATH=/fulsim/lin/cnl
#export REGINI=/mnt/share/cnl_regini.xml

######################EMULATOR SETTINGS######################
#QT163M_2
export TBX_SERVER_IP=10.96.73.18


#############################################################
export HAS_SERIAL='tcp::80,server,nodelay,nowait'

# RUN-LIST & CONCURRENCY STUFF
export INTR_EMUL_SUPP='1'
export CNCRY_INTR_EMUL_SUPP='1'
export RUN_LIST_WA='1'
export HAS_RUN_LIST_WA='1'

#echo timeout settings
#export HANG_TIMEOUT=60
#export HANG_CYCLES=10

#VNC Mouse fix
export HAS_USB='tablet'

# Start HAS 2
if [ "$HAS_SCRIPT" != "" ]
then
	/has/bin/genxfsim-dm -S $HAS_SCRIPT
else
	/has/bin/genxfsim-dm
fi
