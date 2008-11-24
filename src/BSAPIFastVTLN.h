/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
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
	float WaveformScaleUp;
	float *mpInputWaveform;

	float wf;
	
	SFastVtlnI  *mpFastVTLN;

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
