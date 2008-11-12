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
    : CachedPlugin<float>()
{
    mObjectName = iObjectName;
    mAuxiliary = iAuxiliary;
    mArraySize = GetEnv("ArraySize", 39);
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 80);
    mPort = GetEnv("Port", 30000);
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
 * The Fetch call.  Right now it breaks the fetch into smaller bits,
 * which is not big and not clever.
 */
int Tracter::SocketSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    int i;
    int offset = iOutputArea.offset;
    int arraySize = mArraySize == 0 ? 1 : mArraySize;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if (i == iOutputArea.len[0])
            offset = 0;
        char* cache = (char*)GetPointer(offset);

        // Read the data from the socket
        int nGet = arraySize * sizeof(float);
        int nGot = Receive(nGet, cache);
        Verbose(2, "Got %d bytes\n", (int)nGot);
        if (nGot < nGet)
            break;

        iIndex++;
        offset++;
    }

    return i;
}
