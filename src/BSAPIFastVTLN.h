/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/*
 * See the file COPYING for the licence associated with this software.
 */

#ifndef BSAPIFastVTLN_H
#define BSAPIFastVTLN_H

#include <vector>

#include "UnaryPlugin.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * Plugin to calculate delta features
     */
    class BSAPIFastVTLN : public UnaryPlugin<float, float>
    {
    public:
        BSAPIFastVTLN(Plugin<float>* iInput, const char* iObjectName = "BSAPIFastVTLN");
        virtual ~BSAPIFastVTLN() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
	int   inputdim; 
	float WaveFromScaleUp;
	float wf;
	
	SSpeechRecI *mpPLP;
	SFastVtlnI  *mpFastVTLN;

	int SamplingFreq;
	int MaxBufferedFrames;

	bool LastFrameProcess;

        class STarget : public SFeatureExtractionCallbackI
        {  
	public:    
	  float *mpOutBuff;
	  int nbuffsize;
	  int MaxBuffSize;
	  bool BSAPI_METHOD OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags);
	} mTarget;
	
        virtual SMelBanksI *BSAPI_METHOD GetMelTarget(SSpeechRecI *mpSpeechRec);

	//class SFastVtlnCallback : public SFastVtlnCallbackI
	//{
	//public:
	// virtual bool BSAPI_METHOD OnWarpingFactor(long_long time, float warpingFactor);
	//} gFastVtlnCallback;
    };
}

#endif /* BSAPIFilterBank_H */
