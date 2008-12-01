/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETSOURCE_H
#define SOCKETSOURCE_H

#include <netdb.h>

#include "Source.h"
#include "CachedPlugin.h"

namespace Tracter
{
    /**
     * Source from a TCP socket.  This allows the source of a tracter
     * graph to be a TCP socket.  It presupposes that some process on
     * another machine exists to supply data to the socket.
     */
    class SocketSource : public CachedPlugin<float>, public Tracter::Source
    {
    public:
        SocketSource(
            void* iAuxiliary = 0, const char* iObjectName = "SocketSource"
        );
        virtual ~SocketSource() throw ();
        virtual void Open(const char* iHostName);

    protected:

        /** Diverts basic time stamp requests to the Source base class */
        virtual TimeType TimeStamp(IndexType iIndex)
        {
            return Source::TimeStamp(iIndex);
        }
        int Receive(int iNBytes, char* iBuffer);
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);
        int mFD;
        int mBufferSize;
        unsigned short mPort;
    };
}

#endif /* SOCKETSOURCE_H */
