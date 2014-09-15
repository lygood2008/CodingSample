/*!
    @file clDumpGPUInfo.h
    @desc: Helper functions for dumping OpenCL information
    @author: yanli
    @date: May 2013
 */
#ifndef CLDUMPGPUINFO_H
#define CLDUMPGPUINFO_H

#include "CL/cl.h"
#include <QVector>
#include <QString>

/**
 * @brief dumpGPUInfo: dump GPU info
 */
void dumpGPUInfo();

/**
 * @brief getPlatformList: get platform list
 * @return: list of platform id
 */
QVector<cl_platform_id> getPlatformList();

/**
 * @brief getDeviceList: get device list from platform and device type
 * @param platform: the platform id
 * @param type: the device type
 * @return: list of device id
 */
QVector<cl_device_id> getDeviceList(cl_platform_id platform, cl_device_type type);

/**
 * @brief getPlatformName: get name from platform id
 * @param platform: the platform id
 * @return: the name of the platform
 */
QString getPlatformName(cl_platform_id platform);

/**
 * @brief getDeviceName: get device name from device id
 * @param device: the device id
 * @return: the name of device
 */
QString getDeviceName(cl_device_id device);

#endif // CLDUMPGPUINFO_H
