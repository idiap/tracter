/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/* See the file COPYING for the licence associated with this software. */

#ifndef BSAPIFilterBank_H
#define BSAPIFilterBank_H

#include <vector>

#include "CachedComponent.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * Calculate filter-bank features
     */
  class BSAPIFilterBank : public CachedComponent<float>
    {
    public:
        BSAPIFilterBank(Component<float>* iInput, const char* iObjectName = "BSAPIFilterBank");
	BSAPIFilterBank(Component<float>* iInput, Component<float>* iInputWF, const char* iObjectName = "BSAPIFilterBank");
        virtual ~BSAPIFilterBank() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);

    private:
	SMelBanksI *mpMelBanks;
	int SampleFreq;
	
	float WaveformScaleUp;
	float *mpInputWaveform;
	int   inputdim; 

	float wf;

	void InitFrontEnd(void);
	void InitOutBuffer(void);
	Component<float>* mInputWF; 
	Component<float>* mInput; 

	class STarget : public SFeatureExtractionCallbackI
        {  
	public:   
	  float *mpOutBuff;
	  int nbuffsize;
	  // STarget() : mpNnout(NULL) {}
	  // float *mpNnout;
	  bool BSAPI_METHOD OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags);
	} mTarget;
    };
}

#endif /* BSAPIFilterBank_H */
