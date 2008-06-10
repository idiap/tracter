/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef VADGATE_H
#define VADGATE_H

#include "CachedPlugin.h"
#include "VADStateMachine.h"

/**
 * VAD Gate.  Allows frames through from input to output depending on
 * a VAD input.
 */
class VADGate : public CachedPlugin<float>
{
public:
    VADGate(Plugin<float>* iInput, Plugin<VADState>* iVADInput,
            const char* iObjectName = "VADGate");

protected:
    PluginObject* GetInput(int iInput);
    bool UnaryFetch(IndexType iIndex, int iOffset);
    virtual void Reset(bool iPropagate);

private:
    Plugin<float>* mInput;
    Plugin<VADState>* mVADInput;

    bool mEnabled;
    VADState mState;
    IndexType mSpeechTriggered;
    IndexType mSpeechConfirmed;
    IndexType mIndexZero;

    bool gate(IndexType& iIndex);
    bool readVADState(IndexType iIndex);
    bool confirmSpeech();
    bool reconfirmSpeech(IndexType iIndex);
};


#endif /* VADGATE_H */
