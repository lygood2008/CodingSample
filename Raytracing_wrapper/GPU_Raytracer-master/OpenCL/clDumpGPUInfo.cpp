/*!
    @file clDumpGPUInfo.cpp
    @desc: Helper functions for dumping OpenCL information
    @author: yanli
    @date: May 2013
 */
#include "clDumpGPUInfo.h"
#include <iostream>

using std::endl;
using std::cout;

void dumpGPUInfo()
{
    // Query all platforms
    QVector<cl_platform_id> platforms = getPlatformList();

    cout<< "CL DUMP: platform_count = " << platforms.size() << endl;

    for (int p = 0; p < platforms.size(); p++)
    {
        QString platform_name = getPlatformName(platforms.at(p));
        cout << "CL DUMP: platform = " << p << " name = "
             << platform_name.toStdString() << endl;

        //devices
        QVector<cl_device_id> devices = getDeviceList(platforms.at(p),
                                                      CL_DEVICE_TYPE_GPU);

         cout<< "CL DUMP: platform = " << p << " device_count = "
             << devices.size() << endl;

        for (int d = 0; d < devices.size(); d++)
        {
            QString device_name = getDeviceName(devices[d]);
            cout << "CL DUMP: platform = " << p << " device = " << d
                 << " name = " << device_name.toStdString() << endl;
        }
    }
}

QVector<cl_platform_id> getPlatformList()
{
    QVector<cl_platform_id> list;
    cl_int rv = CL_SUCCESS;
    cl_uint count = 0;

    rv = clGetPlatformIDs(0, NULL, &count);
    if (rv != CL_SUCCESS || !count)
    {
        return list;
    }

    list.resize(count);
    rv = clGetPlatformIDs(list.size(), list.data(), NULL);
    if (rv != CL_SUCCESS)
    {
        list.clear();
    }
    return list;
}

QVector<cl_device_id> getDeviceList(cl_platform_id platform, cl_device_type type)
{
    QVector<cl_device_id> list;
    cl_int rv = CL_SUCCESS;
    cl_uint count = 0;

    rv = clGetDeviceIDs(platform, type, 0, NULL, &count);
    if (rv != CL_SUCCESS || !count)
    {
        return list;
    }

    list.resize(count);
    rv = clGetDeviceIDs(platform, type, list.size(), list.data(), NULL);
    if (rv != CL_SUCCESS)
    {
        list.clear();
    }

    return list;
}

QString getPlatformName(cl_platform_id platform)
{
    cl_int rv = CL_SUCCESS;
    size_t size = 0;

    rv = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &size);
    if (rv != CL_SUCCESS || !size)
    {
        return QString();
    }

    QByteArray name;
    name.resize((int)(size) + 1);

    rv = clGetPlatformInfo(platform, CL_PLATFORM_NAME, name.size(), name.data(),
                           NULL);
    if (rv != CL_SUCCESS)
    {
        return QString();
    }
    name.data()[size] = '\0';

    return name;
}

QString getDeviceName(cl_device_id device)
{
    cl_int rv = CL_SUCCESS;
    size_t size = 0;

    rv = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &size);

    if (rv != CL_SUCCESS || !size)
    {
        return QString();
    }

    QByteArray name;
    name.resize((int)(size) + 1);

    rv = clGetDeviceInfo(device, CL_DEVICE_NAME, name.size(), name.data(), NULL);

    if (rv != CL_SUCCESS)
    {
        return QString();
    }
    name.data()[size] = '\0';

    return name;
}
