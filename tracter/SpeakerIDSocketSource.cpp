/*
 * Copyright 2008 by The University of Sheffield
 *
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SpeakerIDSocketSource.h"


Tracter::SpeakerIDSocketSource::SpeakerIDSocketSource(
    void* iAuxiliary, const char* iObjectName
)
    : StreamSocketSource(iAuxiliary, iObjectName)
{
    mTimeOffset = GetEnv("TimeOffset", 0);
}

/**
 * Fetch speaker ID.  This is done by sending a timestamp to the
 * server.  It should respond immediately with a speaker ID, otherwise
 * everything will grind to a halt.
 */
bool Tracter::SpeakerIDSocketSource::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(mFrame.size == 1);

    Verbose(2, "Fetching SpeakerID for index %ld\n", iIndex);
    char* cache = (char*)GetPointer(iOffset);

    // Send time stamp to the socket: converting from nanoseconds
    // to milliseconds
    //TimeType timestamp = ( TimeStamp(0)+TimeOffset(iIndex) ) / 1000000;
    //TimeType timestamp = TimeStamp(iIndex) / 1000000;  ///<--- This doesn't seem to work!
    // PNG: Incoming index is absoute time in seconds
    TimeType timestamp = (TimeType)iIndex * ONEe3 + mTimeOffset;
    Verbose(2, "Sending time %lld ms\n", timestamp);
#if 1
    mSocket.Send(sizeof(TimeType), (char*)&timestamp);

    // Read the data from the socket
    Verbose(2, "Waiting for response\n");
    int nGet = sizeof(float);
    int nGot = mSocket.Receive(nGet, cache);
    Verbose(2, "Got %d bytes\n", (int)nGot);
    if (nGot < nGet)
        return false;
#else
    arraySize += 1; //dummy
    cache[0] = 'a';
    cache[1] = 'b';
    cache[2] = 'c';
    cache[3] = 0;
#endif

    return true;
}
