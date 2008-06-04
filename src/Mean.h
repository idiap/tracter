/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MEAN_H
#define MEAN_H

#include <vector>
#include "UnaryPlugin.h"

enum MeanType
{
    MEAN_STATIC,
    MEAN_ADAPTIVE
};

/**
 * Calculates the mean (over time) of the input stream, typically for
 * Cepstral Mean Normalisation.
 */
class Mean : public UnaryPlugin<float, float>
{
public:
    Mean(Plugin<float>* iInput, const char* iObjectName = "Mean");
    virtual ~Mean() throw() {}
    virtual void Reset(bool iPropagate);
    void SetTimeConstant(float iSeconds);

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    bool mValid;
    MeanType mMeanType;
    std::vector<float> mMean;

    float mPole;
    float mElop;

    void processAll();
    bool adaptFrame(IndexType iIndex);
};


#endif /* MEAN_H */
