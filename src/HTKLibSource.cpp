/*
 * Copyright 2008 by The University of Sheffield
 *
 * Copyright 2008 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

/*
 * HTKLibSource.cpp - Use HTKLib as the source in a tracter component.
 *
 * This version reads files using HTKLib. The resulting features are
 * specified by the HTK config file.
 *
 * Note that the setting of mFrame.size must be set correctly. There is
 * no access to any model files here so there is no easy way of
 * getting it automatically.
 *
 *  Created on: 04-Mar-2009
 *      Author: Vincent Wan, University of Sheffield, UK.
 */

#include "HTKLibSource.h"

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


/*
 * STACKSIZE as defined in HCopy
 */
#define STACKSIZE 100000        /* assume ~100K wave files */

/**
 * HTK initialisation should happen only once so we don't do it
 * here. Initialise HTK elsewhere.
 */
Tracter::HTKLibSource::HTKLibSource(const char* iObjectName)
{
    /***********************************************
     * First sort out tracter specific/non-HTK stuff
     ***********************************************/
    mObjectName = iObjectName;
    mFrame.size = GetEnv("FrameSize", 39);
    mFrameRate = GetEnv("FrameRate", 8000.0f);
    mFrame.period = GetEnv("FramePeriod", 80);

    /***********************************************
     * Now sort out the HTK specific stuff
     ***********************************************/
    /*
     * Allocate an HTK heap iStack
     */
    CreateHeap(&iStack, (char*)"InBuf", MSTAK, 1, 0.0, STACKSIZE, STACKSIZE);
    pbufIsOpen = false;
}


/**
 * Maps the HTK parameter file and reads the header
 */
void Tracter::HTKLibSource::Open(
    const char* iFileName, TimeType iBeginTime, TimeType iEndTime
)
{
    FileFormat dfmt=UNDEFF;    /* Data input file format */

    if ( pbufIsOpen )
    {
        CloseBuffer(pbuf);
        pbufIsOpen = false;
    }

    ResetHeap( &iStack );
    if((pbuf = OpenBuffer(&iStack,(char*)iFileName,50,dfmt,TRI_UNDEF,TRI_UNDEF))==NULL)
        HError(3250,(char*)"ProcessFile: OpenBuffer failed in Tracter::HTKLibSource::Open");   

    /*
     * Make the Observation structure for storing feature vectors.  We
     * assume that mFrame.size is correct because we have no easy
     * access to a model set to get the correct number automatically.
     */
    BufferInfo pbinfo;
    GetBufferInfo(pbuf,&pbinfo);
    int swidth0=1;
    short swidth[SMAX];
    Boolean saveAsVQ = FALSE;
    Boolean eSep;
    ZeroStreamWidths(swidth0,swidth);
    SetStreamWidths(pbinfo.tgtPK,mFrame.size,swidth,&eSep);
    data=MakeObservation(&iStack,swidth,pbinfo.tgtPK,saveAsVQ, eSep);

    StartBuffer(pbuf);
    pbufIsOpen = true;
}

/*
 * Destructor
 */
Tracter::HTKLibSource::~HTKLibSource() throw() {
    CloseBuffer( pbuf );
    DeleteHeap( &iStack );
}

/**
 * The Fetch call: EVIL HACK: We can ignore iIndex because HTK can
 * parse it's own extended filenames to seek to various positions in
 * the file
 */
int Tracter::HTKLibSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    int i;
    int offset = iOutputArea.offset;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if ( BufferStatus( pbuf ) == PB_CLEARED ) 
            break;
        if ( ! ReadAsBuffer( pbuf, &data ) )
            break;
        if (i == iOutputArea.len[0])
            offset = 0;
        float* cache = GetPointer(offset);
        for (int stream = 1 ; stream <= data.swidth[0] ; stream++)
        {
            memcpy(cache,data.fv[stream]+1,data.swidth[stream]*sizeof(float));
            cache+=data.swidth[stream];
        }
            /*
             * Print the feature vectors to STDOUT for debugging purposes.
             */
             /*
                printf("HTKLibSource:  ");
                cache = GetPointer(offset);
                for (int i=0; i<mFrame.size; i++)
                printf("%.3f ",cache[i]);
                printf("\n");
              */
        iIndex++;
        offset++;
    }
    return i;
}

