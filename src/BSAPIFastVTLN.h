/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/* See the file COPYING for the licence associated with this software. */

#ifndef BSAPIFastVTLN_H
#define BSAPIFastVTLN_H

#include <vector>

#include "UnaryPlugin.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * Calculate fast Vocal Tract Length Normalisation parameter
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
	float WaveformScaleUp;
	float *mpInputWaveform;

	float wf;
	
	SGMMBasedEstimatorI *mpFastVTLN;

	int SamplingFreq;
	int MaxBufferedFrames;

	//class SFastVtlnCallback : public SFastVtlnCallbackI
	//{
	//public:
	// virtual bool BSAPI_METHOD OnWarpingFactor(long_long time, float warpingFactor);
	//} gFastVtlnCallback;
    };
}

#endif /* BSAPIFilterBank_H */
