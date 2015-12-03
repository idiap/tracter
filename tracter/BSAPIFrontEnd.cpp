/***********************************************************
 *  Copyright 2008 by Brno University of Technology        *
 *                    www.vutbr.cz                         *
 *                                                         *
 *  Author:           Martin Karafiat                      *
 *                    UPGM,FIT,VUT,Brno                    *
 *                    karafiat@fit.vutbr.cz                *
 ***********************************************************/

/* See the file COPYING for the licence associated with this software. */

#include <cstring>
#include <cstdio>

#include "BSAPIFrontEnd.h"

Tracter::BSAPIFrontEnd::BSAPIFrontEnd(Component<float>* iInput, const char* iObjectName )
{
    mObjectName = iObjectName;
    inputdim    = iInput->Frame().size; 
 
    Connect(iInput, 1);
    mInput   = iInput;
    mInputWF = NULL;
 
    InitFrontEnd();

    InitOutBuffer();

    LastFrameProcess=0;

    //printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->Frame().size, mFrame.size); 
}

Tracter::BSAPIFrontEnd::BSAPIFrontEnd(Component<float>* iInput, Component<float>* iInputWF, const char* iObjectName)
{
    mObjectName = iObjectName;
    inputdim    = iInput->Frame().size; 
    
    Connect(iInput, 1);
    Connect(iInputWF, 1);

    mInput   = iInput;
    mInputWF = iInputWF;

    InitFrontEnd();

    InitOutBuffer();

    LastFrameProcess=0;

    // printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->Frame().size, mFrame.size); 
}


void Tracter::BSAPIFrontEnd::InitFrontEnd(void){
 
    const char* Config = GetEnv("Config","./configs/bsrec.plp.cfg");

    mpPLP = static_cast<SSpeechRecI *>(BSAPICreateInstance(SIID_SPEECHREC));
    if(!mpPLP)
    {
        fprintf(stderr, "No memory!");
        exit(1);
    }
  
    if (!mpPLP->Init((char*)Config)){
        printf("Bad Initialization of Main Front End module!!!\n");
        mpPLP->Release();
        exit(1);
    }
  
    if (mpPLP->GetFeatureTransform()){
        mFrame.size = mpPLP->GetFeatureTransform()->GetNOutputs();
        mpPLP->GetFeatureTransform()->SetTarget(&mTarget);
    }
    else{
        mFrame.size = mpPLP->GetFeatureExtraction()->GetNOutputs();
        mpPLP->GetFeatureExtraction()->SetTarget(&mTarget);
    }

  
    WaveformScaleUp = GetEnv("WaveformScaleUp",32768);
    //MaxBufferedFrames  = GetEnv("MaxBufferedFrames",5);
  
    MaxBufferedFrames = GetMelTarget(mpPLP)->GetEndFrameRepetition() + 1;
    // printf ("MaxBufferedFrames %i\n",MaxBufferedFrames);

    // If scaling is needed, the memory is allocated
    if ( WaveformScaleUp != 1 ) 
        mpInputWaveform = new float[inputdim*MaxBufferedFrames];
  
  
    ComponentBase::MinSize(mInput,  MaxBufferedFrames, MaxBufferedFrames);
    //ComponentBase::MinSize(mInput,  5 , 5);



    assert(mFrame.size > 0);
}

void Tracter::BSAPIFrontEnd::InitOutBuffer(void){
    mTarget.MaxBuffSize = MaxBufferedFrames*mFrame.size;
    mTarget.mpOutBuff = new float[mTarget.MaxBuffSize];
    mTarget.nbuffsize = 0;
}


Tracter::BSAPIFrontEnd::~BSAPIFrontEnd() throw ()
{
    mpPLP->Release();
    delete[]mTarget.mpOutBuff;
    if ( WaveformScaleUp != 1 )
        delete[]mpInputWaveform;
}

void Tracter::BSAPIFrontEnd::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    GetMelTarget(mpPLP)->Reset();
    CachedComponent<float>::Reset(iPropagate);
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPIFrontEnd::UnaryFetch(IndexType iIndex, float* oData)
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

    //    printf("numRead : %i\n", numRead);
  
    float *pframe  = mInput->GetPointer(inputArea.offset);


    // Memory scaling. If it is not needed, no memry is allocated in contructor, so just pointer is copied 
    if ( WaveformScaleUp != 1 ) 
        for (int j=0; j<inputdim*extend; j++) 
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
            GetMelTarget(mpPLP)->SetWarpAlpha(wf);
        }
    }
 
    
    if ( numRead == 0 ) {
        if (!LastFrameProcess)
            mpPLP->GetFeatureExtraction()->OnWaveform( SWaveformSourceCallbackI::wfFloat , WF_UNK_SAMPLEFREQ, 1, mpInputWaveform, numRead * inputdim * sizeof(float), PF_LASTFRAME);
        LastFrameProcess=1;
    }
    else {
        mpPLP->GetFeatureExtraction()->OnWaveform( SWaveformSourceCallbackI::wfFloat , WF_UNK_SAMPLEFREQ, 1, mpInputWaveform, numRead * inputdim * sizeof(float), 0);
        LastFrameProcess=0;
    }

    if (!numRead && !mTarget.nbuffsize)
        return false;
 
    if (mTarget.nbuffsize) {
        for (int i=0; i<mFrame.size; i++) 
            oData[i] = mTarget.mpOutBuff[i];
      
        /*
          for (int i=0; i<mTarget.nbuffsize; i++){
          printf("%f ",mTarget.mpOutBuff[i]);
          if ( (i+1) % 10 == 0 )  printf ("\n");
          if ( (i+1) % 39 == 0 )  printf ("\n");
          }
          printf ("\n"); 
        */

        mTarget.nbuffsize-=mFrame.size;
        memmove(mTarget.mpOutBuff, mTarget.mpOutBuff+mFrame.size,mTarget.nbuffsize*sizeof(float));
    }

    return true;
}

bool Tracter::BSAPIFrontEnd::STarget::OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags)
{

    //  printf("Frames: %i Flag: %i \n",nFrames, flags);

    if (nFrames)
    {
        float *pframe = pMatrix->GetMem();
        int NCol      = pMatrix->GetNColumns();
  
        //printf("NCol: %i nbuffsize: %i\n", NCol,nbuffsize);
  
        //  if ( flags == PF_LASTFRAME )
        //  return true;
      
        if ( nbuffsize+NCol > (MaxBuffSize) ){
            printf("BSAPIFrontEnd: out of range of output FrontEnd buffer"
                   " (%d + %d > %d)\n", nbuffsize, NCol, MaxBuffSize);
            exit(1);
        }
      
        for (int i=0; i<NCol; i++){
            mpOutBuff[nbuffsize+i] = pframe[i];
            // printf("%f ",pframe[i]);
            //if ( (i+1) % 10 == 0 )  printf ("\n");
        }
        // printf ("\n");
        nbuffsize+=NCol;
    }
    //printf("nbuffsize: %i\n",nbuffsize);

    return true;
}


SMelBanksI *BSAPI_METHOD Tracter::BSAPIFrontEnd::GetMelTarget(SSpeechRecI *mpSpeechRec)
{ 
    SFeatureExtractionI *pfe = mpSpeechRec->GetFeatureExtraction();
    SMelBanksI *pmb = 0;
    switch (pfe->GetIID())
    {
    case SIID_FE_MELBANKS:
        pmb = static_cast<SMelBanksI *>(pfe);
        break;
      
    case SIID_FE_PLP:
        pmb = static_cast<SPLPI *>(pfe)->GetMelBanksInterface();
        break;
      
    case SIID_FE_MFCC:
        pmb = static_cast<SMFCCI *>(pfe)->GetMelBanksInterface();
        break;
      
    default:
        printf("Unsupported feature extraction module.\n");
        printf("Should be 'mel_banks', 'plp' or 'mfcc'\n");
        return false;
    }
  
    return pmb;
}
