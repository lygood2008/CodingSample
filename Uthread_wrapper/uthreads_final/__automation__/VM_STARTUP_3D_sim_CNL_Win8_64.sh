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


# Set Up Environment Variables
export HAS_DISK='phy:/dev/vcg/3d_winB,hda,w' 
export HAS_SCRIPT=/has/scripts/start_gen10_fulsim.lua
export HAS_SYSTEM_MEM_SIZE=2048
export HAS_STOLEN_MEM_SIZE=32
export HAS_NIC="type=ioemu,bridge=xenbr-p2p1,mac=$(ifconfig|grep xenbr|sed 's/.*HWaddr //'|od -An -N6 -tx1|sed -e 's/^  *//' -e 's/  */:/g' -e 's/:$//' -e 's/^\(.\)[13579bdf]/\10/')"

export HAS_PRIMARY_PLANE1='skl'

# GT Type
export HAS_GT_TYPE=2

########## Stepping ##########
# a0
export HAS_REVISION_ID=0
export HAS_FULSIM_STEPPING='a0'

# b0
#export HAS_REVISION_ID=1
#export HAS_FULSIM_STEPPING='b0'

# c0
#export HAS_REVISION_ID=2
#export HAS_FULSIM_STEPPING='c0'
##############################

# Sim fulsim settings
export HAS_FULSIM_PATH=/fulsim/lin/cnl

# Setting fulsim as the TBX_SERVER
export TBX_SERVER_IP=192.168.1.101
export HAS_SERIAL='tcp::80,server,nodelay,nowait'

# Hang Detection settings
#export HANG_TIMEOUT=60
#export HANG_CYCLES=30

# VNC Mouse fix
export HAS_USB='tablet'

# RUN-LIST & CONCURRENCY STUFF
export INTR_EMUL_SUPP='1'
export CNCRY_INTR_EMUL_SUPP='1'
export RUN_LIST_WA='1'
export HAS_RUN_LIST_WA='1'

# Starting the script, if defined
if [ "$HAS_SCRIPT" != "" ]
then
	/has/bin/genxfsim-dm -S $HAS_SCRIPT
else
	/has/bin/genxfsim-dm
fi
