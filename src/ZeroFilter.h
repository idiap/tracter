/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ZEROFILTER_H
#define ZEROFILTER_H

#include "UnaryPlugin.h"

/**
 * Implements a filter consisting of a single zero.
 */
class ZeroFilter : public UnaryPlugin<float, float>
{
public:
    ZeroFilter(Plugin<float>* iInput, const char* iObjectName = "ZeroFilter");
    void MinSize(int iSize, int iReadAhead);

protected:
    int Fetch(IndexType iIndex, CacheArea& iOutputArea);

private:
    float mZero;
};

#endif /* ZEROFILTER_H */
