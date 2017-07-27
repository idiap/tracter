/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "StreamSocketSource.h"

void Tracter::StreamSocketSource::open(
    const char* iHostName, TimeType iBeginTime, TimeType iEndTime
)
{
    /* Call the base class */
    SocketSource<float>::open(iHostName);

    /* Get the timestamp */
    if (mSocket.Receive(sizeof(TimeType), (char*)&mTime) != sizeof(TimeType))
        throw Exception("Failed to receive timestamp");

    /* The communication is ms, but we store ns */
    mTime *= ONEe6;
    verbose(2, "Time set to %lld\n", mTime);
}
