#ifndef PERIODOGRAM_H
#define PERIODOGRAM_H

#include <vector>

#include <fftw3.h>
#include "UnaryPlugin.h"

/**
 * Uses the FFTW library to calculate a periodogram (aka power spectral density)
 */
class Periodogram : public UnaryPlugin<float, float>
{
public:
    Periodogram(Plugin<float>* iInput, int iFrameSize, int iFramePeriod);
    ~Periodogram();

protected:
    bool ProcessFrame(IndexType iIndex, int iOffset);

private:
    int mFrameSize;
    int mFramePeriod;
    float* mRealData;
    fftwf_complex* mComplexData;
    fftwf_plan mPlan;
    std::vector<float> mWindow;
};

#endif /* PERIODOGRAM_H */
