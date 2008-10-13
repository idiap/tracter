/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "SocketSource.h"


Tracter::SocketSource::SocketSource(const char* iObjectName)
    : CachedPlugin<float>()
{
    mObjectName = iObjectName;
    mArraySize = GetEnv("ArraySize", 39);
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 80);

    mPort = GetEnv("Port", 30000);
    mPacketMode = GetEnv("PacketMode", 0);
    mFD = 0;
}

Tracter::SocketSource::~SocketSource() throw()
{
    close(mFD);
    mFD = 0;
}


/**
 * Connects to a socket.
 */
void Tracter::SocketSource::Open(const char* iHostName)
{
    assert(iHostName);

    if (mFD)
    {
        close(mFD);
        mFD = 0;
    }

    // For the host address
    struct hostent* hostEnt = gethostbyname(iHostName);
    if (!hostEnt)
    {
        // Would be better to parse h_errno here
        throw Exception("%s: gethostbyname() failed for %s\n",
                        mObjectName, iHostName);
    }

    // For the file descriptor
    mFD = socket(PF_INET, SOCK_STREAM, 0);
    if (mFD < 0)
    {
        perror(mObjectName);
        throw Exception("%s: socket() failed for %s\n",
                        mObjectName, iHostName);
    }

    // Connect using the host address, port and file descriptor
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(mPort);
    server.sin_addr = *((struct in_addr *)hostEnt->h_addr);
    memset(server.sin_zero, '\0', sizeof server.sin_zero);

    if (connect(mFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror(mObjectName);
        throw Exception("%s: connect() failed for %s:%hu\n",
                        mObjectName, iHostName, mPort);
    }
}

/**
 * The Fetch call.  Right now it breaks the fetch into smaller bits,
 * which is not big and not clever.
 */
int Tracter::SocketSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
	bool hread = true;
    int i;
    int offset = iOutputArea.offset;
    int arraySize = mArraySize == 0 ? 1 : mArraySize;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if (i == iOutputArea.len[0])
            offset = 0;
        char* cache = (char*)GetPointer(offset);

        if (mPacketMode && hread)
        {
			// Read the header from the socket
			ssize_t nDHGet = sizeof(mPH);
			ssize_t nHGet = sizeof(mPH);
			ssize_t nHGot = 0;

			Verbose(2, "Ready to get %d bytes of the header (%d of %d)\n", (int)nHGet, i, iOutputArea.Length());

			// Get the main part of the header
			while ((nHGot < nHGet) && (nHGot < nDHGet))
			{
				ssize_t n = recv(mFD, ((char *)&mPH)+nHGot, nHGet-nHGot, 0);
				Verbose(2, "Got first %d bytes of the header\n", (int)nHGot+n);

				// Correct the header size
				if ((nHGot == 0) && (n > 0))
					nDHGet = mPH.header_size;

				nHGot += n;
				if (n == -1)
				{
					perror(mObjectName);
					throw Exception("recv failed");
				}
	#if 0
				if (n == 0)
				{
					printf("Read 0\n");
					break;
				}
	#endif
			}

			// Get the rest of the header
			while (nHGot < nDHGet)
			{
				char dummy;
				ssize_t n = recv(mFD, &dummy, 1, 0);
				Verbose(2, "Got first %d bytes of the header (including extra part)\n", (int)nHGot+n);
				nHGot += n;
				if (n == -1)
				{
					perror(mObjectName);
					throw Exception("recv failed");
				}
	#if 0
				if (n == 0)
				{
					printf("Read 0\n");
					break;
				}
	#endif
			}
			Verbose(2, "Got %d bytes of the header (%d, 0x%X, %d, %d, %d)\n", (int)nHGot, (int)mPH.header_size, mPH.flags, (int)mPH.time_stamp[0], (int)mPH.time_stamp[1], (int)mPH.array_size);
			if (nHGot < nHGet)
				break;
        }

        // Read the data from the socket
        ssize_t nGet = mPacketMode ? (min(max(1, mSize - offset), min(iOutputArea.Length() - i, mPH.array_size)) * sizeof(float)) : (arraySize * sizeof(float));
        ssize_t nGot = 0;
        while (nGot < nGet)
        {
            ssize_t n = recv(mFD, cache+nGot, nGet-nGot, 0);
            nGot += n;
            if (n == -1)
            {
                perror(mObjectName);
                throw Exception("recv failed");
            }
#if 0
            if (n == 0)
            {
                printf("Read 0\n");
                break;
            }
#endif
        }
        if (mPacketMode)
        {
			mPH.array_size -= nGot / sizeof(float);
			hread = (mPH.array_size <= 0);
			iOutputArea.forceDecode = ((mPH.flags & PACKET_HEADER_FLAG_END_OF_UTTERANCE) != 0);
        }
        Verbose(2, "Got %d bytes\n", (int)nGot);
        if (nGot < nGet)
            break;

        // In case of the end of utterance don't process further
        if (mPacketMode && ((mPH.flags & PACKET_HEADER_FLAG_END_OF_UTTERANCE) != 0))
        	break;

        //if (mByteOrder.WrongEndian())
        //    mByteOrder.Swap(cache, 4, arraySize);
        iIndex += nGot / sizeof(float);
        offset += nGot / sizeof(float);
        i += nGot / sizeof(float) - 1;
    }
    return i;
}
