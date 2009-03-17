/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETSOURCE_H
#define SOCKETSOURCE_H

#include <netdb.h>

#include "CachedPlugin.h"
#include "Source.h"

namespace Tracter
{
    /**
     * The type-independent socket source implementation.
     */
    class socketSource : public Tracter::Object
    {
    public:
        socketSource(const char* iObjectName = "SocketSource");
        virtual ~socketSource() throw ();
        virtual void Open(const char* iHostName);
        int Receive(int iNBytes, char* iBuffer);
        void Send(int iNBytes, char* iBuffer);

    private:
        int mFD;
        int mBufferSize;
        unsigned short mPort;
    };

    /**
     * Source from a TCP socket.  This allows the source of a tracter
     * graph to be a TCP socket.  It presupposes that some process on
     * another machine exists to supply data to the socket.
     */
    template <class T>
    class SocketSource : public Source< CachedPlugin<T> >
    {
    public:
        SocketSource(
            void* iAuxiliary = 0, const char* iObjectName = "SocketSource"
        )
            : mSocket(iObjectName)
        {
            Source< CachedPlugin<T> >::mObjectName = iObjectName;
            Source< CachedPlugin<T> >::mAuxiliary = iAuxiliary;
            Source< CachedPlugin<T> >::mArraySize =
                Source< CachedPlugin<T> >::GetEnv("ArraySize", 1);
            Source< CachedPlugin<T> >::mSampleFreq =
                Source< CachedPlugin<T> >::GetEnv("SampleFreq", 48000.0f);
            Source< CachedPlugin<T> >::mSamplePeriod =
                Source< CachedPlugin<T> >::GetEnv("SamplePeriod", 1);
        }
        virtual ~SocketSource() throw () {}
        virtual void Open(const char* iHostName)
        {
            mSocket.Open(iHostName);
        }

    protected:
        /**
         * A simple fetch call.  Implemented as two calls to
         * Receive().
         */
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea)
        {
            int arraySize = Source< CachedPlugin<T> >::mArraySize;
            arraySize = ((arraySize == 0) ? 1 : arraySize) * sizeof(float);

            // First chunk of circular array
            char* cache = (char*)Source< CachedPlugin<T> >::GetPointer(
                iOutputArea.offset
            );
            int nGet = arraySize * iOutputArea.len[0];
            int nGot0 = mSocket.Receive(nGet, cache);
            if (nGot0 < nGet)
                return nGot0 / arraySize;

            if (iOutputArea.len[1])
            {
                // Second chunk of circular array
                cache = (char*)Source< CachedPlugin<T> >::GetPointer();
                nGet = arraySize * iOutputArea.len[1];
                int nGot1 = mSocket.Receive(nGet, cache);
                if (nGot1 < nGet)
                    return (nGot0 + nGot1) / arraySize;
            }

            // If we get here, all was well
            return iOutputArea.Length();
        }

        socketSource mSocket;
    };
}

#endif /* SOCKETSOURCE_H */
