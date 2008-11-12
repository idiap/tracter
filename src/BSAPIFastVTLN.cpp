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


// /mnt/matylda3/ciprtom/BSAPI-TMP

#include "BSAPIFastVTLN.h"

Tracter::BSAPIFastVTLN::BSAPIFastVTLN(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    inputdim    = iInput->GetArraySize(); 

    const char* MainFrontEndConfig = GetEnv("MainFrontEndConfig","./configs/bsrec.plp.cfg");
    const char* VTLNFrontEndConfig = GetEnv("VTLNFrontEndConfig","./configs/bsapi.VTLN.cfg");
    
    MaxBufferedFrames  = GetEnv("MaxBufferedFrames",5);

    WaveFromScaleUp = GetEnv("WaveFromScaleUp",32768);
    
    PluginObject::MinSize(mInput,  MaxBufferedFrames , MaxBufferedFrames);
    //PluginObject::MinSize(mInput,  1 , 1);
    
    mpFastVTLN = static_cast<SFastVtlnI *>(BSAPICreateInstance(SIID_FASTVTLN));
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

    mpPLP = static_cast<SSpeechRecI *>(BSAPICreateInstance(SIID_SPEECHREC));
    if(!mpPLP)
      {
        fprintf(stderr, "No memory!");
        exit(1);
      }

    if (!mpPLP->Init((char*)MainFrontEndConfig)){
      printf("Bad Initialization of Main Front End module!!!\n");
      mpPLP->Release();
      exit(1);
    }
    
    if (mpPLP->GetFeatureTransform()){
      mArraySize = mpPLP->GetFeatureTransform()->GetNOutputs();
      mpPLP->GetFeatureTransform()->SetTarget(&mTarget);
    }
    else{
      mArraySize = mpPLP->GetFeatureExtraction()->GetNOutputs();
      mpPLP->GetFeatureExtraction()->SetTarget(&mTarget);
    }
    
    SamplingFreq = GetMelTarget(mpPLP)->GetSampleFreq();
	
    assert(mArraySize > 0);
    
    printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->GetArraySize(), mArraySize); 


    mTarget.MaxBuffSize = MaxBufferedFrames*mArraySize;
    mTarget.mpOutBuff = new float[mTarget.MaxBuffSize];
    mTarget.nbuffsize = 0;

    LastFrameProcess=0;
}

Tracter::BSAPIFastVTLN::~BSAPIFastVTLN() throw ()
{
  mpFastVTLN->Release();
  mpPLP->Release();
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
      shift  = 4;
    }

    int numRead = mInput->Read(inputArea, iIndex+shift, extend);

    // printf("numRead : %i\n", numRead);
  
    float *pframe  = mInput->GetPointer(inputArea.offset);
 
    for (int j=0; j<inputdim*extend; j++) {
      pframe[j]*=WaveFromScaleUp;
      //  if ( (j+1) % 10 == 0 )  printf ("\n");
      //printf("%f ", pframe[j]);
    }
    //printf ("\n");
    


    //wf=1.0;
    if ( numRead )
    mpFastVTLN->OnWaveform( SWaveformSourceCallbackI::wfFloat , SamplingFreq, 1, pframe,  numRead * inputdim * sizeof(float), 0);

    wf = mpFastVTLN->GetWarpingFactor();
        
    printf("wf: %f\n",wf);

    GetMelTarget(mpPLP)->SetWarpAlpha(wf);
    
    if ( numRead == 0 ) {
      if (!LastFrameProcess)
      mpPLP->GetFeatureExtraction()->OnWaveform( SWaveformSourceCallbackI::wfFloat , WF_UNK_SAMPLEFREQ, 1, pframe, inputdim * sizeof(float), PF_LASTFRAME);
      LastFrameProcess=1;
    }
    else {
      mpPLP->GetFeatureExtraction()->OnWaveform( SWaveformSourceCallbackI::wfFloat , WF_UNK_SAMPLEFREQ, 1, pframe, numRead * inputdim * sizeof(float), 0);
      LastFrameProcess=0;
    }

    if (!numRead && !mTarget.nbuffsize)
      return false;
 
    float* cache = GetPointer(iOffset);

    if (mTarget.nbuffsize) {
      for (int i=0; i<mArraySize; i++) 
      	cache[i] = mTarget.mpOutBuff[i];
      
      /*  for (int i=0; i<mTarget.nbuffsize; i++){
	printf("%f ",mTarget.mpOutBuff[i]);
	if ( (i+1) % 10 == 0 )  printf ("\n");
	if ( (i+1) % 39 == 0 )  printf ("\n");
      }
      printf ("\n");*/ 
      
    mTarget.nbuffsize-=mArraySize;
    memmove(mTarget.mpOutBuff, mTarget.mpOutBuff+mArraySize,mTarget.nbuffsize*sizeof(float));
    }

    return true;
}

bool Tracter::BSAPIFastVTLN::STarget::OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags)
{
  float *pframe = pMatrix->GetMem();
  int NCol      = pMatrix->GetNColumns();
  
  // printf("Frames: %i Flag: %i NCol: %i nbuffsize: %i\n",nFrames, flags, NCol,nbuffsize);
  
  if ( flags == PF_LASTFRAME )
    return true;
  
  if ( nbuffsize+NCol >= (MaxBuffSize) ){
    printf("Trying to write out of range of output FrontEnd buffer...\n");
    exit(1);
  }

  for (int i=0; i<NCol; i++){
    mpOutBuff[nbuffsize+i] = pframe[i];
    // printf("%f ",pframe[i]);
    //if ( (i+1) % 10 == 0 )  printf ("\n");
  }
  // printf ("\n");
  nbuffsize+=NCol;
  
  //printf("nbuffsize: %i\n",nbuffsize);

  return true;
}

SMelBanksI *BSAPI_METHOD Tracter::BSAPIFastVTLN::GetMelTarget(SSpeechRecI *mpSpeechRec)
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
