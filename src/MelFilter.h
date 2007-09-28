#ifndef MELFILTER_H
#define MELFILTER_H

#include <vector>

#include "UnaryPlugin.h"

/**
 * Mel scaled filter bank.
 */
class MelFilter : public UnaryPlugin<float, float>
{
public:
    MelFilter(
        Plugin<float>* iInput, int iNBins, float iSampleRate,
        float iLoHertz, float iHiHertz
    );

protected:
    bool ProcessFrame(IndexType iIndex, int iOffset);

private:
    std::vector<int> mBin;   // Mel 'centers' in terms of DFT bins
    std::vector< std::vector<float> > mWeight; // The actual filters

    float hertzToMel(float iHertz);
    float melToHertz(float iHertz);
    int hertzToBin(float iHertz, int iNBins, float iHiHertz);
};

#endif /* MELFILTER_H */
