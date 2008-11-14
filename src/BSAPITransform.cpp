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

#include "BSAPITransform.h"

Tracter::BSAPITransform::BSAPITransform(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
   
    mContext    = GetEnv("Context", 15);
    assert(mContext > 0);

    MaxBufferedFrames  = GetEnv("MaxBufferedFrames",mContext+1);
 
    PluginObject::MinSize(mInput, MaxBufferedFrames, MaxBufferedFrames);

    mpFeaCat = static_cast<SFeaCatI *>(BSAPICreateInstance(SIID_FTR_FEACAT));
    if(!mpFeaCat)
      {
        fprintf(stderr, "No memory!");
        exit(1);
      }

    mpFeaCat->SetOutBatchSize(1);
    mpFeaCat->SetStartFrameRepetition(mContext);
    mpFeaCat->SetEndFrameRepetition(mContext-1); //Last frame is sent twice so mContext-1 repetition is enough
    mpFeaCat->SetTarget(&mTarget);

    const char* sfeacatmacro = GetEnv("MACRO", "macro");
    if(!mpFeaCat->Load((char*)sfeacatmacro))
      {
        exit(1);
      }
 
    mArraySize = mpFeaCat->GetNOutputs();
    assert(mArraySize > 0);

    // printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->GetArraySize(), mArraySize); 

    mpInput = static_cast<SFloatMatrixI *>(BSAPICreateInstance(SIID_FLOATMATRIX));
    if(!mpInput)
      {
	fprintf(stderr, "No memory!");
        exit(1);
      }

    if(!mpInput->Init(1, iInput->GetArraySize()))
      {
        exit(1);
      }

    mTarget.MaxBuffSize = MaxBufferedFrames*mArraySize;
    mTarget.mpOutBuff = new float[mTarget.MaxBuffSize];
    mTarget.nbuffsize = 0;

    LastFrameProcess=0;
}

Tracter::BSAPITransform::~BSAPITransform() throw ()
{
  mpFeaCat->Release();
  mpInput->Release();
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPITransform::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    
    int extend,shift;

    if (!iIndex){
      extend = MaxBufferedFrames;
      shift  = 0;
    }
    else {
      extend = 1;
      shift  = MaxBufferedFrames-1;
    }

    // printf("iIndex: %i\n", iIndex);  
    int numRead = mInput->Read(inputArea, iIndex+shift, extend);
    //int numRead = mInput->Read(inputArea, iIndex, 1);
  
    float *pframe = mInput->GetPointer(inputArea.offset);
    float *pin_mat = mpInput->GetMem();
 
    /*    for (int j=0; j<mArraySize; j++) {
	if ( j % 10 == 0 )  printf ("\n");
	printf("%f ", pframe[j]);
    }
    printf ("\n");
    */

    if ( numRead == 0 ){
      if (!LastFrameProcess){
	// printf("LastFrameSent\n");
	// In the mpInput is still saved last last frame from previous go
	mpFeaCat->OnFeatureMatrix(mpInput, 1, PF_LASTFRAME);
      }
      LastFrameProcess=1;
    }
    else{
      for ( int ii=0;ii<numRead;ii++ ){
	for(int j = 0; j < mpInput->GetNColumns(); j++){
	  //pin_mat[j] = pframe[j];
	  pin_mat[j] = pframe[ ii*mpInput->GetNColumns() + j];
	}
	/*
	for (int j=0; j<mpInput->GetNColumns(); j++) {
	  if ( j % 10 == 0 )  printf ("\n");
	  printf("%f ", pin_mat[j]);
	}
	printf ("\nSentProcess\n");*/
	mpFeaCat->OnFeatureMatrix(mpInput, 1, 0);  
      }
      LastFrameProcess=0;
    }

    if (!numRead && !mTarget.nbuffsize)
      return false;
    
    float* cache = GetPointer(iOffset);

    if (mTarget.nbuffsize) {
      for (int i=0; i<mArraySize; i++)
        cache[i] = mTarget.mpOutBuff[i];
      
      /*        for (int i=0; i<mTarget.nbuffsize; i++){
	  printf("%f ",mTarget.mpOutBuff[i]);
	  if ( (i+1) % 10 == 0 )  printf ("\n");
	  if ( (i+1) % 25 == 0 )  printf ("\n");
	  }
	  printf ("\n");
      */
      mTarget.nbuffsize-=mArraySize;
      memmove(mTarget.mpOutBuff, mTarget.mpOutBuff+mArraySize,mTarget.nbuffsize*sizeof(float));
    }

    return true;
}

bool Tracter::BSAPITransform::STarget::OnFeatureMatrix(SFloatMatrixI *pMatrix, int nFrames, unsigned int flags)
{  
  
  // printf("Frames: %i Flag: %i \n",nFrames, flags);

  if (nFrames)
    {
      float *pframe = pMatrix->GetMem();
      int NCol      = pMatrix->GetNColumns();

        
      // printf("NCol: %i nbuffsize: %i\n", NCol,nbuffsize);


      // if ( flags == PF_LASTFRAME )
      // return true;
  
      if ( nbuffsize+NCol > (MaxBuffSize) ){
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

      // printf("nbuffsize: %i\n",nbuffsize);
    }

  return true;
}
