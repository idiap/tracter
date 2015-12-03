/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/* See the file COPYING for the licence associated with this software. */

#ifndef BSAPITRANSFORM_H
#define BSAPITRANSFORM_H

#include <vector>

#include "CachedComponent.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * Apply BSAPI transform
     */
    class BSAPITransform : public CachedComponent<float>
    {
    public:
        BSAPITransform(Component<float>* iInput, const char* iObjectName = "BSAPITransform");
        BSAPITransform(Component<float>* iInput, Component<float>* iInputID, const char* iObjectName = "BSAPITransform");
	virtual ~BSAPITransform() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);
	void Reset(bool iPropagate);

    private:
	int inputdim; 
	int mContext;
	int mWindow;
        SFeaCatI *mpFeaCat;
        SFloatMatrixI *mpInput;

	int MaxBufferedFrames;

        bool LastFrameProcess;

	void InitTransform(void);
	void InitOutBuffer(void);

	Component<float>* mInputID; 
	Component<float>* mInput; 

	const char* xformdir;
	char  *mInputID_macroname_full;
	float  mInputID_macroname;

        class STarget : public SFeatureExtractionCallbackI
        {  
	public:    
	  float *mpOutBuff;
          int nbuffsize;
          int MaxBuffSize;
	  bool BSAPI_METHOD OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags);
	} mTarget;
    };
}

#endif /* BSAPITRANSFORM_H */
