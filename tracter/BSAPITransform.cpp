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

#include "BSAPITransform.h"

Tracter::BSAPITransform::BSAPITransform(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    inputdim    = iInput->Frame().size; 

    Connect(iInput, 1);
    mInput   = iInput;
    mInputID   = NULL;

    InitTransform();

    InitOutBuffer();
    
    LastFrameProcess=0;
}

Tracter::BSAPITransform::BSAPITransform(Component<float>* iInput, Component<float>* iInputID, const char* iObjectName)
{
    mObjectName = iObjectName;
    inputdim    = iInput->Frame().size; 

    Connect(iInput, 1);
    Connect(iInputID, 1);

    mInput     = iInput;
    mInputID   = iInputID;
    
    const char* xformdir = GetEnv("MACRODIR", "");
    
    mInputID_macroname_full = new char[strlen(xformdir) + sizeof(float)]; //Dir length + filename

    strcpy(mInputID_macroname_full,xformdir);

    InitTransform(); 

    InitOutBuffer();
    
    LastFrameProcess=0;
}


void Tracter::BSAPITransform::InitTransform(void){
    mContext    = GetEnv("Context", 15);
    assert(mContext >= 0);
  
    MaxBufferedFrames  = GetEnv("MaxBufferedFrames",mContext+1);
  
    ComponentBase::MinSize(mInput, MaxBufferedFrames, MaxBufferedFrames);

    mpFeaCat = static_cast<SFeaCatI *>(BSAPICreateInstance(SIID_FTR_FEACAT));
    if(!mpFeaCat)
    {
        fprintf(stderr, "No memory!");
        exit(1);
    }
  
    mpFeaCat->SetOutBatchSize(1);
    mpFeaCat->SetStartFrameRepetition(mContext);
    mpFeaCat->SetEndFrameRepetition(mContext); //Last frame is sent twice so mContext-1 repetition is enough
    mpFeaCat->SetTarget(&mTarget);

    // Load Transform  
    const char* sfeacatmacro = GetEnv("MACRO", "macro");
    if(!mpFeaCat->Load((char*)sfeacatmacro))
    {
        exit(1);
    }
  
    mFrame.size = mpFeaCat->GetNOutputs();
    assert(mFrame.size > 0);
  
    // printf("ArraySizeIn %i   ArraySizeOut %i\n", iInput->Frame().size, mFrame.size); 

    mpInput = static_cast<SFloatMatrixI *>(BSAPICreateInstance(SIID_FLOATMATRIX));
    if(!mpInput)
    {
        fprintf(stderr, "No memory!");
        exit(1);
    }

    if(!mpInput->Init(1, inputdim))
    {
        exit(1);
    }
}

void Tracter::BSAPITransform::InitOutBuffer(void){
 
    mTarget.MaxBuffSize = MaxBufferedFrames*mFrame.size;
    mTarget.mpOutBuff = new float[mTarget.MaxBuffSize];
    mTarget.nbuffsize = 0;
}


Tracter::BSAPITransform::~BSAPITransform() throw ()
{
    mpFeaCat->Release();
    mpInput->Release();
    delete[]mTarget.mpOutBuff;
}

void Tracter::BSAPITransform::Reset(bool iPropagate)
{
    Verbose(2, "Reset\n");
    mpFeaCat->Reset();
    CachedComponent<float>::Reset(iPropagate);
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::BSAPITransform::UnaryFetch(IndexType iIndex, float* oData)
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
 
    
    if( mInputID && ( mInputID_macroname != *mInputID->GetPointer(inputArea.offset) ) ){
        mInputID_macroname = *mInputID->GetPointer(inputArea.offset);
        strcpy(mInputID_macroname_full+strlen(xformdir), (char*)&mInputID_macroname);

        printf("Using macro from: %s\n", mInputID_macroname_full);
        if(!mpFeaCat->Load(mInputID_macroname_full))
        {
            exit(1);
        }
    }
    
    /*    for (int j=0; j<mFrame.size; j++) {
          if ( j % 10 == 0 )  printf ("\n");
          printf("%f ", pframe[j]);
          }
          printf ("\n");
    */

    if ( numRead == 0 ){
        if (!LastFrameProcess){
            // printf("LastFrameSent\n");
            // In the mpInput is still saved last last frame from previous go
            mpFeaCat->OnFeatureMatrix(mpInput, 0, PF_LASTFRAME);
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
    
    if (mTarget.nbuffsize) {
        for (int i=0; i<mFrame.size; i++)
            oData[i] = mTarget.mpOutBuff[i];
      
        /*        for (int i=0; i<mTarget.nbuffsize; i++){
                  printf("%f ",mTarget.mpOutBuff[i]);
                  if ( (i+1) % 10 == 0 )  printf ("\n");
                  if ( (i+1) % 25 == 0 )  printf ("\n");
                  }
                  printf ("\n");
        */
        mTarget.nbuffsize-=mFrame.size;
        memmove(mTarget.mpOutBuff, mTarget.mpOutBuff+mFrame.size,mTarget.nbuffsize*sizeof(float));
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
            printf("BSAPITransform: out of range of output FrontEnd buffer"
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

        // printf("nbuffsize: %i\n",nbuffsize);
    }

    return true;
}
