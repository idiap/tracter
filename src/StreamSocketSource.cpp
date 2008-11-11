/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "StreamSocketSource.h"

Tracter::StreamSocketSource::StreamSocketSource(
    void* iAuxiliary, const char* iObjectName
)
    : SocketSource(iAuxiliary, iObjectName)
{
    //mArraySize = GetEnv("ArraySize", 39);
    //mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    //mSamplePeriod = GetEnv("SamplePeriod", 80);
}

void Tracter::StreamSocketSource::Open(const char* iHostName)
{
    /* Call the base class */
    SocketSource::Open(iHostName);

    /* Get the timestamp */
    if (Receive(sizeof(TimeType), (char*)&mTime) != sizeof(TimeType))
        throw Exception("Failed to receive timestamp");

    /* The communication is ms, but we store ns */
    mTime *= 1000;
    Verbose(1, "Time set to %lld\n", mTime);
}
