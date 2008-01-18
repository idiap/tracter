/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef RESAMPLE_H
#define RESAMPLE_H

#include <samplerate.h>
#include <vector>

#include "UnaryPlugin.h"

/**
 * Resample or convert sample rate.
 *
 * Implemented using Secret Rabbit Code.
 *
 * N.B., it's almost certainly better to use the rate converter in
 * ALSA or similar if possible.  This one is for when the data is
 * coming from a file or other source where the rate can't be chosen.
 */
class Resample : public UnaryPlugin<float, float>
{
public:
    Resample(Plugin<float>* iInput,
                      const char* iObjectName = "Resample");
    ~Resample();
    void Reset(bool iPropagate = true);
    void MinSize(int iSize, int iReadAhead);

protected:
    int Fetch(IndexType iIndex, CacheArea& iOutputArea);

private:
    void process();

    SRC_STATE* mState;
    SRC_DATA mData;
    std::vector<float> mResample;
};

#endif /* RESAMPLE_H */
