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

#include "BSAPIFilterBank.h"

Tracter::BSAPIFilterBank::BSAPIFilterBank(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    inputdim    = iInput->GetArraySize(); 

   
    SampleFreq = GetEnv("SampleFreq", 16000);
    WaveFromScaleUp = GetEnv("WaveFromScaleUp",32768);

    int VecSize    = GetEnv("VecSize", 400);
    int Step       = GetEnv("Step", 160);
    float PreemCoef  = GetEnv("PreemCoef", 0);
    //int NBanks     = GetEnv("NBanks", 23);
    bool TakeLog    = GetEnv("TakeLog", 1);
    float LoFreq     = GetEnv("LoFreq", 0);
    float HiFreq     = GetEnv("HiFreq", SampleFreq/2);


    PluginObject::MinSize(mInput, 1, 1);

    mpMelBanks = static_cast<SMelBanksI *>(BSAPICreateInstance(SIID_FE_MELBANKS));
    if(!mpMelBanks)
      {
        fprintf(stderr, "No memory!");
        exit(1);
      }

    //    virtual void    BSAPI_METHOD SetTransform(char *pTransformName) = 0;
    mpMelBanks->SetSampleFreq(SampleFreq);
    mpMelBanks->SetVectorSize(VecSize);
    mpMelBanks->SetStep(Step);
    mpMelBanks->SetPreemCoef(PreemCoef);
    mpMelBanks->SetTakeLog(TakeLog);
    mpMelBanks->SetLowFreq(LoFreq);
    mpMelBanks->SetHighFreq(HiFreq);



    mpMelBanks->SetOutBatchSize(1);
    mpMelBanks->SetTarget(&mTarget);

    mArraySize = mpMelBanks->GetNOutputs();
    assert(mArraySize > 0);

    // printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->GetArraySize(), mArraySize); 

    /*
    // mpInput = static_cast<SFloatMatrixI *>(BSAPICreateInstance(SIID_FLOATMATRIX));
    
    if(!mpInput)
      {
	fprintf(stderr, "No memory!");
        exit(1);
      }

    if(!mpInput->Init(1, iInput->GetArraySize()))
      {
        exit(1);
      }
    */
}

Tracter::BSAPIFilterBank::~BSAPIFilterBank() throw ()
{
  mpMelBanks->Release();
  // mpInput->Release();
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPIFilterBank::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    
  
    int one = mInput->Read(inputArea, iIndex, 1);
  
    float *pframe  = mInput->GetPointer(inputArea.offset);
 
    // float *pin_mat = mpInput->GetMem();
 
    for (int j=0; j<inputdim; j++) {
      pframe[j]*=WaveFromScaleUp;
      // if ( j % 10 == 0 )  printf ("\n");
      // printf("%f ", pframe[j]);
    }
    // printf ("\n");
    

    //for(int j = 0; j < mpInput->GetNColumns(); j++)
    //  pin_mat[j] = pframe[j];
    
    if ( one == 0 ) 
      mpMelBanks->OnWaveform( SWaveformSourceCallbackI::wfFloat , SampleFreq, 1, pframe, inputdim * sizeof(float), PF_LASTFRAME);
    else
      mpMelBanks->OnWaveform( SWaveformSourceCallbackI::wfFloat , SampleFreq, 1, pframe, inputdim * sizeof(float), 0);

    float* cache = GetPointer(iOffset);
    
    if (NULL != mTarget.mpNnout) {
      for (int i=0; i<mArraySize; i++) 
	cache[i] = mTarget.mpNnout[i];
         
      //      for (int i=0; i<mArraySize; i++)
      // printf("%f ",cache[i]);
      // printf ("\n"); 
	
    }
    
    return true;
}

bool Tracter::BSAPIFilterBank::STarget::OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags)
{
  float *pframe = pMatrix->GetMem();

  mpNnout = pframe;
 
  /*  
   for (int i=0; i<nFrames; i++) {
    printf("%f ", mpNnout[i]);
  }
  printf ("\n"); 
  */

  return true;
}
