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

#ifndef BSAPIFilterBank_H
#define BSAPIFilterBank_H

#include <vector>

#include "UnaryPlugin.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * Plugin to calculate delta features
     */
    class BSAPIFilterBank : public UnaryPlugin<float, float>
    {
    public:
        BSAPIFilterBank(Plugin<float>* iInput, const char* iObjectName = "BSAPIFilterBank");
        virtual ~BSAPIFilterBank() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
	SMelBanksI *mpMelBanks;
	int SampleFreq;
	
	float WaveFromScaleUp;
	int   inputdim; 

        class STarget : public SFeatureExtractionCallbackI
        {  
	public:    
	  STarget() : mpNnout(NULL) {}
	  float *mpNnout;
	  bool BSAPI_METHOD OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags);
	  } mTarget;
    };
}

#endif /* BSAPIFilterBank_H */
