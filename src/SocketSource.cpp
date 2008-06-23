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


SocketSource::SocketSource(const char* iObjectName)
    : CachedPlugin<float>()
{
    mObjectName = iObjectName;
    mArraySize = GetEnv("ArraySize", 39);
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = GetEnv("SamplePeriod", 80);

    mPort = GetEnv("Port", 30000);
    mFD = 0;
}

SocketSource::~SocketSource() throw()
{
    close(mFD);
    mFD = 0;
}


/**
 * Connects to a socket.
 */
void SocketSource::Open(const char* iHostName)
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
        printf("%s: gethostbyname() failed for %s\n",
               mObjectName, iHostName);
        exit(EXIT_FAILURE);
    }

    // For the file descriptor
    mFD = socket(PF_INET, SOCK_STREAM, 0);
    if (mFD < 0)
    {
        printf("%s: socket() failed for %s\n", mObjectName, iHostName);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }

    // Connect using the host address, port and file descriptor
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(mPort);
    server.sin_addr = *((struct in_addr *)hostEnt->h_addr);
    memset(server.sin_zero, '\0', sizeof server.sin_zero);

    if (connect(mFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("%s: connect() failed for %s:%hu\n",
               mObjectName, iHostName, mPort);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }
}

/**
 * The Fetch call.  Right now it breaks the fetch into smaller bits,
 * which is not big and not clever.
 */
int SocketSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    int i;
    int offset = iOutputArea.offset;
    int arraySize = mArraySize == 0 ? 1 : mArraySize;
    for (i=0; i<iOutputArea.Length(); i++)
    {
        if (i == iOutputArea.len[0])
            offset = 0;
        char* cache = (char*)GetPointer(offset);

        // Read the socket
        ssize_t nGet = arraySize*sizeof(float);
        ssize_t nGot = 0;
        while (nGot < nGet)
        {
	  //printf(".");
            ssize_t n = recv(mFD, cache+nGot, nGet-nGot, 0);
            nGot += n;
            if (n == -1)
            {
                perror("mObjectName");
                exit(EXIT_FAILURE);
            }
#if 0
            if (n == 0)
            {
                printf("Read 0\n");
                break;
            }
#endif
        }
	if (Tracter::sVerbose > 1)
	  printf("Got %d bytes\n", (int)nGot);
        if (nGot < nGet)
            break;

        //if (mByteOrder.WrongEndian())
        //    mByteOrder.Swap(cache, 4, arraySize);
        iIndex++;
        offset++;
    }
    return i;
}
