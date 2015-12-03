/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MLPVAD_H
#define MLPVAD_H

#include "VADStateMachine.h"

namespace Tracter
{
    class MLPVAD : public VADStateMachine
    {
    public:
        MLPVAD(Component<float>* iInput,
               const char* iObjectName = "MLPVAD");

    protected:
        bool UnaryFetch(IndexType iIndex, VADState* oData);
        virtual void Reset(bool iPropagate);

    private:
        Component<float>* mInput;
        IndexType mIndex;
        int mInputIndex;
        float mThreshold;
        bool mSpeech;

        int mLookAhead;
        bool mShowGuts;
    };
}

#endif /* MLPVAD_H */
