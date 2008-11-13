/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MLP_VAD_H
#define MLP_VAD_H

#include "UnaryPlugin.h"
//#include "SlidingDFT.h"
#include "VADStateMachine.h"

/**
 * A VAD feature based on energy modulation.  The energy is filtered
 * using the first non-DC bin of a DFT centered at 4Hz because speech
 * is modulated at about 4Hz.  The threshold is based on the noise
 * part of the unfiltered energy.
 */

namespace Tracter
{
class MLPVAD : public VADStateMachine
{
public:
    MLPVAD(Plugin<float>* iInput,
                  const char* iObjectName = "MLPVAD");

protected:

    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        assert(this->mNInputs == 1);
        return mInput;
    }

    bool UnaryFetch(IndexType iIndex, int iOffset);
    virtual void Reset(bool iPropagate);

private:
    Plugin<float>* mInput;
    IndexType mIndex;
    int mSilIndex;
    float mSilThresh;

    int mLookAhead;
    bool mShowGuts;
};
}
#endif /* MLPVAD_H */
