/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <map>
#include <string>

#include "Window.h"

namespace Tracter
{
    /** Static map of window shapes */
    static std::map<std::string, Window::Shape> sShapeMap;
}

Tracter::Window::Window(const char* iObjectName, int iSize)
{
    objectName(iObjectName);

    // Static shape information
    if (sShapeMap.size() == 0)
    {
        sShapeMap["Rect"] = RECTANGULAR;
        sShapeMap["Rectangular"] = RECTANGULAR;
        sShapeMap["Hann"] = HANN;
        sShapeMap["Hamming"] = HAMMING;
        sShapeMap["Gauss"] = GAUSS;
    }

    // Discern the shape
    const char* shape = config("Shape", "Hamming");
    if (sShapeMap.count(shape) == 0)
        throw Exception("unknown window shape %s", shape);
    mShape = sShapeMap[shape];

    // Must be after shape check
    if (iSize)
        resize(iSize);
}

/**
 * Resize the window.  The window is, by default, symmetrical.  If
 * iDivideN is true, the window is made asymmetrical
 * (see http://en.wikipedia.org/wiki/Window_function)
 */
void Tracter::Window::resize(int iSize, bool iDivideN)
{
    const float PI = 3.14159265358979323846;
    mWeight.resize(iSize);
    float denom = iDivideN ? iSize : (iSize - 1);

    switch (mShape)
    {
    case RECTANGULAR:
        for (int i=0; i<iSize; i++)
            mWeight[i] = 1.0f;
        break;

    case HANN:
        for (int i=0; i<iSize; i++)
            mWeight[i] = 0.5f - 0.5f * cosf(PI * 2.0f * i / denom);
        break;

    case HAMMING:
        for (int i=0; i<iSize; i++)
            mWeight[i] = 0.54f - 0.46f * cosf(PI * 2.0f * i / denom);
        break;

    case GAUSS:
        for (int i=0; i<iSize; i++)
        {
            float sigma = 0.5f;
            float x = (i - denom/2) / (sigma * denom/2);
            mWeight[i] = expf(-0.5 * x * x);
        }
        break;

    default:
        throw Exception("unknown window shape");
    }
}

float* Tracter::Window::Apply(const float* iData, float* oData) const
{
    for (int i=0; i<(int)mWeight.size(); i++)
        oData[i] = iData[i] * mWeight[i];
    return oData;
}
