/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MLP_VAD_H
#define MLP_VAD_H

#include "UnaryPlugin.h"
#include "VADStateMachine.h"

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
        int mInputIndex;
        float mThreshold;
        bool mSpeech;

        int mLookAhead;
        bool mShowGuts;
    };
}

#endif /* MLPVAD_H */
