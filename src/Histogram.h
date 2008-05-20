/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <vector>
#include "UnaryPlugin.h"

/**
 * Writes a Histogram representing the input
 */
class Histogram : public UnaryPlugin<float, float>
{
public:
    Histogram(Plugin<float>* iInput, const char* iObjectName = "Histogram");
    ~Histogram();

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    void write();

    float mMin;
    float mMax;
    float mScale;
    int mCount;
    int mMinCount;
    int mNBins;
    std::vector< std::vector<float> > mBin;
    bool mPDF;
    float mPower;
};


#endif /* HISTOGRAM_H */
