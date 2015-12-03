/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/* See the file COPYING for the licence associated with this software. */

#ifndef BSAPIFrontEnd_H
#define BSAPIFrontEnd_H

#include <vector>

#include "CachedComponent.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * BSAPI based ASR front-end
     */
    class BSAPIFrontEnd : public CachedComponent<float>
    {
    public:
        BSAPIFrontEnd(
            Component<float>* iInput, const char* iObjectName = "BSAPIFrontEnd"
        );
        BSAPIFrontEnd(
            Component<float>* iInput, Component<float>* iInputWF,
            const char* iObjectName = "BSAPIFrontEnd"
        );
        virtual ~BSAPIFrontEnd() throw();
    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);
        void Reset(bool iPropagate);

    private:
        int   inputdim;
        float WaveformScaleUp;
        float *mpInputWaveform;
        float wf;
        SSpeechRecI *mpPLP;
        int MaxBufferedFrames;
        bool LastFrameProcess;

        void InitFrontEnd(void);
        void InitOutBuffer(void);
        Component<float>* mInputWF;
        Component<float>* mInput;

        virtual SMelBanksI *BSAPI_METHOD GetMelTarget(
            SSpeechRecI *mpSpeechRec
        );

        class STarget : public SFeatureExtractionCallbackI
        {
        public:
            float *mpOutBuff;
            int nbuffsize;
            int MaxBuffSize;
            bool BSAPI_METHOD OnFeatureMatrix(
                SFloatMatrixI *pMatrix, int nFrames, unsigned int flags
            );
        } mTarget;
    };
}

#endif /* BSAPIFilterBank_H */
