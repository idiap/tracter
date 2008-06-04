/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HTKSINK_H
#define HTKSINK_H

#include <stdio.h>

#include <vector>

#include "UnarySink.h"
#include "ByteOrder.h"

/**
 * Sinks to an HTK format parameter file.  The file is always written
 * big-endian.
 */
class HTKSink : public UnarySink<float>
{
public:
    HTKSink(Plugin<float>* iInput, const char* iObjectName = "HTKSink");
    virtual ~HTKSink() throw() {}
    void Open(const char* iFile);

private:
    FILE* mFile;
    ByteOrder mByteOrder;
    std::vector<float> mTemp;

    /* Header */
    int mNSamples;
    int mSampPeriod;
    short mSampSize;
    short mParmKind;

    void WriteHeader(FILE* iFile);
};

#endif /* HTKSINK_H */
