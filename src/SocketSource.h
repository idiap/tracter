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

#define PACKET_HEADER_FLAG_END_OF_UTTERANCE 0x1
#define PACKET_HEADER_FLAG_BIG_ENDIAN 0x2

namespace Tracter
{
#pragma pack(push)
#pragma pack(1)
	struct PacketHeader
	{
		unsigned char header_size;
		unsigned char flags;
		long time_stamp[2];
		unsigned short array_size;
	};
#pragma pack(pop)

    /**
     * Source from a TCP socket.  This allows the source of a tracter
     * graph to be a TCP socket.  It presupposes that some process on
     * another machine exists to supply data to the socket.
     */
    class SocketSource : public CachedPlugin<float>, public Tracter::Source
    {
    public:
        SocketSource(const char* iObjectName = "SocketSource");
        ~SocketSource() throw ();
        void Open(const char* iFileName);

    protected:
        int Fetch(IndexType iIndex, CacheArea& iOutputArea);

    private:
        int mFD;
        unsigned short mPort;
        PacketHeader mPH;
        bool mPacketMode;

        ssize_t min(ssize_t a, ssize_t b) { return (a < b) ? a : b; }
        ssize_t max(ssize_t a, ssize_t b) { return (a > b) ? a : b; }
    };
}

#endif /* SOCKETSOURCE_H */
