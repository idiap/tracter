/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/* See the file COPYING for the licence associated with this software. */

#include <cstdio>

#include "BSAPIFilterBank.h"

Tracter::BSAPIFilterBank::BSAPIFilterBank(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    inputdim    = iInput->Frame().size; 

    Connect(iInput, 1);
    mInput   = iInput;
    mInputWF = NULL;
  
    InitFrontEnd();

    InitOutBuffer();
}

Tracter::BSAPIFilterBank::BSAPIFilterBank(Component<float>* iInput,  Component<float>* iInputWF, const char* iObjectName)
{

    mObjectName = iObjectName;
  
    inputdim    = iInput->Frame().size; 
  
    Connect(iInput, 1);
    Connect(iInputWF, 1);
  
    mInput   = iInput;
    mInputWF = iInputWF;
    
    InitFrontEnd();

    InitOutBuffer();
}

void Tracter::BSAPIFilterBank::InitFrontEnd(void){
  
    SampleFreq = GetEnv("SampleFreq", 16000);
    WaveformScaleUp = GetEnv("WaveformScaleUp",32768);
  
    // If scaling is needed, the memory is allocated
    if ( WaveformScaleUp != 1 ) 
        mpInputWaveform = new float[inputdim];

    int VecSize     = GetEnv("VecSize", 400);
    int Step        = GetEnv("Step", 400);
    float PreemCoef = GetEnv("PreemCoef", 0);
    int NBanks      = GetEnv("NBanks", 23);
    bool TakeLog    = GetEnv("TakeLog", 1);
    float LoFreq    = GetEnv("LoFreq", 0);
    float HiFreq    = GetEnv("HiFreq", SampleFreq/2);

    float WarpLoFreq = GetEnv("WarpLoFreq", 3400);
    float WarpHiFreq = GetEnv("WarpHiFreq", 3400);
    float WarpAlpha  = GetEnv("WarpAlpha", 1); 

    ComponentBase::MinSize(mInput, 1, 1);

    mpMelBanks = static_cast<SMelBanksI *>(BSAPICreateInstance(SIID_FE_MELBANKS));
    if(!mpMelBanks)
    {
        fprintf(stderr, "No memory!");
        exit(1);
    }

    mpMelBanks->SetSampleFreq(SampleFreq);
    mpMelBanks->SetVectorSize(VecSize);
    mpMelBanks->SetStep(Step);
    mpMelBanks->SetPreemCoef(PreemCoef);
    mpMelBanks->SetNBanks(NBanks);
    mpMelBanks->SetTakeLog(TakeLog);
    mpMelBanks->SetLowFreq(LoFreq);
    mpMelBanks->SetHighFreq(HiFreq);
  
    mpMelBanks->SetLowWarpFreq(WarpLoFreq);
    mpMelBanks->SetHighWarpFreq(WarpHiFreq);
    mpMelBanks->SetWarpAlpha(WarpAlpha);

    mpMelBanks->SetOutBatchSize(1);
    mpMelBanks->SetTarget(&mTarget);
  
    mFrame.size = mpMelBanks->GetNOutputs();
    assert(mFrame.size > 0);
    //printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->Frame().size, mFrame.size); 
}


void Tracter::BSAPIFilterBank::InitOutBuffer(void){
    //mTarget.MaxBuffSize = MaxBufferedFrames*mFrame.size;
    mTarget.mpOutBuff = new float[mFrame.size];
    mTarget.nbuffsize = 0;
}

Tracter::BSAPIFilterBank::~BSAPIFilterBank() throw ()
{
    mpMelBanks->Release();
    delete[]mTarget.mpOutBuff;
    if ( WaveformScaleUp != 1 )
        delete[]mpInputWaveform;
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPIFilterBank::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    
    //  printf("iIndex: %i\n", iIndex);

    int one = mInput->Read(inputArea, iIndex, 1);
    //    if (!one)
    // return false;

    float *pframe  = mInput->GetPointer(inputArea.offset);
 
    // Memory scaling. If it is not needed, no memry is allocated in contructor, so just pointer is copied 
    if ( WaveformScaleUp != 1 ) 
        for (int j=0; j<inputdim; j++) 
            mpInputWaveform[j] = pframe[j] * WaveformScaleUp;
    else
        mpInputWaveform = pframe;
    

    //for (int j=0; j<inputdim*extend; j++) {
    // if ( (j+1) % 10 == 0 )  printf ("\n");
    //printf("%f ", mpInputWaveform[j]);
    //}
    //printf ("\n");
    

    if (mInputWF) {
        int numReadWF = mInputWF->Read(inputArea, iIndex, 1);
        if (numReadWF)
        {
            wf = *mInputWF->GetPointer(inputArea.offset);
            // printf("wf: %f\n",wf);
            mpMelBanks->SetWarpAlpha(wf);
        }
    }


    if (one)
        mpMelBanks->OnWaveform( SWaveformSourceCallbackI::wfFloat , SampleFreq, 1, mpInputWaveform, one * inputdim * sizeof(float), 0);
    else
        mpMelBanks->OnWaveform( SWaveformSourceCallbackI::wfFloat , SampleFreq, 1, mpInputWaveform, one * inputdim * sizeof(float), PF_LASTFRAME);
 
    
    // printf("nbuffsize %i ", mTarget.nbuffsize);
    if (mTarget.nbuffsize) {
        for (int i=0; i<mFrame.size; i++) 
            oData[i] = mTarget.mpOutBuff[i];
         
        mTarget.nbuffsize=0;
        /*      for (int i=0; i<mFrame.size; i++)
                printf("%f ",cache[i]);
                printf ("\n");*/ 
	
    }
    
    return one>=1;
}

bool Tracter::BSAPIFilterBank::STarget::OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags)
{
  
    // printf("Frames: %i Flag: %i \n",nFrames, flags);
  
    if (nFrames){
        float *pframe = pMatrix->GetMem();
        int NCol      = pMatrix->GetNColumns();
    
        for (int i=0; i<NCol; i++){
            // printf("%f ",pframe[i]);
            // if ( (i+1) % 10 == 0 )  printf ("\n");
            mpOutBuff[i] = pframe[i];
        }
        // printf ("\n");
        nbuffsize+=NCol;
    
        //printf("nbuffsize: %i\n",nbuffsize);
        /*  for (int i=0; i<nFrames; i++) {
            printf("%f ", mpNnout[i]);
            }
            printf ("\n"); 
        */
    
    }
 
    return true;
}
