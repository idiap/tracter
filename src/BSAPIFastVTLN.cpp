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

#include "BSAPIFastVTLN.h"

Tracter::BSAPIFastVTLN::BSAPIFastVTLN(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    inputdim    = iInput->GetArraySize(); 

    const char* VTLNFrontEndConfig = GetEnv("Config","./configs/bsapi.VTLN.cfg");
    
    MaxBufferedFrames  = GetEnv("MaxBufferedFrames",5);
    SamplingFreq = GetEnv("SamplingFreq",16000);
    WaveformScaleUp = GetEnv("WaveformScaleUp",32768);
 
    // If scaling is needed, the memory is allocated
    if ( WaveformScaleUp != 1 ) 
      mpInputWaveform = new float[inputdim*MaxBufferedFrames];
    

    PluginObject::MinSize(mInput,  MaxBufferedFrames , MaxBufferedFrames);
 
    mpFastVTLN = static_cast<SGMMBasedEstimatorI *>(BSAPICreateInstance(SIID_GMMESTIMATOR));
    if(!mpFastVTLN)
      {
        fprintf(stderr, "No memory!");
        exit(1);
      }

    if (!mpFastVTLN->Init((char*)VTLNFrontEndConfig)){
      printf("Bad Initialization of VTLN  module!!!\n");
      mpFastVTLN->Release();
      exit(1);
     }
    //mpFastVTLN->SetTarget(&gFastVtlnCallback);

    mArraySize=1;   // just Warp factors
    assert(mArraySize > 0);
    
    
    printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->GetArraySize(), mArraySize); 
}

Tracter::BSAPIFastVTLN::~BSAPIFastVTLN() throw ()
{
  mpFastVTLN->Release();

  if ( WaveformScaleUp != 1 )
    delete[]mpInputWaveform;
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPIFastVTLN::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    int extend,shift;

    //int one = mInput->Read(inputArea, iIndex, 1);
    
    if (!iIndex){
      extend = MaxBufferedFrames;
      shift  = 0;
    }
    else {
      extend = 1;
      shift  = MaxBufferedFrames-1;
    }

    int numRead = mInput->Read(inputArea, iIndex+shift, extend);

    // printf("numRead : %i\n", numRead);
  
    float *pframe  = mInput->GetPointer(inputArea.offset);
 
    // Memory scaling. If it is not needed, just pointer is copied 
    if ( WaveformScaleUp != 1 ) 
      for (int j=0; j<inputdim*extend; j++) 
      	mpInputWaveform[j] = pframe[j] * WaveformScaleUp;
    else
      mpInputWaveform = pframe;
    
    //for (int j=0; j<inputdim*extend; j++) {
    //  if ( (j+1) % 10 == 0 )  printf ("\n");
    //printf("%f ", mpInputWaveform[j]);
    //}
    //printf ("\n");
    


    //wf=1.0;
    if ( numRead )
      mpFastVTLN->OnWaveform( SWaveformSourceCallbackI::wfFloat , SamplingFreq, 1, mpInputWaveform,  numRead * inputdim * sizeof(float), 0);
    
    wf = mpFastVTLN->GetFactor();
    // printf("wf: %f\n",wf);

    //GetMelTarget(mpPLP)->SetWarpAlpha(wf);
    
    float* cache = GetPointer(iOffset);

    cache[0] = wf;

    return numRead>0;
}
