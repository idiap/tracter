/*
 * Copyright 2008 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef STREAMSOCKETSOURCE_H
#define STREAMSOCKETSOURCE_H

#include "SocketSource.h"

namespace Tracter
{
    class StreamSocketSource : public SocketSource<float>
    {
    public:
        StreamSocketSource(
            void* iAuxiliary = 0,
            const char* iObjectName = "StreamSocketSource"
        )
            : SocketSource<float>(iAuxiliary, iObjectName) {}
        virtual void Open(
            const char* iHostName, TimeType iBeginTime=-1, TimeType iEndTime=-1
        );
    };
}

#endif /* STREAMSOCKETSOURCE_H */
