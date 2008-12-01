/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "SocketSource.h"


Tracter::SocketSource::SocketSource(void* iAuxiliary, const char* iObjectName)
{
    mObjectName = iObjectName;
    mAuxiliary = iAuxiliary;
    mArraySize = GetEnv("ArraySize", 1);
    mSampleFreq = GetEnv("SampleFreq", 48000.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 1);
    mPort = GetEnv("Port", 30000);
    mBufferSize = GetEnv("BufferSize", 0);
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

    int bufSize = 0;
    socklen_t len = sizeof(int);
    if (getsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &bufSize, &len))
        throw Exception("getsockopt failed");
    Verbose(1, "recvbuf is size %d\n", bufSize);

    if (mBufferSize > 0)
    {
        if (setsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &mBufferSize, len))
            throw Exception("setsockopt failed");
        if (getsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &bufSize, &len))
            throw Exception("getsockopt failed");
        Verbose(1, "resize: requested %d granted %d\n", mBufferSize, bufSize);
    }
}


/**
 * Receive data from socket.  Calls recv() until either iNBytes have
 * been received, or recv() returns 0.
 *
 * @returns the number of bytes actually received.
 */
int Tracter::SocketSource::Receive(int iNBytes, char* iBuffer)
{
    assert(iNBytes >= 0);
    assert(iBuffer);
    ssize_t nGet = iNBytes;
    ssize_t nGot = 0;
    while (nGot < nGet)
    {
        ssize_t n = recv(mFD, iBuffer+nGot, nGet-nGot, 0);
        nGot += n;
        if (n == -1)
        {
            perror(mObjectName);
            throw Exception("%s: recv failed for %d bytes",
                            mObjectName, iNBytes);
        }
        if (n == 0)
        {
            Verbose(1, "Read 0\n");
            return (int)nGot;
        }
    }
    return (int)nGot;
}

/**
 * A simple fetch call.  Implemented as two calls to Receive().
 */
int Tracter::SocketSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    int arraySize = ((mArraySize == 0) ? 1 : mArraySize) * sizeof(float);

    // First chunk of circular array
    char* cache = (char*)GetPointer(iOutputArea.offset);
    int nGet = arraySize * iOutputArea.len[0];
    int nGot0 = Receive(nGet, cache);
    if (nGot0 < nGet)
        return nGot0 / arraySize;

    if (iOutputArea.len[1])
    {
        // Second chunk of circular array
        cache = (char*)GetPointer();
        nGet = arraySize * iOutputArea.len[1];
        int nGot1 = Receive(nGet, cache);
        if (nGot1 < nGet)
            return (nGot0 + nGot1) / arraySize;
    }

    // If we get here, all was well
    return iOutputArea.Length();
}
