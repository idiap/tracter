/*
 * Copyright 2008 by The University of Sheffield
 *
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "SpeakerIDSocketSource.h"


Tracter::SpeakerIDSocketSource::SpeakerIDSocketSource(
    void* iAuxiliary, const char* iObjectName
)
  : StreamSocketSource(iAuxiliary, iObjectName)
{
    mObjectName = iObjectName;
    mAuxiliary = iAuxiliary;
    mArraySize = GetEnv("ArraySize", 1);
    mSampleFreq = GetEnv("SampleFreq", 1.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 1);
    mPort = GetEnv("Port", 4002);
    mFD = 0;
}

/**
 * The Fetch call.  Right now it breaks the fetch into smaller bits,
 * which is not big and not clever.
 */
int Tracter::SpeakerIDSocketSource::Fetch(
    IndexType iIndex, CacheArea& iOutputArea
)
{
    int i;
    int offset = iOutputArea.offset;
    int arraySize = mArraySize == 0 ? 1 : mArraySize;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if (i == iOutputArea.len[0])
            offset = 0;
        char* cache = (char*)GetPointer(offset);

	// Send time stamp to the socket: converting from nanoseconds to milliseconds
	TimeType timestamp = ( TimeStamp(0)+TimeOffset(iIndex) ) / 1000000;
	//TimeType timestamp = TimeStamp(iIndex) / 1000000;  ///<--- This doesn't seem to work!
	send( mFD, (char*)&timestamp , sizeof(TimeType) , 0 );

        // Read the data from the socket
        int nGet = arraySize * sizeof(float);
        int nGot = Receive(nGet, cache);
        Verbose(2, "Got %d bytes\n", (int)nGot);
        if (nGot < nGet)
            break;

        iIndex++;
        offset++;
    }

    return i;
}
