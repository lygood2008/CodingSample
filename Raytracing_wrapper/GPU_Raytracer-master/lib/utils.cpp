/******************************************************************************
 ******************************************************************************
 ********************* This file is not written by Yan Li!!********************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/

/*!
    @file utils.cpp
    @desc: definitions of utility functions
    @author: yanli
    @date: May 2013
 */

#include "utils.h"
#include <iostream>

using std::cout;
using std::endl;

void printMat4x4(char* title, const Matrix4x4& mat)
{

    cout<<title<<endl;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (fabs(mat.data[i * 4 + j]) < 0.000001f)
                cout<<0<<" ";
            else
                cout<<mat.data[i * 4 + j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl;
}


RGBA bilinearInterpTexel(const unsigned * texPixels,
                         float x,
                         float y,
                         const int width,
                         const int height,
                         const int offset)
{
    if (x < 0)
        x = 0.f;
    if (y < 0)
        y = 0.f;
    if (x > width - 1)
        x = width - 1;
    if (y > height - 1)
        y = height -1;

    const int X       = (int)x;
    const int Y       = (int)y;
    const float s1    = x - X;
    const float s0    = 1.f - s1;
    const float t1    = y - Y;
    const float t0    = 1.f - t1;

    unsigned e1, e2, e3,e4;
    RGBA r1, r2, r3, r4;
    RGBA result;
    e1 = e2 = e3 = e4 = 0;
    e1 = texPixels[offset + Y * width + X];

    if (Y + 1 <= height - 1)
        e2 = texPixels[offset + (Y + 1) * width + X];
    if (X + 1 <= width -1)
        e3 = texPixels[offset + (Y) * width + X + 1];
    if (Y + 1 <= height - 1 && X + 1 <= width - 1)
        e4 = texPixels[offset + (Y + 1) * width + X + 1];

    r1 = rgbaUintToColor(e1);
    r2 = rgbaUintToColor(e2);
    r3 = rgbaUintToColor(e3);
    r4 = rgbaUintToColor(e4);

    result.r = s0 * (t0 * r1.r + t1 * r2.r) + s1 * (t0 * r3.r + t1 * r4.r);
    result.g = s0 * (t0 * r1.g + t1 * r2.g) + s1 * (t0 * r3.g + t1 * r4.g);
    result.b = s0 * (t0 * r1.b + t1 * r2.b) + s1 * (t0 * r3.b + t1 * r4.b);
    result.a = s0 * (t0 * r1.a + t1 * r2.a) + s1 * (t0 * r3.a + t1 * r4.a);
    return result;
}

RGBA rgbaUintToColor(const unsigned int rgba)
{

    RGBA result;
    unsigned int s3 = (rgba & 0xFF000000) >> 24;
    unsigned int s2 = (rgba & 0x00FF0000) >> 16;
    unsigned int s1 = (rgba & 0x0000FF00) >> 8;
    unsigned int s0 = (rgba & 0x000000FF);
    result.r = s0 / 255.f;
    result.g = s1 / 255.f;
    result.b = s2 / 255.f;
    result.a = s3 / 255.f;
    return result;
}

unsigned rgbaColorToUint(RGBA rgba)
{

    unsigned result;
    int r,g,b,a;
    r = (int)(rgba.r * 255.f);
    g = (int)(rgba.g * 255.f);
    b = (int)(rgba.b * 255.f);
    a = (int)(rgba.a * 255.f);
    result = r + (g << 8) + (b << 16) + (a << 24);
    return result;
}

Vector4 getReflectionDir(Vector3 norm, Vector4 incident)
{

    REAL cos = -incident.x * norm.x + incident.y * norm.y + incident.z * norm.z;

    Vector4 reflection =
            incident + Vector4(2 * cos * norm.x, 2 * cos * norm.y,
                               2 * cos * norm.z, 0).getNormalized();
    return reflection;
}

Vector4 getRefracetionDir(Vector3 norm, Vector4 incident, REAL n1, REAL n2 )
{

    const REAL n = n1 / n2;
    const REAL cosI =
            -norm.x * incident.x + norm.y * incident.y + norm.z * incident.z;
    const REAL sinT2 = n * n*(1.0 - cosI * cosI);
    Vector4 norm4 = Vector4(norm.x, norm.y, norm.z, 0);

    if (sinT2 >= 1.0)
    {
        return Vector4::zero();
    }
    return incident * n + norm4 * (n * cosI - sqrt(1.0 - sinT2));
}

bool isInString(char* string, const char* search)
{

    int pos = 0;
    int maxpos = strlen(search) - 1;
    int len = strlen(string);

    for (int i = 0; i < len; i++)
    {
        if (i == 0 || (i > 1 && string[i - 1] == '\n'))
        {
            pos = 0;

            while (string[i] != '\n' && i < len)
            {
                if (string[i] == search[pos])
                    pos++;
                if (pos > maxpos &&
                   (string[i + 1] == '\n' || string[i + 1] == '\0'))
                    return true;
                i++;
            }
        }
    }
    return false;
}
