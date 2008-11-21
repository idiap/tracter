/*
 * Copyright 2008 by The University of Sheffield
 *
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

/*
 * HCopyWrapper.cpp - HCopy in a tracter plugin.
 *
 * This version supports input samples that are represented as 16 bit shorts.
 *
 * Environment variables for this module are:
 *		ConfigFile			The HTK config file.
 *      SpeechDetection		Turns on HTK's built-in speech activity detector.
 *
 *  Created on: 01-Jul-2008
 *      Author: Vincent Wan, University of Sheffield, UK.
 */

#include "HCopyWrapper.h"

#include "HMath.h"
#include "HSigP.h"
#include "HVQ.h"
#include "HLabel.h"
#include "HModel.h"
#include "esignal.h"
#ifdef UNIX
#include <sys/ioctl.h>
#endif

#undef FALSE
#undef TRUE

extern "C"
{
    /******************************************************************
     * Audio interface required by HParm - see cpp file for more info.
     ******************************************************************/
    Ptr  fOpen(Ptr xInfo,char *fn,BufferInfo *info);
    void fClose(Ptr xInfo,Ptr bInfo);
    void fStart(Ptr xInfo,Ptr bInfo);
    void fStop(Ptr xInfo,Ptr bInfo);
    int  fNumSamp(Ptr xInfo,Ptr bInfo);
    int  fGetData(Ptr xInfo,Ptr bInfo,int n,Ptr data);
};


#include <stdio.h>

/*
 * STACKSIZE as defined in HCopy
 */
#define STACKSIZE 100000        /* assume ~100K wave files */

/*
 * Constructor: Set up variables, initialise HTK components,
 *              allocate memory, read the HTK config file
 *              and lastly start buffering.
 */
Tracter::HCopyWrapper::HCopyWrapper(
    Plugin<hcopy_t>* iInput,
    const char* iObjectName
)
    : UnaryPlugin<float, hcopy_t>(iInput)
{
    /***********************************************
     * First sort out tracter specific/non-HTK stuff
     ***********************************************/
    mObjectName = iObjectName;
    lastSampleCopied = -1;
    lastFrameCopied = -1;

    /***********************************************
     * Now sort out the HTK specific stuff
     ***********************************************/
    TriState activateVAD = FALSE_dup;
    if ( GetEnv("SpeechDetection", 0) == 1 )
        activateVAD = TRUE_dup;
    /*
     * Sample size:
     * In HTK: 1      means  8 bit mulaw
     *         2      means 16 bit linear
     *         0x0101 means  8 bit alaw
     *
     * Since the template for this module is UnaryPlugin<float,short>
     * the sample size is hard coded to be 2. To use mulaw and/or alaw
     * update sampSize accordingly, update fGetData() changing short
     * to char, the class definition to: class HCopyWrapper : public
     * UnaryPlugin<float, char>, and Plugin<short> to Plugin<char>,
     * etc...
     */
    int sampSize = 2;
    /*
     * Initialise HTK (Straight cut and paste from HCopy)
     */
    char hcopy_version[] = "!HVER!HCopy:   3.4 [CUED 25/04/06]";
    char hcopy_vc_id[] = "$Id: HCopy.c,v 1.1.1.1 2006/10/11 09:54:59 jal58 Exp $";
    char* htkargv[4];
    htkargv[0] = (char*)"Tracter::HCopyWrapper";
    htkargv[1] = (char*)"-C";
    htkargv[2] = (char*)GetEnv("ConfigFile", "hcopywrapper.cfg" );
    htkargv[3] = (char*)"-D";
    int htkargc = 4;
    if(InitShell(htkargc,htkargv,hcopy_version,hcopy_vc_id)<SUCCESS)
        HError(1000, (char*)"HCopyWrapper: InitShell failed");
    InitMem();
    InitLabel();
    InitMath();
    InitSigP();
    InitWave();
    InitAudio();
    InitVQ();
    InitModel();
    if(InitParm()<SUCCESS)
        HError(1000,(char*)"HCopyWrapper: InitParm failed");
    /*
     * Get some of the config parameters for use immediately
     */
    ConfParam *cParm[MAXGLOBS];
    int nParm = GetConfig((char*)"HCOPY", TRUE, cParm, MAXGLOBS);
    char buf[MAXSTRLEN];
    /*
     * Determine the source kind
     */
    ParmKind srcPK = 0;
    if (GetConfStr(cParm,nParm,(char*)"SOURCEKIND",buf))
    {
        srcPK = Str2ParmKind(buf);
    } else {
        HError(1000,(char*)"HCopyWrapper: Couldn't determine SOURCEKIND");
    }
    /*
     * Determine the sample rate of the source
     */
    HTime srcSampRate;          /* time in 100ns units */
    if (!GetConfFlt(cParm,nParm,(char*)"SOURCERATE",&srcSampRate))
    {
        HError(1000,(char*)"HCopyWrapper: Couldn't determine SOURCERATE");
    }
    /*
     * Allocate an HTK heap iStack
     */
    CreateHeap(&iStack, (char*)"InBuf", MSTAK, 1, 0.0, STACKSIZE, LONG_MAX);
    /*
     * Create an HTK external source type HParmSrcDef and have it
     * point at the audio functions of this class. The first argument
     * is a pointer to extra info passed about by HTK for the benefit
     * of external sources: here we point back to this instance
     */
    HParmSrcDef hpsd = CreateSrcExt( this, srcPK, sampSize, srcSampRate,
                                     &fOpen, &fClose, &fStart, &fStop,
                                     &fNumSamp, &fGetData);
    /*
     * Open the external buffer. This automatically calls
     *   StartBuffer, FillBufFromChannel and also
     *   fOpen (for determining the target vector size)
     * so no need to do that ourselves.
     *
     * The second argument: char* filename is irrelevant here so set
     * to NULL
     *
     * The third argument: int maxObs = max number of feature vectors
     * to store in pbuf
     *
     * The last 2 args enable/disable speech detection and silence
     * threshold estimation respectively.
     */
    pbuf = OpenExtBuffer(&iStack, NULL, 100, HAUDIO, hpsd, activateVAD,
                         activateVAD);
    if (pbuf == NULL)
    {
        HError(1050,(char*)"HCopyWrapper: OpenExtBuffer returned NULL");
    }
}


/*
 * Destructor
 */
Tracter::HCopyWrapper::~HCopyWrapper() throw() {
    CloseBuffer( pbuf );
    DeleteHeap( &iStack );
}


/*
 * UnaryFetch as required by UnaryPlugin
 */
bool Tracter::HCopyWrapper::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    /*
     * The downstream element is requesting frame index iIndex but HTK
     * can't supply features out of sequence so if the iIndex requests
     * do not come in a very strict ascending order starting from 0
     * then we're stuffed.
     *
     * TODO: We'd be OK if frames were skipped entirely but that
     * involves some more coding.
     */
    if ( iIndex != lastFrameCopied + 1 )
    {
        HError(1000,
               (char*)"HCopyWrapper: Request for frames came out of order");
    }
    /*
     * Check on the HTK buffer status.
     */
    if ( BufferStatus( pbuf ) >= PB_STOPPED ) {
        HError( -1000 , (char*)"HCopyWrapper: Buffer status >= PB_STOPPED");
        return false;
    }
    /*
     * Tell HTK to calculate the next frame by calling ReadAsBuffer()
     * HTK will then access the call back functions fNumSamp() and
     * fGetData() to retrieve data from the cache.
     */
    if ( ! ReadAsBuffer( pbuf, &data ) )
    {
        HError( -1000 , (char*)"HCopyWrapper: ReadAsBuffer returned FALSE");
        return false;
    }
    /*
     * Copy the output of ReadAsBufer into the output buffer
     */
    float* cache = GetPointer(iOffset);
    for (int stream = 1 ; stream <= data.swidth[0] ; stream++)
    {
        memcpy(cache,data.fv[stream]+1,data.swidth[stream]*sizeof(float));
        cache+=data.swidth[stream];
    }
    lastFrameCopied++;
    /*
     * Print the feature vectors to STDOUT for debugging purposes.
     */
    /*
      printf("HCopyWrapper %d:  ",lastFrameCopied+1);
      cache = GetPointer(iOffset);
      for (int i=0; i<mArraySize; i++)
      printf("%.3f ",cache[i]);
      printf("\n");
    */
    /*
     * Finished
     */
    return true;
}







/************************************************************
 * Audio interface required by HParmSrcDef and CreateSrcExt()
 ************************************************************/

/* Open new buffer
 *
 * Return: Pointer to buffer specific data
 *
 * Connect to source and allocate necessary structures.
 * Each buffer is associated with a specific pointer that is assigned
 * to the return value of this function.  All other buffer operations
 * are passed this pointer.  Typically it will be used to access a
 * source specific data structure containing the necessary information
 * for operating the source.
 */
Ptr fOpen(Ptr thisHC,char *fn,BufferInfo *bInfo){
    if (Tracter::sVerbose > 0)
        printf("HCopyWrapper: fOpen()\n");
    return ((Tracter::HCopyWrapper*)thisHC)->fOpen__(fn,bInfo);
}
Ptr Tracter::HCopyWrapper::fOpen__(char *fn,BufferInfo *bInfo)
{
    // At this point we have enough information to set up the plugin
    mArraySize=bInfo->tgtVecSize;
    mSamplePeriod=bInfo->frRate;
    MinSize(mInput, bInfo->frSize);

    Verbose(1, "vec size %d, frame size %d, frame rate %d\n",
            bInfo->tgtVecSize, bInfo->frSize, bInfo->frRate);

    /*
     * Get the feature vector dimensionality and allocate memory for
     * an observation.
     */
    int swidth0=1;
    short swidth[SMAX];
    Boolean saveAsVQ = FALSE;
    Boolean eSep;
    ZeroStreamWidths(swidth0,swidth);
    SetStreamWidths(bInfo->tgtPK,mArraySize,swidth,&eSep);
    data = MakeObservation(&iStack, swidth, bInfo->tgtPK, saveAsVQ, eSep);
    return bInfo;
}

/* Close buffer and free resources
 *
 * Ptr bInfo: Pointer returned by fOpen for this buffer
 *
 * Free all the resources associated with the buffer (including if
 * necessary the info block itself).
 */
void fClose(Ptr thisHC,Ptr bInfo)
{
    // Most of the work will be done by the destructor so nothing to do.
    if (Tracter::sVerbose > 0)
        printf("HCopyWrapper: fClose()\n");
}

/* Start data capture for real-time sources
 *
 * Ptr bInfo: Pointer returned by fOpen for this buffer
 *
 * Start data capture.  Offline sources can ignore this call.
 */
void fStart(Ptr thisHC,Ptr bInfo)
{
    // Most of the work will have been done by tracter so nothing to do.
    if (Tracter::sVerbose > 0)
        printf("HCopyWrapper: fStart()\n");
}

/* Stop data capture for real-time sources
 *
 * Ptr bInfo: Pointer returned by fOpen for this buffer
 *
 * Stop data capture.  Offline sources can ignore this call.
 */
void fStop(Ptr thisHC,Ptr bInfo)
{
    // Most of the work will be done by the destructor so nothing to do.
    if (Tracter::sVerbose > 0)
        printf("HCopyWrapper: fStop()\n");
}

/* Query samples readable without blocking
 *
 * Ptr bInfo: Pointer returned by fOpen for this buffer
 * Return:   Samples readable without blocking
 *
 * Used to determine size of next read.  Offline sources can specify the
 * whole utterance whereas real-time sources should return the number of
 * buffered data samples once data capture has finished or -1 minus the
 * number of samples that can be read without blocking.
 */
int fNumSamp(Ptr thisHC,Ptr bInfo)
{
    /*
     * This is a global wrapper function so that a C program can access
     * the C++ function below as a call back.
     */
    return ( (Tracter::HCopyWrapper*)thisHC )->fNumSamp__(bInfo);
}
int Tracter::HCopyWrapper::fNumSamp__(Ptr bInfo)
{
    /*
     * The only way to determine the number of samples available is to
     * call "Read()" whose more descriptive name is
     * "CalculateTheEntireUpstreamChain()"
     */
    CacheArea inputArea;
    int nRead = mInput->Read(inputArea, lastSampleCopied+1, mSamplePeriod);
    int i = -1 - nRead;
    if (Tracter::sVerbose > 2)
        printf("%s::fNumSamp: read %d\n", mObjectName, nRead);
    return (i<-1) ? i : 0;
}

/* Read samples
 *
 * Ptr bInfo: Pointer returned by fOpen for this buffer
 * int n:    Number of samples required
 * Ptr data: Buffer for returned samples
 * Return:   Samples read correctly
 *
 * Read samples from the source.
 * In general will only read one frame at a time (either frSize samples
 * for the first frame or frRate samples for the rest).
 * Will only request a frame that fNumSamp indicates will block when the
 * next thing to do is process the frame.  Normally only non-blocking
 * data will be requested (unless the decoder is keeping up with the
 * source).
 */
int fGetData(Ptr thisHC,Ptr bInfo,int n,Ptr samples)
{
    /*
     * This is a global wrapper function so that a C program can access
     * the C++ function below as a call back.
     */
    return ( (Tracter::HCopyWrapper*)thisHC )->fGetData__(bInfo,n,samples);
}
int Tracter::HCopyWrapper::fGetData__(Ptr bInfo,int n,Ptr samples)
{
    /*
     * Perform a read for n samples
     */
    CacheArea inputArea;
    int got = mInput->Read(inputArea, lastSampleCopied+1, n );
    if ( got == 0 )
    {
        return 0;
    }
    /*
     * Copy the audio data from tracter's circular input buffer to samples.
     */
    short* s = (short*)samples;
#ifndef HCOPY_FLOAT
    short* p = (short*)mInput->GetPointer();
    memcpy( s , p+inputArea.offset , inputArea.len[0]*sizeof(short) );
    if (inputArea.len[1]>0)
        memcpy( s+inputArea.len[0] , p , inputArea.len[1]*sizeof(short) );
#else
    /* Convert float to short */
    float* p = mInput->GetPointer(inputArea.offset);
    for (int i=0; i<inputArea.len[0]; i++)
        s[i] = (short)(p[i]*32768);
    if (inputArea.len[1] > 0)
    {
        p = mInput->GetPointer();
        for (int i=0; i<inputArea.len[1]; i++)
            s[inputArea.len[0] + i] = (short)(p[i]*32768);
    }
#endif
    lastSampleCopied += got;
    return got;
}
