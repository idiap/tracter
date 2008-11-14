/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/** See the file COPYING for the licence associated with this software.
 */

#ifndef BSAPITRANSFORM_H
#define BSAPITRANSFORM_H

#include <vector>

#include "UnaryPlugin.h"
#include "bsapi.h"

namespace Tracter
{
    /**
     * Plugin to calculate delta features
     */
    class BSAPITransform : public UnaryPlugin<float, float>
    {
    public:
        BSAPITransform(Plugin<float>* iInput, const char* iObjectName = "BSAPITransform");
        virtual ~BSAPITransform() throw();

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
	int mContext;
	int mWindow;
        SFeaCatI *mpFeaCat;
        SFloatMatrixI *mpInput;

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
    };
}

#endif /* BSAPITRANSFORM_H */
