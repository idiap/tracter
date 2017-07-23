/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETSOURCE_H
#define SOCKETSOURCE_H

#include "CachedComponent.h"
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
    class SocketSource : public Source< CachedComponent<T> >
    {
    public:
        SocketSource(
            void* iAuxiliary = 0, const char* iObjectName = "SocketSource"
        )
            : mSocket(iObjectName)
        {
            Source< CachedComponent<T> >::objectName(iObjectName);
            Source< CachedComponent<T> >::mAuxiliary = iAuxiliary;
            Source< CachedComponent<T> >::mFrame.size =
                Source< CachedComponent<T> >::config("FrameSize", 1);
            Source< CachedComponent<T> >::mFrame.period =
                Source< CachedComponent<T> >::config("FramePeriod", 1);
            Source< CachedComponent<T> >::mFrameRate =
                Source< CachedComponent<T> >::config("FrameRate", 48000.0f);
        }
        virtual ~SocketSource() throw () {}
        virtual void Open(
            const char* iHostName,
            TimeType iBeginTime = -1,
            TimeType iEndTime = -1
        )
        {
            mSocket.Open(iHostName);
        }

    protected:
        /**
         * A simple fetch call.  Implemented as two calls to
         * Receive().
         */
        virtual SizeType Fetch(IndexType iIndex, CacheArea& iOutputArea)
        {
            int getSize = Source< CachedComponent<T> >::mFrame.size;
            getSize = ((getSize == 0) ? 1 : getSize) * sizeof(T);

            // First chunk of circular array
            char* cache = (char*)Source< CachedComponent<T> >::GetPointer(
                iOutputArea.offset
            );
            SizeType nGet = getSize * iOutputArea.len[0];
            SizeType nGot0 = mSocket.Receive(nGet, cache);
            if (nGot0 < nGet)
                return nGot0 / getSize;

            if (iOutputArea.len[1])
            {
                // Second chunk of circular array
                cache = (char*)Source< CachedComponent<T> >::GetPointer();
                nGet = getSize * iOutputArea.len[1];
                int nGot1 = mSocket.Receive(nGet, cache);
                if (nGot1 < nGet)
                    return (nGot0 + nGot1) / getSize;
            }

            // If we get here, all was well
            return iOutputArea.Length();
        }

        socketSource mSocket;
    };
}

#endif /* SOCKETSOURCE_H */
