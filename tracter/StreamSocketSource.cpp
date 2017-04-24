/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "StreamSocketSource.h"

void Tracter::StreamSocketSource::Open(
    const char* iHostName, TimeType iBeginTime, TimeType iEndTime
)
{
    /* Call the base class */
    SocketSource<float>::Open(iHostName);

    /* Get the timestamp */
    if (mSocket.Receive(sizeof(TimeType), (char*)&mTime) != sizeof(TimeType))
        throw Exception("Failed to receive timestamp");

    /* The communication is ms, but we store ns */
    mTime *= ONEe6;
    Verbose(2, "Time set to %lld\n", mTime);
}
