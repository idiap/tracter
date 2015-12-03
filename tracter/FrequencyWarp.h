/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FREQUENCYWARP_H
#define FREQUENCYWARP_H

#include <cassert>
#include <vector>

#include "TracterObject.h"

namespace Tracter
{
    /**
     * Mel scaled filter bank.
     */
    class FrequencyWarp : public Object
    {
    public:
        FrequencyWarp(const char* iObjectName = "FrequencyWarp");
        virtual ~FrequencyWarp() throw () {};

        void Initialise(int iNPSD);
        void DumpBins();
        float Weight(int iBin);
        int BinSize(int iBin) { return mWeight[iBin].size(); }
        int NBins() { return mNBins; }

        /**
         * Apply a warp to a vector.  It's a template so it can apply to
         * complex or int as well as float.
         */
        template <class T>
        void Warp(const T* iInput, T* iOutput)
        {
            assert(iInput);
            assert(iOutput);

            for (int i=0; i<mNBins; i++)
            {
                iOutput[i] = 0.0f;
                for (size_t j=0; j<mWeight[i].size(); j++)
                    iOutput[i] += mWeight[i][j] * iInput[mBin[i]+j];
            }
        }

        void SetRange(float iLoHertz, float iHiHertz)
        {
            mLoHertz = iLoHertz;
            mHiHertz = iHiHertz;
        }

        void SetWarp(float iLoWarp, float iHiWarp, float iAlpha)
        {
            mLoWarp = iLoWarp;
            mHiWarp = iHiWarp;
            mAlpha = iAlpha;
        }

    protected:

    private:
        std::vector<int> mBin;   // Warp 'centers' in terms of DFT bins
        std::vector< std::vector<float> > mWeight; // The actual filters

        float hertzToWarp(float iHertz);
        float warpToHertz(float iHertz);
        int hertzToBin(float iHertz);
        float binToHertz(int iBin);

        void initAlignedBins();
        void initSmoothBins();
        void normaliseBins();
        float warpHertz(float iHertz, float iAlpha);

        int mNPSD;
        int mNBins;

        float mMaxHertz;
        float mLoHertz;
        float mHiHertz;
        float mLoWarp;
        float mHiWarp;
        float mAlpha;
    };
}

#endif /* FREQUENCYWARP_H */
