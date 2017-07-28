/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MELFILTER_H
#define MELFILTER_H

#include <vector>

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Mel scaled filter bank.
     */
    class MelFilter : public CachedComponent<float>
    {
    public:
        MelFilter(
            Component<float>* iInput, const char* iObjectName = "MelFilter"
        );
        void dumpBins();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;

        std::vector<int> mBin;   // Mel 'centers' in terms of DFT bins
        std::vector< std::vector<float> > mWeight; // The actual filters

        float hertzToMel(float iHertz);
        float melToHertz(float iHertz);
        int hertzToBin(float iHertz, int iNBins);
        float binToHertz(int iBin, int iNBins);

        void initAlignedBins();
        void initSmoothBins();
        void normaliseBins();
        float warpHertz(float iHertz, float iAlpha);

        float mMaxHertz;
        float mLoHertz;
        float mHiHertz;
        float mLoWarp;
        float mHiWarp;
        float mAlpha;
    };
}

#endif /* MELFILTER_H */
